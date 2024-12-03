#include "StockTracker/Types.h"

namespace StockTracker {
	
	StockQuote StockQuote::create(std::string sym, double price_val) {
		return {
			std::move(sym),
			price_val,
			std::chrono::system_clock::now(),
			std::nullopt,
			"USD"
		};
	}

	void to_json(json& j, const StockQuote& quote) {
		j = json{
			{"symbol", quote.symbol},
			{"price", quote.price},
			{"timestamp", quote.timestamp.time_since_epoch().count()},
			{"currency", quote.currency}
		};

		if (quote.change_percent.has_value()) {
			j["change_percent"] = quote.change_percent.value();
		}
	}

	void from_json(const json& j, StockQuote& quote) {
		j.at("symbol").get_to(quote.symbol);
		j.at("price").get_to(quote.price);
		auto ts = j.at("timestamp").get<int64_t>();
		quote.timestamp = std::chrono::system_clock::time_point(std::chrono::milliseconds(ts));

		// fallback to USD
		quote.currency = j.value("currency", "USD");


		if (j.contains("change_percent") && !j["change_percent"].is_null()) {
			quote.change_percent = j["change_percent"].get<double>();
		}
		else {
			quote.change_percent = std::nullopt;
		}
	}
	
}