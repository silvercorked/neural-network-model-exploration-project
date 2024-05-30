#pragma once

#include <variant>

#include "Globals.hpp"
#include "JsonUtils.hpp"
#include "Domain.hpp"

namespace ModelFeatureJsonUtils {

    auto readInputFile(std::string_view) -> const json;
    auto getFeaturesFromInput(const json&) -> const std::vector<std::string>;
    auto getFeaturesAndDomainsFromInput(const json&) -> const std::unordered_map<std::string, std::variant<Domain<double>, Domain<int64_t>>>;
    auto getConstraintedFeaturesFromInput(const json&) -> const std::unordered_map<CONSTRAINT_TYPE, std::vector<std::vector<std::string>>>;

    auto readInputFile(std::string featureDomainConstraintPath) -> const json {
        json j = JsonUtils::readJsonFile(featureDomainConstraintPath);
        j.at("featuresAndDomains"); // this will likely throw exceptions if that property isn't there
        j.at("constrainedFeatureSets");
        return j;
    }
    auto getFeaturesFromInput(const json& j) -> const std::vector<std::string> {
        auto& features = j["featuresAndDomains"];
        std::vector<std::string> ret = std::vector<std::string>();
        ret.reserve(features.size());
        for(auto& feat: features) {
            ret.push_back(feat["name"].get<std::string>());
        }
        return ret;
    }
    auto getFeaturesAndDomainsFromInput(const json& j) -> const std::unordered_map<std::string, std::variant<Domain<double>, Domain<int64_t>>> {
        auto& features = j["featuresAndDomains"];
        auto ret = std::unordered_map<std::string, std::variant<Domain<double>, Domain<int64_t>>>();
        for (auto& f : features) {
            std::variant<Domain<double>, Domain<int64_t>> domainVariant;
            //std::cout << "reading in feature domain " << f["domain_type"].get<std::string>() << std::endl;
            if (f["domain_type"].get<std::string>() == "continuous") {
                domainVariant = Domain<double>(f["min"].get<double>(), f["max"].get<double>());
            }
            else if (f["domain_type"].get<std::string>() == "discrete") {
                domainVariant = Domain<int64_t>(f["min"].get<int64_t>(), f["max"].get<int64_t>());
            }
            else {
                std::cout << std::endl << std::endl << "DID DEFAULT!!!!!!! THIS IS PROBABLY A BUG!" << std::endl << std::endl;
                domainVariant = Domain<double>(); // default
            }
            ret[f["name"].get<std::string>()] = domainVariant;
        }
        return ret;
    }
    auto getConstraintedFeaturesFromInput(const json& j) -> const std::unordered_map<CONSTRAINT_TYPE, std::vector<std::vector<std::string>>> {
        auto& constrainedSets = j["constrainedFeatureSets"];
        auto ret = std::unordered_map<CONSTRAINT_TYPE, std::vector<std::vector<std::string>>>();
        for (auto& s : constrainedSets) {
            auto names = std::vector<std::string>();
            for (const auto& n : s["feature_names"])
                names.push_back(n.get<std::string>());
            CONSTRAINT_TYPE type;
            if (s["constraint_type"].get<std::string>() == "OnlyOneHigh")
                type = CONSTRAINT_TYPE::ONLYONEHIGHBINARY;
            else
                assert((false, "Invalid constraint type"));
            if (ret.find(type) != ret.end()) // if exists in map already
                ret[type].push_back(names); // adds types and names
            else
                ret[type] = std::vector<std::vector<std::string>>(1, names);
        }
        return ret;
    }

};
