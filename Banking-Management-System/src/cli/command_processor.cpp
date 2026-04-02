#include "banking/cli/command_processor.hpp"

#include <optional>
#include <sstream>
#include <string>

namespace banking::cli {

CommandProcessor::CommandProcessor(application::BankingService service) : service_(std::move(service)) {}

void CommandProcessor::print_help(std::ostream& output) {
    output << "Commands:\n"
           << "  customer_create <name> <email>\n"
           << "  customer_list\n"
           << "  account_open <customer_id> <currency> <initial_deposit_cents> [memo]\n"
           << "  account_list [customer_id]\n"
           << "  deposit <account_id> <amount_cents> [memo] [idempotency_key]\n"
           << "  withdraw <account_id> <amount_cents> [memo] [idempotency_key]\n"
           << "  transfer <source_id> <target_id> <amount_cents> [memo] [idempotency_key]\n"
           << "  ledger_list [account_id]\n"
           << "  help\n"
           << "  exit\n";
}

int CommandProcessor::run(std::istream& input, std::ostream& output) {
    print_help(output);

    std::string line;
    while (output << "> " && std::getline(input, line)) {
        if (line.empty()) {
            continue;
        }

        std::istringstream iss(line);
        std::string command;
        iss >> command;

        try {
            if (command == "exit") {
                return 0;
            }
            if (command == "help") {
                print_help(output);
            } else if (command == "customer_create") {
                std::string name;
                std::string email;
                iss >> name >> email;
                const auto customer = service_.create_customer(name, email);
                output << "Customer created: id=" << customer.id << " email=" << customer.email << '\n';
            } else if (command == "customer_list") {
                for (const auto& customer : service_.list_customers()) {
                    output << customer.id << ' ' << customer.name << ' ' << customer.email << '\n';
                }
            } else if (command == "account_open") {
                std::int64_t customer_id{};
                std::string currency;
                std::int64_t initial{};
                std::string memo = "Account opening";
                iss >> customer_id >> currency >> initial;
                if (iss >> memo) {
                }

                const auto [account, ledger] = service_.open_account(customer_id, currency, initial, memo);
                output << "Account opened: id=" << account.id << " balance=" << domain::format_money(account.balance_cents)
                       << ' ' << account.currency << '\n';
                if (ledger) {
                    output << "Opening ledger entry id=" << ledger->id << '\n';
                }
            } else if (command == "account_list") {
                std::optional<std::int64_t> customer_id;
                std::int64_t id{};
                if (iss >> id) {
                    customer_id = id;
                }
                for (const auto& account : service_.list_accounts(customer_id)) {
                    output << account.id << " customer=" << account.customer_id
                           << " balance=" << domain::format_money(account.balance_cents)
                           << ' ' << account.currency << '\n';
                }
            } else if (command == "deposit" || command == "withdraw") {
                std::int64_t account_id{};
                std::int64_t amount{};
                std::string memo = command == "deposit" ? "Deposit" : "Withdrawal";
                std::optional<std::string> key;
                iss >> account_id >> amount;
                if (iss >> memo) {
                    std::string maybe_key;
                    if (iss >> maybe_key) {
                        key = maybe_key;
                    }
                }

                const auto entry = (command == "deposit") ? service_.deposit(account_id, amount, memo, key)
                                                           : service_.withdraw(account_id, amount, memo, key);
                output << "Recorded: entry=" << entry.id << " type=" << domain::to_string(entry.entry_type)
                       << " amount=" << domain::format_money(entry.amount_cents) << '\n';
            } else if (command == "transfer") {
                std::int64_t source{};
                std::int64_t target{};
                std::int64_t amount{};
                std::string memo = "Transfer";
                std::optional<std::string> key;
                iss >> source >> target >> amount;
                if (iss >> memo) {
                    std::string maybe_key;
                    if (iss >> maybe_key) {
                        key = maybe_key;
                    }
                }
                const auto result = service_.transfer(source, target, amount, memo, key);
                output << "Transfer complete: debit=" << result.debit_entry.id << " credit=" << result.credit_entry.id
                       << " amount=" << domain::format_money(amount) << '\n';
            } else if (command == "ledger_list") {
                std::optional<std::int64_t> account_id;
                std::int64_t id{};
                if (iss >> id) {
                    account_id = id;
                }
                for (const auto& entry : service_.list_ledger(account_id)) {
                    output << entry.id << " acct=" << entry.account_id << " " << domain::to_string(entry.entry_type)
                           << " amount=" << domain::format_money(entry.amount_cents) << ' ' << entry.currency << '\n';
                }
            } else {
                output << "Unknown command. Type 'help'.\n";
            }
        } catch (const domain::ValidationError& ex) {
            output << "Validation error: " << ex.what() << '\n';
        }
    }

    return 0;
}

}  // namespace banking::cli
