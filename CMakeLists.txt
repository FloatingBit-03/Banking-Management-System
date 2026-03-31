cmake_minimum_required(VERSION 3.16)
project(BankingManagementSystem VERSION 2.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(banking_lib
    src/domain/entities.cpp
    src/infrastructure/in_memory_repository.cpp
    src/application/banking_service.cpp
    src/cli/command_processor.cpp
)

target_include_directories(banking_lib PUBLIC include)
target_compile_options(banking_lib PRIVATE -Wall -Wextra -Wpedantic)

add_executable(bank_cli src/main.cpp)
target_link_libraries(bank_cli PRIVATE banking_lib)

add_executable(banking_tests tests/banking_tests.cpp)
target_link_libraries(banking_tests PRIVATE banking_lib)

enable_testing()
add_test(NAME banking_tests COMMAND banking_tests)
