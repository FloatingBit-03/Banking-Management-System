#include "banking/infrastructure/in_memory_repository.hpp"

#include <algorithm>
#include <chrono>

namespace banking::infrastructure {

namespace {
std::chrono::system_clock::time_point now_utc() {
    return std::chrono::system_clock::now();
}
}  // namespace

domain::Customer InMemoryRepository::create_customer(std::string name, std::string email) {
    std::lock_guard lock(mutex_);
    domain::Customer customer{next_customer_id_++, std::move(name), std::move(email), now_utc()};
    customers_[customer.id] = customer;
    return customer;
}

std::vector<domain::Customer> InMemoryRepository::list_customers() const {
    std::lock_guard lock(mutex_);
    std::vector<domain::Customer> out;
    out.reserve(customers_.size());
    for (const auto& [_, customer] : customers_) {
        out.push_back(customer);
    }
    std::sort(out.begin(), out.end(), [](const auto& a, const auto& b) { return a.id < b.id; });
    return out;
}

std::optional<domain::Customer> InMemoryRepository::find_customer(std::int64_t customer_id) const {
    std::lock_guard lock(mutex_);
    const auto it = customers_.find(customer_id);
    if (it == customers_.end()) {
        return std::nullopt;
    }
    return it->second;
}

domain::Account InMemoryRepository::create_account(std::int64_t customer_id,
                                                   std::string currency,
                                                   std::int64_t initial_balance_cents) {
    std::lock_guard lock(mutex_);
    domain::Account account{next_account_id_++, customer_id, std::move(currency), initial_balance_cents, true, now_utc()};
    accounts_[account.id] = account;
    return account;
}

std::vector<domain::Account> InMemoryRepository::list_accounts(std::optional<std::int64_t> customer_id) const {
    std::lock_guard lock(mutex_);
    std::vector<domain::Account> out;
    out.reserve(accounts_.size());
    for (const auto& [_, account] : accounts_) {
        if (!customer_id || account.customer_id == *customer_id) {
            out.push_back(account);
        }
    }
    std::sort(out.begin(), out.end(), [](const auto& a, const auto& b) { return a.id < b.id; });
    return out;
}

std::optional<domain::Account> InMemoryRepository::find_account(std::int64_t account_id) const {
    std::lock_guard lock(mutex_);
    const auto it = accounts_.find(account_id);
    if (it == accounts_.end()) {
        return std::nullopt;
    }
    return it->second;
}

void InMemoryRepository::save_account(const domain::Account& account) {
    std::lock_guard lock(mutex_);
    accounts_[account.id] = account;
}

domain::LedgerEntry InMemoryRepository::append_ledger_entry(std::int64_t account_id,
                                                            domain::EntryType type,
                                                            std::int64_t amount_cents,
                                                            std::string currency,
                                                            std::string memo,
                                                            std::optional<std::string> idempotency_key) {
    std::lock_guard lock(mutex_);
    domain::LedgerEntry entry{next_ledger_id_++,
                              account_id,
                              type,
                              amount_cents,
                              std::move(currency),
                              std::move(memo),
                              std::move(idempotency_key),
                              now_utc()};
    ledger_.push_back(entry);
    if (entry.idempotency_key) {
        idempotency_index_[*entry.idempotency_key] = entry.id;
    }
    return entry;
}

std::vector<domain::LedgerEntry> InMemoryRepository::list_ledger(std::optional<std::int64_t> account_id) const {
    std::lock_guard lock(mutex_);
    if (!account_id) {
        return ledger_;
    }
    std::vector<domain::LedgerEntry> out;
    for (const auto& entry : ledger_) {
        if (entry.account_id == *account_id) {
            out.push_back(entry);
        }
    }
    return out;
}

std::optional<domain::LedgerEntry> InMemoryRepository::find_ledger_by_idempotency(const std::string& key) const {
    std::lock_guard lock(mutex_);
    const auto idx = idempotency_index_.find(key);
    if (idx == idempotency_index_.end()) {
        return std::nullopt;
    }
    const auto it = std::find_if(ledger_.begin(), ledger_.end(),
                                 [&](const domain::LedgerEntry& entry) { return entry.id == idx->second; });
    if (it == ledger_.end()) {
        return std::nullopt;
    }
    return *it;
}

}  // namespace banking::infrastructure
