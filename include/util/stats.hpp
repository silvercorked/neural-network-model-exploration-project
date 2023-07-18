#pragma once

#include <cmath>
#include <limits>
#include <vector>
#include <unordered_map>

#include "Numeric.hpp"

namespace Stats {
    template <Concepts::Numeric V>
    double arithmeticMean(const std::vector<V>& nums) {
        const size_t len = nums.size();
        double sum = 0.0;
        for (const V& curr : nums) {
            sum += curr;
        }
        return sum / len;
    }
    template <Concepts::Numeric V>
    double arithmeticMeanStep(size_t n, V an, double mnsub1 = 0.0) {
        return mnsub1 + ((an - mnsub1) / n);
    }
    template <Concepts::Numeric V>
    double median(const std::vector<V>& nums) { // presumes sorted
        const size_t len = nums.size();
        const size_t halfLen = len / 2;
        if (len % 2 == 0) {
            return len != 0 ?
                (double) (nums.at(halfLen) + nums.at(halfLen - 1)) / 2.0
                : 0;
        }
        return (double) nums.at(halfLen);
    }
    template <Concepts::Numeric V>
    double midrange(const std::vector<V>& nums) { // presumes sorted
        const size_t len = nums.size();
        return len != 0
            ? (double) (nums.at(0) + nums.at(len - 1)) / 2.0
            : 0;
    }
    template <Concepts::Numeric V>
    std::unordered_map<V, size_t> frequency(const std::vector<V>& nums) {
        const size_t len = nums.size();
        std::unordered_map<V, size_t> freq = std::unordered_map<V, size_t>();
        for(size_t i = 0; i < len; i++) {
            if (freq.find(nums[i]) != freq.end())
                freq[nums[i]]++;
            else
                freq[nums[i]] = 1;
        }
        return freq;
    }
    template <Concepts::Numeric V>
    V mode(const std::vector<V>& nums) { // https://stackoverflow.com/questions/51667662/default-value-of-static-stdunordered-map
        const size_t len = nums.size();
        std::unordered_map<V, size_t> freq = std::unordered_map<V, size_t>();
        V biggestKey = 0; // replaced by first elem if len > 0, else 0
        for(size_t i = 0; i < len; i++) { // can skip checking for key existance and just set
            freq[nums[i]]++; // on search of missing key, key is added and pair's value is default constructed (size_t constructs to 0)
            if (freq[nums[i]] > freq[biggestKey]) // default value of missing key initialized to V constructor default which should be 0
                biggestKey = nums[i]; // this strategy removed 3 branches, but also initializes the key V key 0 with it's size_t pair to 0 as a consequence of the first run biggestkey comparison
        }
        return biggestKey;
    }
    template <Concepts::Numeric V>
    double variance(
        const std::vector<V>& nums,
        double mean = std::numeric_limits<double>::lowest(),
        bool sample = true
    ) {
        if (mean == std::numeric_limits<double>::lowest())
            mean = arithmeticMean(nums);
        const size_t len = nums.size();
        double sum = 0;
        for (size_t i = 0; i < len; i++) {
            sum += pow(nums[i] - mean, 2);
        }
        return sum / (len - sample); // if sample == true, len - 1, else len. sample = true = 1, false = 0
    }
    template <Concepts::Numeric V>
    double sampleVariance(
        const std::vector<V>& nums,
        double mean = std::numeric_limits<double>::lowest()
    ) {
        return variance(nums, mean, true);
    }
    template <Concepts::Numeric V>
    double populationVariance(
        const std::vector<V>& nums,
        double mean = std::numeric_limits<double>::lowest()
    ) {
        return variance(nums, mean, false);
    }
    template <Concepts::Numeric V>
    double sampleStandardDeviation(
        const std::vector<V>& nums,
        double mean = std::numeric_limits<double>::lowest()
    ) {
        return sqrt(sampleVariance(nums, mean));
    }
    template <Concepts::Numeric V>
    double populationStandardDeviation(
        const std::vector<V>& nums,
        double mean = std::numeric_limits<double>::lowest()
    ) {
        return sqrt(populationVariance(nums, mean));
    }

    template <Concepts::Numeric V>
    long double sampleVarianceStep(
        size_t n,
        V nElem,
        long double previousMean,
        long double previousVariance = 0
    ) { // https://en.wikipedia.org/wiki/Algorithms_for_calculating_variance#Welford's_online_algorithm
        if (n == 0 || n == 1) return 0; // variance isnt defined if n = 1 or 0
        return previousVariance + (pow(nElem - previousMean, 2) / n) - (previousVariance / (n - 1)); 
    }

    template <Concepts::Numeric StorageType>
    class TallyCounter {
        std::unordered_map<std::string, StorageType> counts;
        uint64_t totalCounts;
    public:
        TallyCounter();
        TallyCounter(TallyCounter&) = default;
        TallyCounter(const TallyCounter&) = default;
        auto add(const std::string&) -> bool;
        auto tally(const std::string&) -> const StorageType&;
        auto getCountsMap() const -> const std::unordered_map<std::string, StorageType>&;
        auto getPercentMap() const -> const std::unordered_map<std::string, double>;
        auto getTotalCounts() const -> uint64_t;
        auto getTallyKeys() const -> const std::vector<std::string>;
        auto getCount(const std::string&) const -> const StorageType&;
        auto getPercentage(const std::string&) const -> double;
        auto keyExists(const std::string&) const -> bool;
    };
    template <Concepts::Numeric StorageType>
    TallyCounter<StorageType>::TallyCounter() {
        this->counts = std::unordered_map<std::string, StorageType>();
        this->totalCounts = 0;
    }
    template <Concepts::Numeric StorageType>
    auto TallyCounter<StorageType>::add(const std::string& key) -> bool {
        if (this->counts.find(key) != this->counts.end()) // was found
            return false;
        this->counts[key] = 0;
        return true;
    }
    template <Concepts::Numeric StorageType>
    auto TallyCounter<StorageType>::tally(const std::string& key) -> const StorageType& {
        if (this->counts.find(key) != this->counts.end())
            this->counts[key]++;
        else
            this->counts[key] = 1;
        this->totalCounts++;
        return this->counts[key];
    }
    template <Concepts::Numeric StorageType>
    auto TallyCounter<StorageType>::getCountsMap() const -> const std::unordered_map<std::string, StorageType>& {
        return this->counts;
    }
    template <Concepts::Numeric StorageType>
    auto TallyCounter<StorageType>::getPercentMap() const -> const std::unordered_map<std::string, double> {
        auto result = std::unordered_map<std::string, double>();
        for (const auto& countPair : this->counts) {
            result[countPair.first] = countPair.second / this->totalCounts;
        }
        return result; // if no counts, still works
    }
    template <Concepts::Numeric StorageType>
    auto TallyCounter<StorageType>::getTotalCounts() const -> uint64_t {
        return this->totalCounts;
    }
    template <Concepts::Numeric StorageType>
    auto TallyCounter<StorageType>::getTallyKeys() const -> const std::vector<std::string> {
        auto result = std::vector<std::string>();
        result.reserve(this->counts.size());
        for (const auto& countPair : this->counts)
            result.push_back(countPair.first);
        return result;
    }
    template <Concepts::Numeric StorageType>
    auto TallyCounter<StorageType>::getCount(const std::string& key) const -> const StorageType& {
        return this->counts.at(key); // bubbling up case where key doesnt exist rather than handle it
    }
    template <Concepts::Numeric StorageType>
    auto TallyCounter<StorageType>::getPercentage(const std::string& key) const -> double {
        return this->totalCounts != 0
            ? this->getCount(key) / (double) this->totalCounts
            : 0; // returning is only possible if key doesnt exist, cause if it did, getcount would be positive and thus total counts would be positive 
    }
    template <Concepts::Numeric StorageType>
    auto TallyCounter<StorageType>::keyExists(const std::string& key) const -> bool {
        return this->counts.find(key) != this->counts.end();
    }
}

