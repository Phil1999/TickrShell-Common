#include "StockTracker/CurrencyService.h"
#include <spdlog/spdlog.h>
#include <algorithm>

namespace StockTracker {

    CurrencyService::CurrencyService()
        : context(std::make_unique<zmq::context_t>(1))
        , socket(std::make_unique<zmq::socket_t>(*context, zmq::socket_type::req))
    {
        socket->connect(ENDPOINT);
        //spdlog::info("CurrencyService connected to {}", ENDPOINT);
    }

    bool CurrencyService::isValidCurrencyCode(const std::string& currency) {
        return CURRENCY_SYMBOLS.find(currency) != CURRENCY_SYMBOLS.end();
    }

    std::string CurrencyService::getCurrencySymbol(const std::string& currency_code) {
        auto it = CURRENCY_SYMBOLS.find(currency_code);
        if (it != CURRENCY_SYMBOLS.end()) {
            return it->second;
        }
        return currency_code; // Return code itself if symbol not found
    }

    double CurrencyService::convertCurrency(double amount, const std::string& to_currency) {
        if (!isValidCurrencyCode(to_currency)) {
            throw std::runtime_error("Invalid currency code: " + to_currency);
        }

        // Create request JSON
        nlohmann::json request = {
            {"amount", amount},
            {"from_currency", DEFAULT_CURRENCY},
            {"to_currency", to_currency}
        };

        spdlog::debug("Requesting conversion: {} USD to {}", amount, to_currency);

        // Send request
        std::string request_str = request.dump();
        socket->send(zmq::buffer(request_str), zmq::send_flags::none);

        // Set timeout for receive
        socket->set(zmq::sockopt::rcvtimeo, 2000);

        // Receive response
        zmq::message_t reply;
        auto result = socket->recv(reply);

        if (result) {
            std::string reply_str(static_cast<char*>(reply.data()), reply.size());
            auto response = nlohmann::json::parse(reply_str);

            if (response.contains("error")) {
                throw std::runtime_error("Currency conversion failed: " +
                    response["error"].get<std::string>());
            }

            if (!response.contains("converted_amount") || response["converted_amount"].is_null()) {
                throw std::runtime_error("Invalid response from currency service");
            }

            return response["converted_amount"].get<double>();
        }

        throw std::runtime_error("No response from currency service");
    }


}