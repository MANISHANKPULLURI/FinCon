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

        Timestamp parseTimestamp(const std::string& s)
        {
            if (s.empty()) return Timestamp{};
            int y=0,m=0,d=0,hh=0,mm=0,ss=0;
            if (sscanf(s.c_str(), "%d-%d-%dT%d:%d:%d", &y,&m,&d,&hh,&mm,&ss)==6)
            {
                std::tm tm{};
                tm.tm_year=y-1900; tm.tm_mon=m-1; tm.tm_mday=d;
                tm.tm_hour=hh; tm.tm_min=mm; tm.tm_sec=ss;
                tm.tm_isdst=0;
#ifdef __APPLE__
                time_t t = timegm(&tm);
#else
                time_t t = timegm(&tm);
#endif
                if (t!=-1) return std::chrono::system_clock::from_time_t(t);
            }
            return std::chrono::system_clock::now();
        }

        void parseSettlements(const Json& payload, FinancialDataset& data)
        {
            const Json* arr = nullptr;
            if (payload.contains("settlements") && payload.at("settlements").is_array()) arr = &payload.at("settlements");
            else if (payload.contains("settlement") && payload.at("settlement").is_array()) arr = &payload.at("settlement");
            if (!arr) return;
            for (const auto& r : *arr)
            {
                if (!r.is_object()) continue;
                std::string sid = stringField(r, {"id","settlement_id","settlementId"});
                if (!validId(sid)) continue;
                bool bad=false;
                std::int64_t gross = amountField(r, {"grossAmount","gross_amount","gross","amount"}, bad);
                std::int64_t refund = 0, fee=0, net=0;
                bool t=false;
                std::int64_t rv = amountField(r, {"refundAmount","refund_amount","refund"}, t);
                if (!t) refund=rv;
                t=false; rv = amountField(r, {"feeAmount","fee_amount","fee"}, t);
                if (!t) fee=rv;
                t=false; rv = amountField(r, {"netAmount","net_amount","net"}, t);
                if (!t) net=rv; else net=gross-refund-fee;
                if (bad || gross<=0) continue;
                Settlement s;
                s.id=sid;
                s.merchantId=stringField(r, {"merchantId","merchant_id"});
                s.grossAmount=gross;
                s.refundAmount=refund;
                s.feeAmount=fee;
                s.netAmount=net;
                s.status=SettlementStatus::Settled;
                {
                    std::string ca = stringField(r, {"createdAt","created_at","created"});
                    std::string sa = stringField(r, {"settledAt","settled_at","settled","settlementTime"});
                    if (!ca.empty()) s.createdAt = parseTimestamp(ca);
                    if (!sa.empty()) s.settledAt = parseTimestamp(sa);
                    else if (!ca.empty()) s.settledAt = s.createdAt + std::chrono::hours(24);
                }
                if (r.contains("paymentIds") && r.at("paymentIds").is_array())
                    for (auto& pid : r.at("paymentIds")) if (pid.is_string()) s.paymentIds.push_back(pid.get<std::string>());
                else if (r.contains("paymentId") && r.at("paymentId").is_string()) s.paymentIds.push_back(r.at("paymentId").get<std::string>());
                else if (r.contains("payment_id") && r.at("payment_id").is_string()) s.paymentIds.push_back(r.at("payment_id").get<std::string>());
                if (s.paymentIds.empty() && r.contains("paymentIds")==false) {}
                data.settlements.push_back(std::move(s));
            }
        }
        void parseBank(const Json& payload, FinancialDataset& data)
        {
            const Json* arr=nullptr;
            if (payload.contains("bankTransactions") && payload.at("bankTransactions").is_array()) arr=&payload.at("bankTransactions");
            else if (payload.contains("bank_transactions") && payload.at("bank_transactions").is_array()) arr=&payload.at("bank_transactions");
            if (!arr) return;
            for (const auto& r:*arr){ if(!r.is_object()) continue; std::string id=stringField(r,{"id","bankTransactionId","transactionId"}); if(!validId(id)) continue; bool bad=false; std::int64_t amt=amountField(r,{"amount","amountPaise","value"},bad); if(bad||amt<=0) continue; BankTransaction b; b.id=id; b.merchantId=stringField(r,{"merchantId","merchant_id"}); b.settlementId=stringField(r,{"settlementId","settlement_id","settlement"}); b.amount=amt; b.type=BankTransactionType::Credit; { std::string pa = stringField(r, {"postedAt","posted_at","posted","timestamp"}); if (!pa.empty()) b.postedAt = parseTimestamp(pa); } data.bankTransactions.push_back(std::move(b)); }
        }
        void parseAccounting(const Json& payload, FinancialDataset& data)
        {
            const Json* arr=nullptr;
            if (payload.contains("accountingEntries") && payload.at("accountingEntries").is_array()) arr=&payload.at("accountingEntries");
            else if (payload.contains("accounting_entries") && payload.at("accounting_entries").is_array()) arr=&payload.at("accounting_entries");
            if (!arr) return;
            for (const auto& r:*arr){ if(!r.is_object()) continue; std::string id=stringField(r,{"id","entryId"}); if(!validId(id)) continue; bool bad=false; std::int64_t amt=amountField(r,{"amount","amountPaise","value"},bad); if(bad||amt<=0) continue; AccountingEntry a; a.id=id; a.merchantId=stringField(r,{"merchantId","merchant_id"}); a.reference=stringField(r,{"reference","settlementId","settlement_id"}); a.amount=amt; std::string tp=stringField(r,{"type"}); a.type=(tp=="debit"||tp=="Debit")?AccountingEntryType::Debit:AccountingEntryType::Credit; { std::string ca = stringField(r, {"createdAt","created_at"}); if (!ca.empty()) a.createdAt = parseTimestamp(ca); } data.accountingEntries.push_back(std::move(a)); }
        }
        void parseRefunds(const Json& payload, FinancialDataset& data)
        {
            const Json* arr=nullptr;
            if (payload.contains("refunds") && payload.at("refunds").is_array()) arr=&payload.at("refunds");
            if (!arr) return;
            for (const auto& r:*arr){ if(!r.is_object()) continue; std::string id=stringField(r,{"id","refundId"}); if(!validId(id)) continue; bool bad=false; std::int64_t amt=amountField(r,{"amount","amountPaise"},bad); if(bad||amt<=0) continue; Refund rf; rf.id=id; rf.merchantId=stringField(r,{"merchantId","merchant_id"}); rf.paymentId=stringField(r,{"paymentId","payment_id"}); rf.amount=amt; rf.status=RefundStatus::Processed; data.refunds.push_back(std::move(rf)); }
        }
        void parseFees(const Json& payload, FinancialDataset& data)
        {
            const Json* arr=nullptr;
            if (payload.contains("fees") && payload.at("fees").is_array()) arr=&payload.at("fees");
            if (!arr) return;
            for (const auto& r:*arr){ if(!r.is_object()) continue; std::string id=stringField(r,{"id","feeId"}); if(!validId(id)) continue; bool bad=false; std::int64_t amt=amountField(r,{"amount","amountPaise"},bad); if(bad||amt<=0) continue; Fee f; f.id=id; f.merchantId=stringField(r,{"merchantId","merchant_id"}); f.paymentId=stringField(r,{"paymentId","payment_id"}); f.amount=amt; f.type=FeeType::Processing; data.fees.push_back(std::move(f)); }
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

        parseSettlements(payload, batch.data);
        parseBank(payload, batch.data);
        parseAccounting(payload, batch.data);
        parseRefunds(payload, batch.data);
        parseFees(payload, batch.data);

        result.accepted = true;
        result.status = "MAPPED";
        result.batch = std::move(batch);
        return result;
    }
}
