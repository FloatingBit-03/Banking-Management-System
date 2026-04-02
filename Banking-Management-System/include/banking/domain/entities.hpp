#pragma once

#include <chrono>
#include <cstdint>
#include <optional>
#include <string>

namespace banking::domain {

enum class EntryType {
    Deposit,
    Withdrawal,
    TransferDebit,
    TransferCredit
};

struct Customer {
    std::int64_t id{};
    std::string name;
    std::string email;
    std::chrono::system_clock::time_point created_at;
};

struct Account {
    std::int64_t id{};
    std::int64_t customer_id{};
    std::string currency;
    std::int64_t balance_cents{};
    bool active{true};
    std::chrono::system_clock::time_point created_at;
};

struct LedgerEntry {
    std::int64_t id{};
    std::int64_t account_id{};
    EntryType entry_type{};
    std::int64_t amount_cents{};
    std::string currency;
    std::string memo;
    std::optional<std::string> idempotency_key;
    std::chrono::system_clock::time_point created_at;
};

struct TransferResult {
    Account source;
    Account target;
    LedgerEntry debit_entry;
    LedgerEntry credit_entry;
};

std::string to_string(EntryType type);
std::string format_money(std::int64_t cents);

}  // namespace banking::domain
