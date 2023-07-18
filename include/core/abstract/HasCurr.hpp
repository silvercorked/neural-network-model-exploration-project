#pragma once

#include <Numeric.hpp>

template <Concepts::Numeric N>
struct HasCurr {
    virtual N getCurr() const = 0;
    virtual ~HasCurr() = default;
};
