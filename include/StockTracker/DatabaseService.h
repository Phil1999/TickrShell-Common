#pragma once
#include "Types.h"
#include <sqlite3.h>
#include <string>
#include <vector>
#include <memory>

namespace StockTracker {

    class DatabaseService {
    public:
        DatabaseService(const std::string& db_path = "stocktracker.db");
        ~DatabaseService();

        // Stock price history
        void savePrice(const StockQuote& quote);
        std::vector<StockQuote> getPriceHistory(const std::string& symbol, int limit = 5);

        // Subscriptions
        void saveSubscription(const std::string& symbol);
        void removeSubscription(const std::string& symbol);
        std::vector<std::string> getSubscriptions();

    private:
        sqlite3* db{ nullptr };
        void initializeTables();
    };

}