#include <iostream>
#include <memory>

#include "banking/application/banking_service.hpp"
#include "banking/cli/command_processor.hpp"
#include "banking/infrastructure/in_memory_repository.hpp"

int main() {
    auto repository = std::make_shared<banking::infrastructure::InMemoryRepository>();
    banking::application::BankingService service(repository);
    banking::cli::CommandProcessor processor(std::move(service));
    return processor.run(std::cin, std::cout);
}
