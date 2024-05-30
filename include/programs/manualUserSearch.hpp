#pragma once

#include <limits.h>
#include <string>
#include <vector>
#include <unordered_map>

#include <fplus/fplus.hpp>
#include <fdeep/fdeep.hpp>

#include "JsonUtils.hpp"
#include "TerminalUtils.hpp"

namespace ManualUserSearch {

    constexpr const auto FEATURE_DOMAIN_CONSTRAINT_PATH = "../in/features_iris.json";
    constexpr const auto MODEL_PATH = "../in/saved_model_iris.json";

    const auto model = fdeep::load_model(MODEL_PATH); // load model once

    auto program() -> int;
    auto getFeaturesAndDomainsFromInput(const json&) -> const std::unordered_map<std::string, Domain<double>>;
    auto printFeatureAndDomain(const std::string&, const Domain<double>&) -> void;
    auto getPrediction(const std::vector<float>&) -> std::vector<float>;

    auto program() -> int {
        auto j = JsonUtils::readJsonFile(std::string(FEATURE_DOMAIN_CONSTRAINT_PATH));
        const auto featAndDomains = getFeaturesAndDomainsFromInput(j);
        const auto len = featAndDomains.size();
        auto features = std::vector<std::string>();
        features.reserve(len);
        for (auto kv : featAndDomains)
            features.push_back(kv.first);
        ManualUserSearch_Program_Label:
        TerminalUtils::clearTerminal();
        auto inputVals = std::vector<float>();
        inputVals.reserve(len);
        for (auto i = 0; i < len; i++) {
            printFeatureAndDomain(features[i], featAndDomains.at(features[i]));
            double in = TerminalUtils::awaitDouble();
            if (in == TerminalUtils::ErrorAwaitingDouble) {
                i--;
                continue;
            }
            inputVals.push_back(in);
        }
        assert(inputVals.size() == featAndDomains.size());
        const auto result = getPrediction(inputVals);
        std::cout << "Result:" << std::endl << "\t";
        for (auto i = 0; i < result.size(); i++)
            std::cout << result.at(i) << ", ";
        std::cout << std::endl << "Press enter to go again. Type 'exit' to quit." << std::endl;
        std::string out = TerminalUtils::awaitInput();
        if (out != "exit")
            goto ManualUserSearch_Program_Label;
        return 1;
    }
    auto getFeaturesAndDomainsFromInput(const json& j) -> const std::unordered_map<std::string, Domain<double>> {
        auto& features = j["featuresAndDomains"];
        auto ret = std::unordered_map<std::string, Domain<double>>();
        for (auto& f : features) {
            Domain<double> domain;
            domain = Domain<double>(f["min"].get<double>(), f["max"].get<double>());
            ret[f["name"].get<std::string>()] = domain;
        }
        return ret;
    }
    auto printFeatureAndDomain(const std::string& s, const Domain<double>& d) -> void {
        std::cout << "Feature: " << s << std::endl;
        std::cout << "\tMin-Max: " << d.getMin() << "-" << d.getMax() << std::endl;
    }
    auto getPrediction(const std::vector<float>& inputs) -> std::vector<float> {
        const auto len = inputs.size();
        auto alignedInput = fdeep::float_vec();
        alignedInput.reserve(len);
        for (auto i = 0; i < len; i++)
            alignedInput.push_back(inputs.at(i));
        const auto sharedAlignedInput = fplus::make_shared_ref<fdeep::float_vec>(alignedInput);
        const auto tensorInput = fdeep::tensor(fdeep::tensor_shape(sharedAlignedInput->size()), sharedAlignedInput);
        auto result = model.predict({tensorInput});
        std::vector<float> res = result.at(0).to_vector();
        return res;
    }
}
