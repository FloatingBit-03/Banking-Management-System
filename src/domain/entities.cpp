#include "banking/domain/entities.hpp"

#include <iomanip>
#include <sstream>

namespace banking::domain {

std::string to_string(EntryType type) {
    switch (type) {
        case EntryType::Deposit:
            return "deposit";
        case EntryType::Withdrawal:
            return "withdrawal";
        case EntryType::TransferDebit:
            return "transfer-debit";
        case EntryType::TransferCredit:
            return "transfer-credit";
    }
    return "unknown";
}

std::string format_money(std::int64_t cents) {
    std::ostringstream out;
    const auto absolute = cents < 0 ? -cents : cents;
    if (cents < 0) {
        out << '-';
    }
    out << (absolute / 100) << '.' << std::setw(2) << std::setfill('0') << (absolute % 100);
    return out.str();
}

}  // namespace banking::domain
