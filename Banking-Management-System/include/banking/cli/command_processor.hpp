#pragma once

#include <istream>
#include <ostream>

#include "banking/application/banking_service.hpp"

namespace banking::cli {

class CommandProcessor {
public:
    explicit CommandProcessor(application::BankingService service);
    int run(std::istream& input, std::ostream& output);

private:
    static void print_help(std::ostream& output);
    application::BankingService service_;
};

}  // namespace banking::cli
