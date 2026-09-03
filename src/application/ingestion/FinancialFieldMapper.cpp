#include "application/ingestion/FinancialFieldMapper.h"

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <limits>

namespace fincon
{
    namespace
    {
        using Json = nlohmann::json;

        const Json* findField(const Json& record,
                              const std::vector<std::string>& names)
        {
            const Json* match = nullptr;
            for (const auto& name : names)
            {
                if (!record.contains(name))
                    continue;
                if (match != nullptr)
                    return nullptr;
                match = &record.at(name);
            }
            return match;
        }

        std::string stringField(const Json& record,
                                const std::vector<std::string>& names)
        {
            const Json* value = findField(record, names);
            return value != nullptr && value->is_string()
                ? value->get<std::string>() : std::string{};
        }

        std::int64_t amountField(const Json& record,
                                 const std::vector<std::string>& names,
                                 bool& ambiguous)
        {
            const Json* value = findField(record, names);
            if (value == nullptr)
            {
                ambiguous = false;
                return 0;
            }
            if (!value->is_number_integer())
            {
                ambiguous = true;
                return 0;
            }
            const auto amount = value->get<std::int64_t>();
            if (amount < 0)
                ambiguous = true;
            return amount;
        }

        bool validId(const std::string& value)
        {
            return value.size() >= 3 && value.size() <= 128 &&
                value.find_first_not_of(
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_"
                ) == std::string::npos;
        }
    }

    MappingResult FinancialFieldMapper::map(const nlohmann::json& payload) const
    {
        MappingResult result;
        if (!payload.is_object() || !payload.contains("batchId") ||
            !payload.at("batchId").is_string())
        {
            result.status = "REQUIRES_MAPPING";
            result.reason = "A valid batchId is required.";
            return result;
        }

        result.batchId = payload.at("batchId").get<std::string>();
        if (!validId(result.batchId))
        {
            result.status = "REQUIRES_MAPPING";
            result.reason = "batchId contains unsupported characters.";
            return result;
        }

        const Json* records = payload.contains("records")
            ? &payload.at("records") : nullptr;
        if (records == nullptr || !records->is_array() || records->empty())
        {
            result.status = "REQUIRES_MAPPING";
            result.reason = "records must be a non-empty JSON array.";
            return result;
        }

        FinancialDataBatch batch;
        batch.batchId = result.batchId;
        std::size_t recordIndex = 0;
        std::vector<std::string> aggregatedMissing;
        bool hasInvalidAmount = false;
        for (const Json& record : *records)
        {
            if (!record.is_object())
            {
                result.reason = "Every record must be a JSON object at index " + std::to_string(recordIndex) + ".";
                result.status = "REQUIRES_MAPPING";
                return result;
            }

            bool invalidAmount = false;
            const std::string paymentId = stringField(
                record,
                {"id", "payment_id", "paymentId", "txn_id",
                 "transaction_id", "transactionReference", "paymentReference"}
            );
            const std::int64_t amount = amountField(
                record,
                {"amount", "amount_paise", "amountPaise", "gross",
                 "gross_amount", "transaction_amount"},
                invalidAmount
            );
            std::vector<std::string> missing;
            if (!validId(paymentId))
                missing.push_back("paymentId[" + std::to_string(recordIndex) + "]");
            if (invalidAmount || amount <= 0)
                missing.push_back("amount[" + std::to_string(recordIndex) + "]");

            if (!missing.empty())
            {
                aggregatedMissing.insert(aggregatedMissing.end(), missing.begin(), missing.end());
                if (invalidAmount) hasInvalidAmount = true;
            }
            else
            {
                Payment payment;
                payment.id = paymentId;
                payment.merchantId = stringField(record, {"merchantId", "merchant_id"});
                payment.orderId = stringField(record, {"orderId", "order_id", "orderNumber", "merchant_ref"});
                payment.amount = amount;
                payment.status = PaymentStatus::Captured;
                batch.data.payments.push_back(std::move(payment));

                Order order;
                order.id = batch.data.payments.back().orderId.empty()
                    ? "ORD-" + paymentId : batch.data.payments.back().orderId;
                order.merchantId = batch.data.payments.back().merchantId;
                order.amount = amount;
                order.status = OrderStatus::Paid;
                batch.data.orders.push_back(std::move(order));
            }
            ++recordIndex;
        }

        if (!aggregatedMissing.empty())
        {
            result.missingFields = std::move(aggregatedMissing);
            result.status = "REQUIRES_MAPPING";
            result.reason = hasInvalidAmount
                ? "Amount must be a positive integer paise value."
                : "Required paymentId and amount fields are missing or ambiguous.";
            return result;
        }

        result.accepted = true;
        result.status = "MAPPED";
        result.batch = std::move(batch);
        return result;
    }
}