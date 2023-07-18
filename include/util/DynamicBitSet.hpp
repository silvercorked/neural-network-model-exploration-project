#pragma once

#include <vector>
#include <string>
#include <stdexcept>
#include <random>

#include "ByteSizedIntegral.hpp"

template <Concepts::ByteSizedIntegral I>
class DynamicBitSet {
    std::vector<I> bitCollections;
    constexpr static uint8_t storageContainerBitLen = sizeof(I) * 8;
    constexpr static I oneOfTypeI = 1;
    constexpr static I zeroOfTypeI = 0;
    constexpr static I negOneOfTypeI = -1;
    uint32_t maxIndex;
    uint8_t maxInnerIndex; // if max storage type == 80, so no underflow problem
    bool isEmpty;
public:
    DynamicBitSet();
    DynamicBitSet(uint32_t);
    auto setSize(uint32_t) -> bool;
    auto get(uint32_t) const -> bool;
    auto set(uint32_t, bool) -> bool;
    auto popcount() -> uint32_t;
    auto operator[](uint32_t) -> bool;
    auto operator[](uint32_t) const -> const bool;
    auto size() const -> uint32_t;
    auto clear() -> void;
    auto setAll() -> void;
};
template <Concepts::ByteSizedIntegral I>
DynamicBitSet<I>::DynamicBitSet() {
    this->bitCollections = std::vector<I>();
    this->setSize(1);
}
template <Concepts::ByteSizedIntegral I>
DynamicBitSet<I>::DynamicBitSet(uint32_t startSize) {
    this->bitCollections = std::vector<I>();
    this->setSize(startSize);
}
template <Concepts::ByteSizedIntegral I>
auto DynamicBitSet<I>::setSize(uint32_t nSize) -> bool {
    if (nSize == 0)
        this->isEmpty = true;
    else
        this->isEmpty = false;
    uint32_t mod = nSize % storageContainerBitLen;
    bool isExact = mod == 0;
    uint32_t size = isExact // if exact, will be perfect amount
        ? nSize / storageContainerBitLen // so just divide
        : (nSize / storageContainerBitLen) + 1; // but not exact, overestimate to hold all since int division rounds down
    if (this->bitCollections.size() > size) // remove
        this->bitCollections.erase(this->bitCollections.begin() + size, this->bitCollections.end());
    else if (this->bitCollections.size() < size) {
        uint32_t diffSize = size - this->bitCollections.size();
        this->bitCollections.reserve(diffSize);
        for (uint32_t i = 0; i < diffSize; i++)
            this->bitCollections.push_back(0);
    }
    this->maxIndex = size - 1;
    this->maxInnerIndex = isExact
        ? storageContainerBitLen - 1
        : mod - 1;
    return true;
}
template <Concepts::ByteSizedIntegral I>
auto DynamicBitSet<I>::get(uint32_t index) const -> bool {
    if (this->isEmpty) throw std::out_of_range("Index to out of range of this dynamic bitset");
    uint32_t arrIndex = index / storageContainerBitLen;
    uint32_t innerIndex = index % storageContainerBitLen;
    if (arrIndex > this->maxIndex || (arrIndex == this->maxIndex && innerIndex > this->maxInnerIndex))
        throw std::out_of_range("Index to out of range of this dynamic bitset");
    return this->bitCollections[arrIndex] & (oneOfTypeI << innerIndex);
}
template <Concepts::ByteSizedIntegral I>
auto DynamicBitSet<I>::set(uint32_t index, bool value) -> bool {
    if (this->isEmpty) throw std::out_of_range("Index to out of range of this dynamic bitset");
    uint32_t arrIndex = index / storageContainerBitLen;
    uint32_t innerIndex = index % storageContainerBitLen;
    //std::cout << "index: " << index << " arrIndex: " << arrIndex << " innerIndex: " << innerIndex
    //    << " maxIndex: " << this->maxIndex << " maxInnerIndex: " << this->maxInnerIndex
    //    << " bitcollections size: " << this->bitCollections.size() << std::endl;
    if (arrIndex > this->maxIndex || (arrIndex == this->maxIndex && innerIndex > this->maxInnerIndex))
        throw std::out_of_range("Index to out of range of this dynamic bitset");
    this->bitCollections[arrIndex] = (this->bitCollections[arrIndex]
        & ~(oneOfTypeI << innerIndex))
        | ((value ? oneOfTypeI : zeroOfTypeI) << innerIndex);
    return true;
}
template <Concepts::ByteSizedIntegral I>
auto DynamicBitSet<I>::popcount() -> uint32_t {
    uint32_t sum = 0;
    for (const auto& set : bitCollections) {
        #ifdef __GNUC__
        sum += __builtin_popcount(set);
        #else // https://codereview.stackexchange.com/questions/38182/counting-number-of-1s-and-0s-from-integer-with-bitwise-operation
        uint32_t count;
        for (count = 0; set != 0; count++, set &= set - 1); // x &= x - 1 removes the rightmost set bit. do this until exhausted
        sum += count; // 1100 - 1 = 1011, 1100 & 1011 = 1000
        #endif
    }
    return sum;
}
template <Concepts::ByteSizedIntegral I>
auto DynamicBitSet<I>::operator[](uint32_t index) -> bool {
    return this->get(index);
}
template <Concepts::ByteSizedIntegral I>
auto DynamicBitSet<I>::operator[](uint32_t index) const -> const bool {
    return this->get(index);
}
template <Concepts::ByteSizedIntegral I>
auto DynamicBitSet<I>::size() const -> uint32_t {
    if (this->isEmpty) return 0;
    return (this->maxIndex * storageContainerBitLen) + this->maxInnerIndex + 1;
}
template <Concepts::ByteSizedIntegral I>
auto DynamicBitSet<I>::clear() -> void {
    for (uint32_t i = 0; i < this->bitCollections.size(); i++)
        this->bitCollections[i] = zeroOfTypeI;
}
template <Concepts::ByteSizedIntegral I>
auto DynamicBitSet<I>::setAll() -> void {
    for (uint32_t i = 0; i < this->bitCollections.size(); i++)
        this->bitCollections[i] = negOneOfTypeI;
}