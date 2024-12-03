#pragma once
#include <string>
#include <chrono>
#include <optional>
#include <nlohmann/json.hpp>

namespace StockTracker {
    using json = nlohmann::json;

    struct StockQuote {
        std::string symbol; // Stock symbol (e.g. "AAPL")
        double price; // Current price of stock.
        std::chrono::system_clock::time_point timestamp; // When the price was recorded
        std::optional<double> change_percent; // Percent change
        std::string currency{ "USD" }; // USD default

        // Static factory method to easily create a StockQuote.
        static StockQuote create(std::string sym, double price_val);
    };

    // JSON serialization for StockQuote
    void to_json(json& j, const StockQuote& quote);
    void from_json(const json& j, StockQuote& quote);

    // Message types for service communication
    enum class MessageType {
        Subscribe,              // Client wants to start getting updates for a stock (subscribe AAPL)
        Unsubscribe,            // Stop getting updates (unsubscribe AAPL)
        Query,                  // Ask for current price (query AAPL)
        QuoteUpdate,            // new price update. When server needs to send price updates to subscribed clients.
        SubscriptionsList,      // Message containing list of current subscriptions.
        RequestSubscriptions,   // Request for list of subscriptions
        PriceHistoryRequest,    // Request price history for a stock.
        PriceHistoryResponse,   // Response to price history request, sends history back to CLI
        SetCurrency,            // Setting the currency.
        Error                   // Something went wrong
    };

    // Macro for JSON serialization for our MessageType enum.
    // Maps enum values to strings for JSON.
    // Ex: MessageType::Subscribe will become "subscribe" in JSON.
    NLOHMANN_JSON_SERIALIZE_ENUM(MessageType, {
        {MessageType::Subscribe, "subscribe"},
        {MessageType::Unsubscribe, "unsubscribe"},
        {MessageType::Query, "query"},
        {MessageType::QuoteUpdate, "quote_update"},
        {MessageType::PriceHistoryRequest, "price_history_request"},
        {MessageType::PriceHistoryResponse, "price_history_response"},
        {MessageType::SubscriptionsList, "subscriptions_list"},
        {MessageType::RequestSubscriptions, "request_subscriptions"},
        {MessageType::SetCurrency, "set_currency"},
        {MessageType::Error, "error"}
    })

}