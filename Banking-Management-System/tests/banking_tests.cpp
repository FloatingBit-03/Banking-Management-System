#include <cassert>
#include <memory>

#include "banking/application/banking_service.hpp"
#include "banking/infrastructure/in_memory_repository.hpp"

namespace {

banking::application::BankingService build_service() {
    return banking::application::BankingService(std::make_shared<banking::infrastructure::InMemoryRepository>());
}

void test_deposit_and_withdraw() {
    auto service = build_service();
    const auto customer = service.create_customer("Ada", "ada@example.com");
    const auto [account, opening] = service.open_account(customer.id, "usd", 10'000);
    assert(opening.has_value());

    const auto deposit = service.deposit(account.id, 2'550, "ATM");
    assert(deposit.amount_cents == 2'550);

    const auto withdrawal = service.withdraw(account.id, 5'000, "Teller");
    assert(withdrawal.amount_cents == 5'000);

    const auto accounts = service.list_accounts(customer.id);
    assert(accounts.size() == 1);
    assert(accounts.front().balance_cents == 7'550);
}

void test_transfer_and_same_account_validation() {
    auto service = build_service();
    const auto customer = service.create_customer("Grace", "grace@example.com");
    const auto [source, _] = service.open_account(customer.id, "USD", 20'000);
    const auto [target, __] = service.open_account(customer.id, "USD", 5'000);

    const auto result = service.transfer(source.id, target.id, 2'500, "Ops");
    assert(result.debit_entry.amount_cents == 2'500);
    assert(result.credit_entry.amount_cents == 2'500);

    bool threw = false;
    try {
        static_cast<void>(service.transfer(source.id, source.id, 100, "Invalid"));
    } catch (const banking::domain::ValidationError&) {
        threw = true;
    }
    assert(threw);
}

void test_idempotency() {
    auto service = build_service();
    const auto customer = service.create_customer("Linus", "linus@example.com");
    const auto [account, _] = service.open_account(customer.id, "USD", 5'000);

    const auto first = service.deposit(account.id, 1'000, "Retry", "key-1");
    const auto second = service.deposit(account.id, 1'000, "Retry", "key-1");

    assert(first.id == second.id);

    const auto ledger = service.list_ledger(account.id);
    assert(ledger.size() == 2);
}

}  // namespace

int main() {
    test_deposit_and_withdraw();
    test_transfer_and_same_account_validation();
    test_idempotency();
    return 0;
}
