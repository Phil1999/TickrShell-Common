#pragma once
#include "Types.h"
#include <zmq.hpp>
#include <memory>
#include <optional>
#include <vector>
#include <spdlog/spdlog.h>

namespace StockTracker {
    // Message class
    class Message {
    public:
        MessageType type;
        std::string symbol;
        std::optional<StockQuote> quote;
        std::optional<std::string> error;
        std::optional<std::vector<StockQuote>> priceHistory;
        std::optional<std::vector<std::string>> subscriptions; // For subscriptions list
        std::string currency;

        // Static factory methods (declarations only)
        static Message makeSubscribe(std::string symbol);
        static Message makeUnsubscribe(std::string symbol);
        static Message makeQuoteUpdate(StockQuote quote);
        static Message makeQuery(std::string symbol);
        static Message makeError(std::string error);
        static Message makeRequestSubscriptions();   // Request list of subscriptions
        static Message makeSubscriptionsList(const std::vector<std::string>& subscriptions);
        static Message makeRequestPriceHistory(const std::string& symbol);  // Request price history
        static Message makePriceHistory(const std::string& symbol, const std::vector<StockQuote>& history);
        static Message makeSetCurrency(std::string currency_code);
    };

    // JSON serialization declarations
    void to_json(json& j, const Message& msg);
    void from_json(const json& j, Message& msg);

    // MessageSocket class
    class MessageSocket {
    private:
        std::unique_ptr<zmq::context_t> context;
        std::unique_ptr<zmq::socket_t> socket;

    public:
        explicit MessageSocket(zmq::socket_type type);
        void bind(const std::string& endpoint);
        void connect(const std::string& endpoint);
        void send(const Message& msg);
        std::optional<Message> receive(bool nonBlocking = false);

        // For string options specifically (like subscribe)
        void setSubscribe(const std::string& topic = "") {
            socket->set(zmq::sockopt::subscribe, topic);
        }

        // Add this for timeout
        void setTimeout(int timeout_ms) {
            socket->set(zmq::sockopt::rcvtimeo, timeout_ms);
        }

        zmq::socket_t& getSocket() { return *socket; }
    };
}