#pragma once
#include <zmq.hpp>
#include <string>
#include <memory>
#include <nlohmann/json.hpp>

namespace StockTracker {

    class CurrencyService {
    public:
        CurrencyService();
        ~CurrencyService() = default;

        // Convert amount from USD to target currency
        double convertCurrency(double amount, const std::string& to_currency);

        // Get currency symbol (e.g., "$" for USD, "€" for EUR)
        static std::string getCurrencySymbol(const std::string& currency_code);

        // Check if currency code is valid
        static bool isValidCurrencyCode(const std::string& currency);

    private:
        std::unique_ptr<zmq::context_t> context;
        std::unique_ptr<zmq::socket_t> socket;

        const std::string DEFAULT_CURRENCY = "USD";
        const std::string ENDPOINT = "tcp://localhost:5555";

        static const inline std::unordered_map<std::string, std::string> CURRENCY_SYMBOLS = {
            {"USD", "$"},
            {"EUR", "€"},
            {"GBP", "£"},
            {"JPY", "¥"},
            {"CNY", "¥"},
            {"KRW", "₩"},
            {"INR", "₹"},
            {"CAD", "C$"},
            {"AUD", "A$"},
            {"CHF", "₣"},
            {"HKD", "HK$"},
            {"SGD", "S$"}
        };
    };
}