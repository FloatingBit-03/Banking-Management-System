#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "banking/domain/entities.hpp"
#include "banking/domain/errors.hpp"
#include "banking/infrastructure/account_repository.hpp"

namespace banking::application {

class BankingService {
public:
    explicit BankingService(std::shared_ptr<infrastructure::AccountRepository> repository);

    domain::Customer create_customer(std::string name, std::string email);
    std::vector<domain::Customer> list_customers() const;

    std::pair<domain::Account, std::optional<domain::LedgerEntry>> open_account(
        std::int64_t customer_id,
        std::string currency,
        std::int64_t initial_deposit_cents,
        std::string memo = "Account opening",
        std::optional<std::string> idempotency_key = std::nullopt);

    std::vector<domain::Account> list_accounts(std::optional<std::int64_t> customer_id = std::nullopt) const;
    domain::LedgerEntry deposit(std::int64_t account_id,
                                std::int64_t amount_cents,
                                std::string memo,
                                std::optional<std::string> idempotency_key = std::nullopt);
    domain::LedgerEntry withdraw(std::int64_t account_id,
                                 std::int64_t amount_cents,
                                 std::string memo,
                                 std::optional<std::string> idempotency_key = std::nullopt);
    domain::TransferResult transfer(std::int64_t source_id,
                                    std::int64_t target_id,
                                    std::int64_t amount_cents,
                                    std::string memo,
                                    std::optional<std::string> idempotency_key = std::nullopt);

    std::vector<domain::LedgerEntry> list_ledger(std::optional<std::int64_t> account_id = std::nullopt) const;

private:
    static std::string normalize_currency(const std::string& currency);
    static std::string normalize_email(const std::string& email);
    static void ensure_not_blank(const std::string& value, const std::string& field);
    static void ensure_positive(std::int64_t amount_cents, const std::string& field);

    domain::Customer require_customer(std::int64_t customer_id) const;
    domain::Account require_account(std::int64_t account_id) const;
    domain::LedgerEntry record_entry(std::int64_t account_id,
                                     domain::EntryType type,
                                     std::int64_t amount_cents,
                                     const std::string& currency,
                                     const std::string& memo,
                                     const std::optional<std::string>& idempotency_key);

    std::shared_ptr<infrastructure::AccountRepository> repository_;
};

}  // namespace banking::application
