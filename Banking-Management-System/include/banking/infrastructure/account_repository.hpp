#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include "banking/domain/entities.hpp"

namespace banking::infrastructure {

class AccountRepository {
public:
    virtual ~AccountRepository() = default;

    virtual domain::Customer create_customer(std::string name, std::string email) = 0;
    virtual std::vector<domain::Customer> list_customers() const = 0;
    virtual std::optional<domain::Customer> find_customer(std::int64_t customer_id) const = 0;

    virtual domain::Account create_account(std::int64_t customer_id, std::string currency, std::int64_t initial_balance_cents) = 0;
    virtual std::vector<domain::Account> list_accounts(std::optional<std::int64_t> customer_id) const = 0;
    virtual std::optional<domain::Account> find_account(std::int64_t account_id) const = 0;
    virtual void save_account(const domain::Account& account) = 0;

    virtual domain::LedgerEntry append_ledger_entry(std::int64_t account_id,
                                                    domain::EntryType type,
                                                    std::int64_t amount_cents,
                                                    std::string currency,
                                                    std::string memo,
                                                    std::optional<std::string> idempotency_key) = 0;
    virtual std::vector<domain::LedgerEntry> list_ledger(std::optional<std::int64_t> account_id) const = 0;
    virtual std::optional<domain::LedgerEntry> find_ledger_by_idempotency(const std::string& key) const = 0;
};

}  // namespace banking::infrastructure
