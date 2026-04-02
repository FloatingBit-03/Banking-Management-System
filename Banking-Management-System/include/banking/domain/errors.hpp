#pragma once

#include <stdexcept>

namespace banking::domain {

class ValidationError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

}  // namespace banking::domain
