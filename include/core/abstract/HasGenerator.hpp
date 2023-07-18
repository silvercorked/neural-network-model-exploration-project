#pragma once

#include <random>

struct HasGenerator {
    virtual void setGenerator(std::mt19937*) = 0;
};