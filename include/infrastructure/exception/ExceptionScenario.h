#pragma once

#include "infrastructure/exception/ExceptionInjector.h"

#include <string>
#include <vector>

namespace fincon {

struct ExceptionScenario {
    ExceptionType type;
    EntityType entity;
    std::string reason;
};

const std::vector<ExceptionScenario>& exceptionScenarios();

} 