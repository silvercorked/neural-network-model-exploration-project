#pragma once

#include <algorithm>
#include <iostream>
#include <vector>

#include <stdio.h>

#include "DynamicBitSet.hpp"

/*
    pack onlyOneBitRaised-constrained Features into an int (make expandable wtih multiple ints for large groups).
    randomly generate a number between 0 and 31, and raise that bit (2^ randomly generated number). Use that as the output result.

    combine two of this class to manage a nonrandom + random set in a wrapper.
    if linear contains a set value, fill rest with 0, else set value in random and fill linears with 0
*/

class OnlyOneHighBConstrainedFeatureSet {
    DynamicBitSet<uint8_t> nonRandomBools;
    DynamicBitSet<uint8_t> randomBools; // 2 of these? one for non randoms one for randoms?
    std::mt19937* gen;
    std::vector<uint32_t> nonRandomIndexes;
public:
    OnlyOneHighBConstrainedFeatureSet(
        const std::vector<uint32_t>&,
        const std::vector<std::string>&
    );
    OnlyOneHighBConstrainedFeatureSet() = delete;
    auto setGenerator(std::mt19937*) -> void;
    auto setRandoms() -> void;
    auto clearRandoms() -> void;
    auto getCurr() const -> std::vector<double>;
    auto getAtIndex(uint32_t) const -> double;
    auto next() -> bool;
    auto reset() -> void;
};

OnlyOneHighBConstrainedFeatureSet::OnlyOneHighBConstrainedFeatureSet(
    const std::vector<uint32_t>& nRandomIndexes,
    const std::vector<std::string>& allFeatures
) {
    uint32_t nonRandomsLen = nRandomIndexes.size();
    uint32_t featuresLen = allFeatures.size();
    this->nonRandomIndexes = std::vector<uint32_t>(nRandomIndexes);
    this->nonRandomBools = DynamicBitSet<uint8_t>(nonRandomsLen);
    this->randomBools = DynamicBitSet<uint8_t>(featuresLen - nonRandomsLen);
    //std::cout << "nonRandomSize: " << this->nonRandomBools.size() << " randomSize: " << this->randomBools.size()
    //    << " m1 " << nonRandomsLen << " m2 " << (featuresLen - nonRandomsLen) << std::endl;
    if (this->randomBools.size() == 0) // if there are no randoms, then the start step where all non randoms are zero is invalid
        this->next();
}
auto OnlyOneHighBConstrainedFeatureSet::setGenerator(std::mt19937* g) -> void {
    this->gen = g;
}
auto OnlyOneHighBConstrainedFeatureSet::setRandoms() -> void { // useful when non randoms are all not set
    //std::cout << "setting randoms 0" << std::endl;
    if (this->randomBools.size() == 0) return; // if empty, nothign to do
    if (this->nonRandomBools.popcount() != 0) {
        this->clearRandoms();
        return; // if nonrandoms are set, then randoms must be zero
    }
    //std::cout << "setting randoms 1" << std::endl;
    uint32_t size = this->randomBools.size();
    auto dist = std::uniform_int_distribution<uint32_t>(0, size - 1);
    uint32_t rand = dist(*this->gen);
    this->randomBools.clear();
    //std::cout << "setting randoms 2 size: " << this->randomBools.size() << std::endl;
    this->randomBools.set(rand, true);
    //std::cout << "setting randoms done" << std::endl;
}
auto OnlyOneHighBConstrainedFeatureSet::clearRandoms() -> void {
    this->randomBools.clear();
}
auto OnlyOneHighBConstrainedFeatureSet::getCurr() const -> std::vector<double> {
    auto ret = std::vector<double>();
    const auto totalLen = this->nonRandomBools.size() + this->randomBools.size();
    ret.reserve(totalLen);
    auto randomsIndexOffset = 0;
    for (auto i = 0; i < totalLen; i++) {
        uint32_t found = 0;
        for (auto j = 0; j < this->nonRandomIndexes.size(); j++)
            if (this->nonRandomIndexes[j] == i) {
                found = j + 1; // add one cause zeroth index is falsey
                break;
            }
        if (found) { // check if truely, ie not zero
            ret.push_back(this->nonRandomBools.get(found - 1));
            randomsIndexOffset++; // indirect that gets the correct index in randomBools when subtracted from i
        }
        else
            ret.push_back(this->randomBools.get(i - randomsIndexOffset));
    }
    return ret;
}
auto OnlyOneHighBConstrainedFeatureSet::getAtIndex(uint32_t index) const -> double {
    const auto totalLen = this->nonRandomBools.size() + this->randomBools.size();
    if (index >= totalLen)
        throw std::out_of_range("Index is out of range on contraint. index: "
            + std::to_string(index) + " totalLen: " + std::to_string(totalLen)
        );
    uint32_t s = 0;
    for (auto i = 0; i < this->nonRandomIndexes.size(); i++) {
        if (this->nonRandomIndexes[i] == index) return this->nonRandomBools[i];
        if (this->nonRandomIndexes[i] < index) s++; // number of nonRandoms which are offsetting index
    }
    return this->randomBools[index - s]; // index - s = index if non rnadoms were gone
}
auto OnlyOneHighBConstrainedFeatureSet::next() -> bool {
    const auto len = this->nonRandomBools.size();
    if (len == 0) return false;
    if (this->nonRandomBools.popcount() == 0 && len > 0) { // case all zero
        this->nonRandomBools.set(0, true);
        return true;
    }
    uint32_t i = 0;
    while (i < len) { // case one set
        if (this->nonRandomBools[i])
            break;
        i++;
    }
    // i is either == len (ie none found (shouldn't be possible)) or an index of the set value
    if (i == len) throw std::out_of_range("next on constrained set i == len: " + std::to_string(len));
    if (i == len - 1) return false; // can't next cause we're at the end
    this->nonRandomBools.set(i, false);
    this->nonRandomBools.set(i + 1, true);
    return true;
}
auto OnlyOneHighBConstrainedFeatureSet::reset() -> void {
    this->nonRandomBools.clear();
}
