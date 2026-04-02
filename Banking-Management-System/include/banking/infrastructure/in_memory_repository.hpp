#pragma once

#include <mutex>
#include <unordered_map>

#include "banking/infrastructure/account_repository.hpp"

namespace banking::infrastructure {

class InMemoryRepository final : public AccountRepository {
public:
    domain::Customer create_customer(std::string name, std::string email) override;
    std::vector<domain::Customer> list_customers() const override;
    std::optional<domain::Customer> find_customer(std::int64_t customer_id) const override;

    domain::Account create_account(std::int64_t customer_id, std::string currency, std::int64_t initial_balance_cents) override;
    std::vector<domain::Account> list_accounts(std::optional<std::int64_t> customer_id) const override;
    std::optional<domain::Account> find_account(std::int64_t account_id) const override;
    void save_account(const domain::Account& account) override;

    domain::LedgerEntry append_ledger_entry(std::int64_t account_id,
                                            domain::EntryType type,
                                            std::int64_t amount_cents,
                                            std::string currency,
                                            std::string memo,
                                            std::optional<std::string> idempotency_key) override;
    std::vector<domain::LedgerEntry> list_ledger(std::optional<std::int64_t> account_id) const override;
    std::optional<domain::LedgerEntry> find_ledger_by_idempotency(const std::string& key) const override;

private:
    mutable std::mutex mutex_;
    std::int64_t next_customer_id_{1};
    std::int64_t next_account_id_{1};
    std::int64_t next_ledger_id_{1};

    std::unordered_map<std::int64_t, domain::Customer> customers_;
    std::unordered_map<std::int64_t, domain::Account> accounts_;
    std::vector<domain::LedgerEntry> ledger_;
    std::unordered_map<std::string, std::int64_t> idempotency_index_;
};

}  // namespace banking::infrastructure
