#include "StockTracker/DatabaseService.h"
#include <spdlog/spdlog.h>
#include <stdexcept>

namespace StockTracker {

    DatabaseService::DatabaseService(const std::string& db_path) {
        if (sqlite3_open(db_path.c_str(), &db) != SQLITE_OK) {
            throw std::runtime_error("Failed to open database");
        }
        initializeTables();
    }

    DatabaseService::~DatabaseService() {
        if (db) {
            sqlite3_close(db);
        }
    }

    void DatabaseService::initializeTables() {
        const char* sql = R"(
        CREATE TABLE IF NOT EXISTS price_history (
            symbol TEXT NOT NULL,
            price REAL NOT NULL,
            timestamp INTEGER NOT NULL,
            change_percent REAL
        );

        CREATE TABLE IF NOT EXISTS subscriptions (
            symbol TEXT PRIMARY KEY,
            added_at INTEGER NOT NULL
        );
    )";

        char* errMsg = nullptr;
        if (sqlite3_exec(db, sql, nullptr, nullptr, &errMsg) != SQLITE_OK) {
            std::string error = errMsg;
            sqlite3_free(errMsg);
            throw std::runtime_error("Failed to create tables: " + error);
        }
    }

    void DatabaseService::savePrice(const StockQuote& quote) {
        const char* sql = R"(
        INSERT INTO price_history (symbol, price, timestamp, change_percent)
        VALUES (?, ?, ?, ?)
    )";

        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
            throw std::runtime_error("Failed to prepare statement");
        }

        auto timestamp = quote.timestamp.time_since_epoch().count();
        sqlite3_bind_text(stmt, 1, quote.symbol.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_double(stmt, 2, quote.price);
        sqlite3_bind_int64(stmt, 3, timestamp);
        if (quote.change_percent) {
            sqlite3_bind_double(stmt, 4, *quote.change_percent);
        }
        else {
            sqlite3_bind_null(stmt, 4);
        }

        if (sqlite3_step(stmt) != SQLITE_DONE) {
            sqlite3_finalize(stmt);
            throw std::runtime_error("Failed to save price");
        }

        sqlite3_finalize(stmt);
    }

    std::vector<StockQuote> DatabaseService::getPriceHistory(const std::string& symbol, int limit) {
        const char* sql = R"(
        SELECT symbol, price, timestamp, change_percent
        FROM price_history
        WHERE symbol = ?
        ORDER BY timestamp DESC
        LIMIT ?
    )";

        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
            throw std::runtime_error("Failed to prepare statement");
        }

        sqlite3_bind_text(stmt, 1, symbol.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 2, limit);

        std::vector<StockQuote> history;
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            StockQuote quote;
            quote.symbol = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
            quote.price = sqlite3_column_double(stmt, 1);
            auto timestamp = sqlite3_column_int64(stmt, 2);
            quote.timestamp = std::chrono::system_clock::time_point(
                std::chrono::milliseconds(timestamp));

            if (sqlite3_column_type(stmt, 3) != SQLITE_NULL) {
                quote.change_percent = sqlite3_column_double(stmt, 3);
            }

            history.push_back(quote);
        }

        sqlite3_finalize(stmt);
        return history;
    }

    void DatabaseService::saveSubscription(const std::string& symbol) {
        const char* sql = R"(
        INSERT OR REPLACE INTO subscriptions (symbol, added_at)
        VALUES (?, ?)
    )";

        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
            throw std::runtime_error("Failed to prepare statement");
        }

        auto now = std::chrono::system_clock::now().time_since_epoch().count();
        sqlite3_bind_text(stmt, 1, symbol.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_int64(stmt, 2, now);

        if (sqlite3_step(stmt) != SQLITE_DONE) {
            sqlite3_finalize(stmt);
            throw std::runtime_error("Failed to save subscription");
        }

        sqlite3_finalize(stmt);
    }

    void DatabaseService::removeSubscription(const std::string& symbol) {
        const char* sql = "DELETE FROM subscriptions WHERE symbol = ?";

        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
            throw std::runtime_error("Failed to prepare statement");
        }

        sqlite3_bind_text(stmt, 1, symbol.c_str(), -1, SQLITE_STATIC);

        if (sqlite3_step(stmt) != SQLITE_DONE) {
            sqlite3_finalize(stmt);
            throw std::runtime_error("Failed to remove subscription");
        }

        sqlite3_finalize(stmt);
    }

    std::vector<std::string> DatabaseService::getSubscriptions() {
        const char* sql = "SELECT symbol FROM subscriptions ORDER BY symbol";

        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
            throw std::runtime_error("Failed to prepare statement");
        }

        std::vector<std::string> subscriptions;
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            subscriptions.push_back(
                reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0))
            );
        }

        sqlite3_finalize(stmt);
        return subscriptions;
    }

} // namespace StockTracker