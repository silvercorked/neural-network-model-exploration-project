#pragma once

#include <vector>
#include <unordered_map>
#include <variant>
#include <string>
#include <memory>
#include <type_traits>
#include <functional>

#include <stdio.h>

#include "OverloadedLambda.hpp"
#include "Domain.hpp"
#include "ContinuousFeature.hpp"
#include "DiscreteFeature.hpp"
//#include "RandomContinuousFeature.hpp"
//#include "RandomDiscreteFeature.hpp"
#include "RandomFeature.hpp"
#include "OnlyOneHighBConstrainedFeatureSet.hpp"

enum CONSTRAINT_TYPE {
    ONLYONEHIGHBINARY = 0,
    OTHER = 100
};

// helper constant for visitors
template<class>
inline constexpr bool always_false_v = false;

template <typename F>
concept FeatureTypes = std::is_same_v<ContinuousFeature, F> || std::is_same_v<DiscreteFeature, F>
    || std::is_same_v<RandomFeature<double>, F> || std::is_same_v<RandomFeature<int64_t>, F>;
template <typename F>
concept CountingFeatureTypes = std::is_same_v<ContinuousFeature, F> || std::is_same_v<DiscreteFeature, F>;
template <typename F>
concept FloatingFeatureTypes = std::is_same_v<ContinuousFeature, F> || std::is_same_v<RandomFeature<double>, F>;
template <typename F>
concept IntegralFeatureTypes = std::is_same_v<DiscreteFeature, F> || std::is_same_v<RandomFeature<int64_t>, F>;
template <typename F>
concept RandomFeatureTypes = std::is_same_v<RandomFeature<double>, F> || std::is_same_v<RandomFeature<int64_t>, F>;

using DomainVariantType = std::variant<Domain<double>, Domain<int64_t>>;
using NonRandomVariant = std::variant<ContinuousFeature, DiscreteFeature>; // use std::visit to interact
using RandomVariant = std::variant<RandomFeature<double>, RandomFeature<int64_t>>; // use polymorphism to interact

template <typename C>
concept ContainerTypes = std::is_same_v<NonRandomVariant, C> || std::is_same_v<RandomVariant, C>;

using CurrVariantType = std::variant<double, int64_t>;

using GetCurrFunctionType = std::function<bool (CurrVariantType&)>;

class TrialManager {
    std::vector<NonRandomVariant> nonRandoms;
    std::vector<OnlyOneHighBConstrainedFeatureSet> constrainedFeatures;
    std::vector<RandomVariant> randoms;
    std::vector<std::string> featureNamesInOrder;
    std::vector<uint32_t> nonRandomIndexes;
    std::unordered_map<std::string, GetCurrFunctionType> getCurrMap;
    // with the above 2 combines, indexMap becomes simpler.
        // rands generally larger, so keep rands in order amongst themselves to output order
        // keep non rnads in order amongst themsevles
        // have generate an array of size (num featuers) and merge using merge vectors util function
    // non randoms will have a maintained array containing their indexes
    // unconstrained randoms will not
    // constrained randoms or non randoms will maintain local arrays of their position
        // perhaps a new method of getting currs could avoid these local copies
            // maybe hashmap or vector of functions whihc each get the float of that index?

    template <FeatureTypes F, typename D, ContainerTypes C> requires (
        (
            std::is_same_v<RandomVariant, C>
                && ((std::is_same_v<Domain<double>, D> && std::is_same_v<RandomFeature<double>, F>)
                    || (std::is_same_v<Domain<int64_t>, D> && std::is_same_v<RandomFeature<int64_t>, F>))
        )
        || (
            std::is_same_v<NonRandomVariant, C>
                && ((std::is_same_v<Domain<double>, D> && std::is_same_v<ContinuousFeature, F>)
                    || (std::is_same_v<Domain<int64_t>, D> && std::is_same_v<DiscreteFeature, F>))
        )
    )
    static auto addFeature(
        const std::string&,
        const D&,
        std::vector<C>&
    ) -> bool;
    template <ContainerTypes C>
    static auto getNames(const std::vector<C>&, std::vector<std::string>&) -> void;
    auto findIndexOrder(const std::string&) const -> uint32_t;
public:
    TrialManager(
        const std::vector<std::string>&,
        const std::vector<std::string>&,
        const std::unordered_map<std::string, DomainVariantType>&,
        const std::unordered_map<CONSTRAINT_TYPE, std::vector<std::vector<std::string>>>&
    );
    TrialManager(const TrialManager&) = delete;
    auto setContinuousN(uint64_t) -> void;
    auto setRandomGen(std::mt19937*) -> void;
    auto iterateCountingFeatures() -> bool;
    auto iterateRandomFeatures() -> void;
    auto getFeatureNames() const -> std::vector<std::string>;
    auto getCountingFeatureNames() const -> std::vector<std::string>;
    auto getCurrent() const -> std::vector<CurrVariantType>;
    auto getCountingCurrent() const -> std::vector<CurrVariantType>;
    ~TrialManager() = default;
};

TrialManager::TrialManager(
    const std::vector<std::string>& nonRandomFeatures,
    const std::vector<std::string>& predictionOrderFeatureNames,
    const std::unordered_map<std::string, DomainVariantType>& featuresAndDomains,
    const std::unordered_map<CONSTRAINT_TYPE, std::vector<std::vector<std::string>>>& constrainedFeatures
) {
    //std::cout << "starting construction" << std::endl;
    this->nonRandoms = std::vector<NonRandomVariant>();
    this->randoms = std::vector<RandomVariant>();
    this->featureNamesInOrder = std::vector<std::string>(predictionOrderFeatureNames);
    this->nonRandomIndexes = std::vector<uint32_t>();

    for (const auto& nonRandom : nonRandomFeatures) {
        uint32_t index = -1;
        for (auto i = 0; i < this->featureNamesInOrder.size(); i++) {
            if (nonRandom == this->featureNamesInOrder[i]) {
                index = i;
                break;
            }
        }
        if (index == -1) throw std::invalid_argument("Non random feature name not found in feature list.");
        this->nonRandomIndexes.push_back(index);
    }

    GetCurrFunctionType tempLambda = [](CurrVariantType& out) -> bool {
        std::cout << "TEMP handler getCurr being called! THIS IS AN ERROR!" << std::endl;
        out = 0;
        return false; // used as default. will always return false, signallying something went wrong.
    };
    this->getCurrMap = std::unordered_map<std::string, GetCurrFunctionType>();
    
    for (const auto& featName : this->featureNamesInOrder) {
        this->getCurrMap[featName] = tempLambda;
    }
    std::vector<std::string> handled = std::vector<std::string>();
    
    if (constrainedFeatures.find(CONSTRAINT_TYPE::ONLYONEHIGHBINARY) != constrainedFeatures.end()) { // if has any constrained features
        for (auto constrainedSetIndex = 0; constrainedSetIndex < constrainedFeatures.at(CONSTRAINT_TYPE::ONLYONEHIGHBINARY).size(); constrainedSetIndex++) { // handle only high binary constrains
            const auto constrainedSet = constrainedFeatures.at(CONSTRAINT_TYPE::ONLYONEHIGHBINARY)[constrainedSetIndex];
            std::vector<uint32_t> nonRandomConstrIndexes = std::vector<uint32_t>();
            for (auto i = 0; i < constrainedSet.size(); i++) {
                bool isRandom = nonRandomFeatures.end() == std::find(nonRandomFeatures.begin(), nonRandomFeatures.end(), constrainedSet[i]);
                if (!isRandom) { // if not found in non randoms, must be random
                    nonRandomConstrIndexes.push_back(i);
                }
            }
            /*std::cout << "creating constrained, nonRandomConstrIndexesSize: " << nonRandomConstrIndexes.size()
                << " constrainedSetSize: " << constrainedSet.size() << std::endl;
            std::cout << "nonRandoms: ";
            for (const auto& n : nonRandomFeatures)
                std::cout << n << ", ";
            std::cout << std::endl;
            std::cout << "nonRandoms indexes: ";
            for (const auto& n : nonRandomConstrIndexes)
                std::cout << n << ", ";
            std::cout << std::endl;
            std::cout << "constrainedSet: ";
            for (const auto& n : constrainedSet)
                std::cout << n << ", ";
            std::cout << std::endl;*/
            auto constrained = OnlyOneHighBConstrainedFeatureSet(nonRandomConstrIndexes, constrainedSet);
            this->constrainedFeatures.push_back(constrained);
            for (auto i = 0; i < constrainedSet.size(); i++) { // add to handled and setup getCurrMap
                const auto featName = constrainedSet[i];
                handled.push_back(featName);
                //std::cout << "handling (constr): " << featName << std::endl;
                this->getCurrMap[featName] = [this, constrainedSetIndex, i](CurrVariantType& out) -> bool {
                    //std::cout << "onlyOne Handler getCurr being called!" << std::endl;
                    //auto temp = this->constrainedFeatures[constrainedSetIndex].getAtIndex(i);
                    //std::cout << "\t onlyOneHandler: val was " << temp << std::endl;
                    out = this->constrainedFeatures[constrainedSetIndex].getAtIndex(i);
                    return true;
                };
            }
        } // constrained should all be made correctly
    }
    bool wentWell = true;
    //std::cout << "nonRandomLen" << nonRandomFeatures.size() << std::endl;
    for (const auto& nonRandom : nonRandomFeatures) {
        bool beenHandled = handled.end() != std::find(handled.begin(), handled.end(), nonRandom);
        //std::cout << "nonRandom: " << nonRandom << " beenHandled? " << (beenHandled ? "true" : "false") << std::endl;
        if (beenHandled) continue;
        auto domainVariant = featuresAndDomains.at(nonRandom);
        std::visit([&](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, Domain<double>>) {
                auto domain = std::get<Domain<double>>(domainVariant);
                wentWell &= this->addFeature<ContinuousFeature>(nonRandom, domain, this->nonRandoms);
            }
            else if constexpr (std::is_same_v<T, Domain<int64_t>>) {
                auto domain = std::get<Domain<int64_t>>(domainVariant);
                wentWell &= this->addFeature<DiscreteFeature>(nonRandom, domain, this->nonRandoms);
            }
            else
                static_assert(always_false_v<T>, "Constructor (NonRandoms): non-exhaustive visitor!");
        }, domainVariant);
        if (!wentWell) {
            std::cout << "Failed adding feature to nonRandoms "
                << "featureName: " << nonRandom << std::endl;
        }
        uint32_t indexOfFeature = this->nonRandoms.size() - 1;
        this->getCurrMap[nonRandom] = [this, indexOfFeature](CurrVariantType& out) -> bool {
            //std::cout << "non random handler getCurr being called!" << std::endl;
            std::visit([&](auto&& arg) {
                //auto temp = arg.getCurr();
                //std::cout << "\t nonRandomHandler: val was " << temp << std::endl;
                out = arg.getCurr();
            }, this->nonRandoms[indexOfFeature]);
            return true;
        };
        //std::cout << "handling (norand): " << nonRandom << std::endl;
        handled.push_back(nonRandom);
    } // nonRandomsShould all be done
    for (const auto& featName : this->featureNamesInOrder) {
        bool beenHandled = handled.end() != std::find(handled.begin(), handled.end(), featName);
        if (beenHandled) continue;
        auto domainVariant = featuresAndDomains.at(featName);
        std::visit([&](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, Domain<double>>) {
                auto domain = std::get<Domain<double>>(domainVariant);
                wentWell &= this->addFeature<RandomFeature<double>>(featName, domain, this->randoms);
            }
            else if constexpr (std::is_same_v<T, Domain<int64_t>>) {
                auto domain = std::get<Domain<int64_t>>(domainVariant);
                wentWell &= this->addFeature<RandomFeature<int64_t>>(featName, domain, this->randoms);
            }
            else
                static_assert(always_false_v<T>, "Constructor (Randoms): non-exhaustive visitor!");
        }, domainVariant);
        if (!wentWell) {
            std::cout << "Failed adding feature to randoms "
                << "featureName: " << featName << std::endl;
        }
        uint32_t indexOfFeature = this->randoms.size() - 1;
        this->getCurrMap[featName] = [this, indexOfFeature](CurrVariantType& out) -> bool {
            //std::cout << "random handler getCurr being called!" << std::endl;
            std::visit([&](auto&& arg) {
                out = arg.getCurr();
                //auto temp = arg.getCurr();
                //std::cout << "\t randomHandler: val was " << temp << std::endl;
            }, this->randoms[indexOfFeature]);
            return true;
        };
        //std::cout << "handling (random): " << featName << std::endl;
        handled.push_back(featName);
    }
    assert(handled.size() == this->featureNamesInOrder.size());
    //std::cout << "total unconstrained features" << this->randoms.size() + this->nonRandoms.size() << std::endl;
    //std::cout << "construction complete" << std::endl;
    

    // after creating all constructs, can setup curr getter functions

    // std::cout << "algorithm order:\n"
    //     << '\t';
    // for (const auto& f : this->getFeatureNames()) {
    //     std::cout << f << ", ";
    // }
    // std::cout << "\nprediction order:\n";
    // for (const auto& f : predictionOrderFeatureNames) {
    //     std::cout << f << ", ";
    // }
}

template <FeatureTypes F, typename D, ContainerTypes C> requires (
    (
        std::is_same_v<RandomVariant, C>
            && ((std::is_same_v<Domain<double>, D> && std::is_same_v<RandomFeature<double>, F>)
                || (std::is_same_v<Domain<int64_t>, D> && std::is_same_v<RandomFeature<int64_t>, F>))
    )
    || (
        std::is_same_v<NonRandomVariant, C>
            && ((std::is_same_v<Domain<double>, D> && std::is_same_v<ContinuousFeature, F>)
                || (std::is_same_v<Domain<int64_t>, D> && std::is_same_v<DiscreteFeature, F>))
    )
)
auto TrialManager::addFeature(
    const std::string& featName,
    const D& domain,
    std::vector<C>& curr
) -> bool {
    curr.push_back(F(featName, domain.getMin(), domain.getMax()));
    return true;
}
auto TrialManager::setContinuousN(uint64_t n) -> void {
    double exp = pow(2, n);
    for (auto& f : this->nonRandoms) {
        std::visit([&](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, ContinuousFeature>) {
                arg.setDenominator((uint64_t) exp);
            }
            else if constexpr (std::is_same_v<T, DiscreteFeature>) {} // continue
            else
                static_assert(always_false_v<T>, "setContinuousN: non-exhaustive visitor!");
        }, f);
    }
}
auto TrialManager::setRandomGen(std::mt19937* gen) -> void {
    for (auto& f : this->randoms) {
        std::visit([&](auto&& arg) {
            //using T = std::decay_t<decltype(arg)>;
            //if constexpr (std::is_same_v<T, RandomFeature<double>> || std::is_same_v<T, RandomFeature<int64_t>>) {
            arg.setGenerator(gen);
            //}
            //else
                //static_assert(always_false_v<T>, "setRandomGen: non-exhaustive visitor!");
        }, f);
    }
    for (auto& c : this->constrainedFeatures) {
        c.setGenerator(gen);
    }
}
auto TrialManager::iterateCountingFeatures() -> bool {
    bool notAbleToNext = true;
    const auto len = this->nonRandoms.size();
    for (auto i = 0; notAbleToNext && i < len; i++) {
        std::visit([&](auto&& arg) {
            if (arg.next()) { // if can next, then no need to next any more as already have unique output
                notAbleToNext = false;
                return;
            } // if cant next, then reset, as eventually we'll get a new one
            arg.reset(); // start again
        }, this->nonRandoms[i]);
    } // if still none can next, then check if constrains have nonRandoms and can next
    if (notAbleToNext) {
        for (auto& constrained : this->constrainedFeatures) {
            if (constrained.next()) {
                notAbleToNext = false;
                break;
            }
            constrained.reset();
        }
    }
    return notAbleToNext;
}
auto TrialManager::iterateRandomFeatures() -> void {
    for (auto& r : this->randoms)
        std::visit([&](auto&& arg) {
            arg.next();
        }, r);
    //std::cout << "done with randoms, now doing constraineds" << std::endl;
    for (auto& cr : this->constrainedFeatures) {
        cr.setRandoms();
    }
    //std::cout << "done with iterating randoms" << std::endl;
}
auto TrialManager::getFeatureNames() const -> std::vector<std::string>{
    return std::vector<std::string>(this->featureNamesInOrder);
}
auto TrialManager::getCountingFeatureNames() const -> std::vector<std::string> {
    auto names = std::vector<std::string>();
    for (auto i = 0; i < this->nonRandomIndexes.size(); i++)
        names.push_back(this->featureNamesInOrder[this->nonRandomIndexes[i]]);
    return names;
}
template <ContainerTypes C>
auto TrialManager::getNames(const std::vector<C>& features, std::vector<std::string>& ret) -> void {
    ret.reserve(features.size());
    for (const auto& feat : features)
        ret.push_back(
            std::visit([](auto&& arg){
                return arg.getName();
            }, feat)
        );
}

auto TrialManager::getCurrent() const -> std::vector<CurrVariantType> {
    //std::cout << "attemtping to get currents" << std::endl;
    const size_t size = this->featureNamesInOrder.size();
    auto currents = std::vector<CurrVariantType>();
    currents.reserve(size);
    for (const auto& name : this->featureNamesInOrder) {
        CurrVariantType building;
        //std::cout << "calling " << name << "'s getcurr handler" << std::endl;
        this->getCurrMap.at(name)(building);
        //std::cout << "called getcurr func and got curr" << std::endl;
        currents.push_back(building);
    }
    return currents;
}
auto TrialManager::findIndexOrder(const std::string& s) const -> uint32_t {
    const auto len = this->featureNamesInOrder.size();
    for (auto i = 0; i < len; i++)
        if (s == this->featureNamesInOrder[i])
            return i;
    return -1; // not found
}
auto TrialManager::getCountingCurrent() const -> std::vector<CurrVariantType> {
    auto currents = std::vector<CurrVariantType>();
    auto names = this->getCountingFeatureNames();
    for (const auto& name : names) {
        CurrVariantType building;
        this->getCurrMap.at(name)(building);
        currents.push_back(building);
    }
    return currents;
}
