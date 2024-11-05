#include "StockTracker/Messages.h"

namespace StockTracker {
    // Message factory methods
    Message Message::makeSubscribe(std::string symbol) {
        return Message{ MessageType::Subscribe, std::move(symbol) };
    }

    Message Message::makeUnsubscribe(std::string symbol) {
        return Message{ MessageType::Unsubscribe, std::move(symbol) };
    }

    Message Message::makeQuoteUpdate(StockQuote quote) {
        return Message{
            MessageType::QuoteUpdate,
            quote.symbol,
            std::move(quote)
        };
    }

    Message Message::makeQuery(std::string symbol) {
        return Message{
            MessageType::Query,
            symbol
        };
    }

    Message Message::makeRequestPriceHistory(const std::string& symbol) {
        return Message{ MessageType::PriceHistoryRequest, symbol };
    }

    // Create a message to request the subscription list from the backend
    Message Message::makeRequestSubscriptions() {
        return Message{ MessageType::RequestSubscriptions };
    }

    // Create a message to send the subscription list to the frontend (CLI)
    Message Message::makeSubscriptionsList(const std::vector<std::string>& subscriptions) {
        return Message{
            MessageType::SubscriptionsList,
            "",  // No specific symbol for this message
            std::nullopt,   // No quote
            std::nullopt,   // No error
            std::nullopt,   // No priceHistory
            subscriptions   // Subscription list
        };
    }

    Message Message::makePriceHistory(const std::string& symbol, const std::vector<StockQuote>& history) {
        return Message{
            MessageType::PriceHistoryResponse,  // Use appropriate message type
            symbol,
            std::nullopt,  // No quote
            std::nullopt,  // No error
            history        // Price history
        };
    }

    Message Message::makeError(std::string error) {
        return Message{
            MessageType::Error,
            "",
            std::nullopt,
            std::move(error)
        };
    }

    // JSON serialization
    void to_json(json& j, const Message& msg) {
        j = json{
            {"type", msg.type},
            {"symbol", msg.symbol}
        };
        if (msg.quote) {
            j["quote"] = *msg.quote;
        }
        if (msg.error) {
            j["error"] = *msg.error;
        }
        if (msg.subscriptions) {
            j["subscriptions"] = *msg.subscriptions;
        }
        if (msg.priceHistory) {
            j["priceHistory"] = *msg.priceHistory;
        }
    }

    void from_json(const json& j, Message& msg) {
        j.at("type").get_to(msg.type);
        j.at("symbol").get_to(msg.symbol);

        if (j.contains("quote") && !j["quote"].is_null()) {
            msg.quote = j.at("quote").get<StockQuote>();
        }
        if (j.contains("error") && !j["error"].is_null()) {
            msg.error = j.at("error").get<std::string>();
        }
        if (j.contains("subscriptions") && !j["subscriptions"].is_null()) {
            msg.subscriptions = j.at("subscriptions").get<std::vector<std::string>>();
        }
        if (j.contains("priceHistory") && !j["priceHistory"].is_null()) {
            msg.priceHistory = j.at("priceHistory").get<std::vector<StockQuote>>();
        }
    }

    // MessageSocket implementation
    MessageSocket::MessageSocket(zmq::socket_type type)
        : context(std::make_unique<zmq::context_t>(1))
        , socket(std::make_unique<zmq::socket_t>(*context, type))
    {}

    void MessageSocket::bind(const std::string& endpoint) {
        socket->bind(endpoint);
        //spdlog::info("Socket bound to {}", endpoint);
    }

    void MessageSocket::connect(const std::string& endpoint) {
        socket->connect(endpoint);
        //spdlog::info("Socket connected to {}", endpoint);
    }

    void MessageSocket::send(const Message& msg) {
        json j;
        to_json(j, msg);
        std::string serialized = j.dump();
        socket->send(zmq::buffer(serialized), zmq::send_flags::none);
    }

    std::optional<Message> MessageSocket::receive(bool nonBlocking) {
        zmq::message_t zmq_msg;
        auto flags = nonBlocking ? zmq::recv_flags::dontwait : zmq::recv_flags::none;

        if (socket->recv(zmq_msg, flags)) {
            std::string serialized(static_cast<char*>(zmq_msg.data()), zmq_msg.size());
            try {
                auto j = nlohmann::json::parse(serialized);
                Message msg;
                from_json(j, msg);
                return msg;
            }
            catch (const std::exception& e) {
                spdlog::error("Failed to parse message: {}", e.what());
            }
        }
        return std::nullopt;
    }
}