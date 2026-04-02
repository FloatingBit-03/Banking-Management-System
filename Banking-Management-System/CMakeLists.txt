# Banking Management System (C++20)

A modular banking management system structured using industry-standard layers:

- **Domain layer**: entities and error contracts.
- **Application layer**: business workflows and validation policies.
- **Infrastructure layer**: repository implementation (in-memory for now).
- **Interface layer (CLI)**: operator command processing.

## Project Layout

```text
include/banking/
  application/
    banking_service.hpp
  cli/
    command_processor.hpp
  domain/
    entities.hpp
    errors.hpp
  infrastructure/
    account_repository.hpp
    in_memory_repository.hpp
src/
  application/banking_service.cpp
  cli/command_processor.cpp
  domain/entities.cpp
  infrastructure/in_memory_repository.cpp
  main.cpp
tests/
  banking_tests.cpp
```

## Engineering Conventions Applied

- Clear separation of concerns by layer.
- Interface-driven design through `AccountRepository` abstraction.
- Dependency injection (`BankingService` receives repository implementation).
- Money represented as integer cents to avoid floating-point precision issues.
- Deterministic, isolated unit tests.

## Build

```bash
cmake -S . -B build
cmake --build build
```

## Run

```bash
./build/bank_cli
```

## Test

```bash
ctest --test-dir build --output-on-failure
```
