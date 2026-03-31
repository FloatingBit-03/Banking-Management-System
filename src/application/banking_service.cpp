#include "banking/application/banking_service.hpp"

#include <algorithm>
#include <cctype>

namespace banking::application {

namespace {
std::string lowercase(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) { return std::tolower(c); });
    return value;
}
}  // namespace

BankingService::BankingService(std::shared_ptr<infrastructure::AccountRepository> repository)
    : repository_(std::move(repository)) {}

domain::Customer BankingService::create_customer(std::string name, std::string email) {
    ensure_not_blank(name, "name");
    ensure_not_blank(email, "email");

    const auto normalized_email = normalize_email(email);
    for (const auto& customer : repository_->list_customers()) {
        if (customer.email == normalized_email) {
            throw domain::ValidationError("Customer email already exists");
        }
    }

    return repository_->create_customer(std::move(name), normalized_email);
}

std::vector<domain::Customer> BankingService::list_customers() const {
    return repository_->list_customers();
}

std::pair<domain::Account, std::optional<domain::LedgerEntry>> BankingService::open_account(
    std::int64_t customer_id,
    std::string currency,
    std::int64_t initial_deposit_cents,
    std::string memo,
    std::optional<std::string> idempotency_key) {
    if (initial_deposit_cents < 0) {
        throw domain::ValidationError("Initial deposit cannot be negative");
    }
    require_customer(customer_id);

    auto account = repository_->create_account(customer_id, normalize_currency(currency), initial_deposit_cents);
    std::optional<domain::LedgerEntry> entry;

    if (initial_deposit_cents > 0) {
        entry = record_entry(account.id,
                             domain::EntryType::Deposit,
                             initial_deposit_cents,
                             account.currency,
                             memo,
                             idempotency_key);
    }

    return {account, entry};
}

std::vector<domain::Account> BankingService::list_accounts(std::optional<std::int64_t> customer_id) const {
    return repository_->list_accounts(customer_id);
}

domain::LedgerEntry BankingService::deposit(std::int64_t account_id,
                                            std::int64_t amount_cents,
                                            std::string memo,
                                            std::optional<std::string> idempotency_key) {
    ensure_positive(amount_cents, "Deposit amount");
    auto account = require_account(account_id);
    account.balance_cents += amount_cents;
    repository_->save_account(account);
    return record_entry(account.id, domain::EntryType::Deposit, amount_cents, account.currency, memo, idempotency_key);
}

domain::LedgerEntry BankingService::withdraw(std::int64_t account_id,
                                             std::int64_t amount_cents,
                                             std::string memo,
                                             std::optional<std::string> idempotency_key) {
    ensure_positive(amount_cents, "Withdrawal amount");
    auto account = require_account(account_id);
    if (account.balance_cents < amount_cents) {
        throw domain::ValidationError("Insufficient funds");
    }
    account.balance_cents -= amount_cents;
    repository_->save_account(account);
    return record_entry(account.id, domain::EntryType::Withdrawal, amount_cents, account.currency, memo, idempotency_key);
}

domain::TransferResult BankingService::transfer(std::int64_t source_id,
                                                std::int64_t target_id,
                                                std::int64_t amount_cents,
                                                std::string memo,
                                                std::optional<std::string> idempotency_key) {
    ensure_positive(amount_cents, "Transfer amount");
    if (source_id == target_id) {
        throw domain::ValidationError("Source and target accounts must be different");
    }

    auto source = require_account(source_id);
    auto target = require_account(target_id);

    if (source.currency != target.currency) {
        throw domain::ValidationError("Currency mismatch between accounts");
    }
    if (source.balance_cents < amount_cents) {
        throw domain::ValidationError("Insufficient funds");
    }

    source.balance_cents -= amount_cents;
    target.balance_cents += amount_cents;

    repository_->save_account(source);
    repository_->save_account(target);

    const auto debit_key = idempotency_key ? std::make_optional(*idempotency_key + "-debit") : std::nullopt;
    const auto credit_key = idempotency_key ? std::make_optional(*idempotency_key + "-credit") : std::nullopt;

    const auto debit = record_entry(source.id,
                                    domain::EntryType::TransferDebit,
                                    amount_cents,
                                    source.currency,
                                    memo,
                                    debit_key);
    const auto credit = record_entry(target.id,
                                     domain::EntryType::TransferCredit,
                                     amount_cents,
                                     target.currency,
                                     memo,
                                     credit_key);

    return domain::TransferResult{source, target, debit, credit};
}

std::vector<domain::LedgerEntry> BankingService::list_ledger(std::optional<std::int64_t> account_id) const {
    return repository_->list_ledger(account_id);
}

std::string BankingService::normalize_currency(const std::string& currency) {
    ensure_not_blank(currency, "currency");
    std::string normalized = currency;
    std::transform(normalized.begin(), normalized.end(), normalized.begin(),
                   [](unsigned char c) { return static_cast<char>(std::toupper(c)); });

    if (normalized.size() != 3 || !std::all_of(normalized.begin(), normalized.end(), [](unsigned char c) { return std::isalpha(c); })) {
        throw domain::ValidationError("Currency must be a 3-letter ISO code");
    }

    return normalized;
}

std::string BankingService::normalize_email(const std::string& email) {
    return lowercase(email);
}

void BankingService::ensure_not_blank(const std::string& value, const std::string& field) {
    if (value.empty() || std::all_of(value.begin(), value.end(), [](unsigned char c) { return std::isspace(c); })) {
        throw domain::ValidationError(field + " must not be blank");
    }
}

void BankingService::ensure_positive(std::int64_t amount_cents, const std::string& field) {
    if (amount_cents <= 0) {
        throw domain::ValidationError(field + " must be greater than zero");
    }
}

domain::Customer BankingService::require_customer(std::int64_t customer_id) const {
    const auto customer = repository_->find_customer(customer_id);
    if (!customer) {
        throw domain::ValidationError("Customer does not exist");
    }
    return *customer;
}

domain::Account BankingService::require_account(std::int64_t account_id) const {
    const auto account = repository_->find_account(account_id);
    if (!account) {
        throw domain::ValidationError("Account does not exist");
    }
    if (!account->active) {
        throw domain::ValidationError("Account is not active");
    }
    return *account;
}

domain::LedgerEntry BankingService::record_entry(std::int64_t account_id,
                                                  domain::EntryType type,
                                                  std::int64_t amount_cents,
                                                  const std::string& currency,
                                                  const std::string& memo,
                                                  const std::optional<std::string>& idempotency_key) {
    if (idempotency_key) {
        const auto existing = repository_->find_ledger_by_idempotency(*idempotency_key);
        if (existing) {
            return *existing;
        }
    }
    return repository_->append_ledger_entry(account_id, type, amount_cents, currency, memo, idempotency_key);
}

}  // namespace banking::application
