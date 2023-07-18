#pragma once

#include <string>
#include <vector>
#include <map>
#include <mutex>
#include <limits>

#include "stats.hpp"

namespace Stats {
    namespace Internal {
        struct StatPack {
            long double mean;
            long double sampleVariance;
            uint64_t n;
            StatPack() : mean(0), sampleVariance(0), n(0) {};
            StatPack(long double m, long double sv, uint64_t n)
                : mean(m), sampleVariance(sv), n(n) {};
            StatPack(StatPack& sp)
                : mean(sp.mean), sampleVariance(sp.sampleVariance), n(sp.n) {};
            StatPack(const StatPack& sp)
                : mean(sp.mean), sampleVariance(sp.sampleVariance), n(sp.n) {};
        };
    };
    class StatsTracker {
        std::mutex lock;
        std::map<std::string, Internal::StatPack> stats;
        TallyCounter<uint64_t> tallies;
    public:
        StatsTracker();
        StatsTracker(const std::vector<std::string>&);
        StatsTracker(StatsTracker&&);

        auto add(const std::string&) -> bool;
        template <Concepts::Numeric N>
        auto addNewValue(const std::string&, N) -> bool;
        auto addTally(const std::string& key) -> bool;

        auto getTallyCount(const std::string&) const -> uint64_t;
        auto getTallyPercentage(const std::string&) const -> double;
        auto getMean(const std::string&) const -> long double;
        auto getSampleVariance(const std::string&) const -> long double;
        auto getN(const std::string&) const -> uint64_t;
        
        StatsTracker(StatsTracker&) = delete;
        StatsTracker(const StatsTracker&) = delete;
        auto operator=(StatsTracker&&) -> StatsTracker& = delete;
    };
    StatsTracker::StatsTracker() {
        this->stats = std::map<std::string, Internal::StatPack>();
        this->tallies = TallyCounter<uint64_t>();
    }
    StatsTracker::StatsTracker(StatsTracker&& rv) : lock() {
        std::lock_guard m(rv.lock);
        this->stats = rv.stats;
        this->tallies = rv.tallies;
    }
    StatsTracker::StatsTracker(const std::vector<std::string>& initKeys) {
        this->stats = std::map<std::string, Internal::StatPack>();
        this->tallies = TallyCounter<uint64_t>();
        for (const auto& k : initKeys) {
            this->add(k);
        }
    }
    auto StatsTracker::add(const std::string& key) -> bool {
        std::lock_guard m(this->lock);
        if (this->stats.find(key) != this->stats.end()) // already exists
            return false;
        this->stats[key] = Internal::StatPack();
        this->tallies.add(key);
        return true;
    }
    auto StatsTracker::getTallyCount(const std::string& key) const -> uint64_t {
        if (this->tallies.keyExists(key))
            return this->tallies.getCount(key);
        return -1;
    }
    auto StatsTracker::getTallyPercentage(const std::string& key) const -> double {
        if (this->tallies.keyExists(key))
            return this->tallies.getPercentage(key);
        return -1;
    }
    auto StatsTracker::getMean(const std::string& key) const -> long double {
        if (this->stats.find(key) != this->stats.end())
            return this->stats.at(key).mean;
        return std::numeric_limits<double>::lowest();
    }
    auto StatsTracker::getSampleVariance(const std::string& key) const -> long double {
        if (this->stats.find(key) != this->stats.end())
            return this->stats.at(key).sampleVariance;
        return std::numeric_limits<double>::lowest();
    }
    auto StatsTracker::getN(const std::string& key) const -> uint64_t {
        if (this->stats.find(key) != this->stats.end())
            return this->stats.at(key).n;
        return -1;
    }
    template <Concepts::Numeric N>
    auto StatsTracker::addNewValue(const std::string& key, N val) -> bool {
        std::lock_guard m(this->lock);
        if (this->stats.find(key) == this->stats.end())
            return false;
        Internal::StatPack& sp = this->stats.at(key);
        sp.n++;
        sp.sampleVariance = sampleVarianceStep(sp.n, val, sp.mean, sp.sampleVariance);
        sp.mean = arithmeticMeanStep(sp.n, val, sp.mean);
        return true;
    }
    auto StatsTracker::addTally(const std::string& key) -> bool {
        if (this->tallies.keyExists(key))
            this->tallies.tally(key);
        else
            return false;
        return true;
    }
};