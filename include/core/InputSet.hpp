#pragma once

#include <unordered_map>
#include <string>
#include <vector>

template <typename K, typename V>
class InputSet {
    std::vector<K> orderedFeatureNames;
    std::vector<V> orderedValues;
public:
    InputSet(const std::vector<K>&);
    auto add(const std::pair<K, V>&) -> bool;
    auto getAll() const -> const std::vector<V>&;
};
template <typename K, typename V>
InputSet<K, V>::InputSet(const std::vector<K>& inOrderFeatureNames) {
    this->orderedFeatureNames = std::vector<K>(inOrderFeatureNames);
    this->orderedValues = std::vector<V>();
    this->orderedValues.reserve(this->orderedFeatureNames.size());
}
template <typename K, typename V>
auto InputSet<K, V>::add(const std::pair<K, V>& nVal) -> bool {
    auto len = this->orderedFeatureNames.size();
    uint32_t i = 0;
    for (; i < len; i++) {
        if (nVal.first == this->orderedFeatureNames.at(i)) break;
    }
    if (i >= len) return false; // string not found
    this->orderedValues[i] = nVal.second;
    return true;
}
template <typename K, typename V>
auto InputSet<K, V>::getAll() const -> const std::vector<V>& {
    return this->orderedValues;
}
