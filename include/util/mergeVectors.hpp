#pragma once

#include "Numeric.hpp"
#include <assert.h>

template<Concepts::Numeric T>
std::vector<T>& mergeVectors(std::vector<T>& a, const std::vector<T>& b, const std::vector<uint32_t> indexesToMerge) {
    auto len = indexesToMerge.size();
    auto vecSize = a.size();
    assert(vecSize = b.size());
    assert(vecSize <= len);
    for (const auto& i : indexesToMerge)
        a.insert(i, b[i]);
    return a;
}
