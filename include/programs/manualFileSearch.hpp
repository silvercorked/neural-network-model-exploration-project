#pragma once

#include <iostream>

#include "JsonUtils.hpp"
#include "TerminalUtils.hpp"

namespace ManualFileSearch {

    constexpr const auto INPUT_FILE_PATH = "../in/input_file_wine.json";
    constexpr const auto OUTPUT_FILE_PATH = "../out/output_file_wine.json";
    constexpr const auto MODEL_PATH = "../in/wineModel.json";

    const auto model = fdeep::load_model(MODEL_PATH); // load model once

    auto program() -> int;
    auto readInputFile() -> json;
    auto writeOutputFile(const std::vector<float>&, const std::vector<float>&) -> bool;
    auto getPrediction(const std::vector<float>&) -> std::vector<float>;


    auto program() -> int {
        std::cout << "Printing input files:" << std::endl;
        auto j = readInputFile();
        JsonUtils::writeJsonFile(OUTPUT_FILE_PATH, JsonUtils::JsonArray); // create and clear output file
        for (auto inputVals : j) {
            std::cout << "[";
            for (auto input : inputVals)
                std::cout << input << ", ";
            std::cout << "]" << std::endl;
            std::vector<float> inputs;
            inputVals.get_to(inputs);
            std::vector<float> result = getPrediction(inputs);
            std::cout << "\t[";
            for (auto output : result)
                std::cout << output << ", ";
            std::cout << "]" << std::endl;
            writeOutputFile(inputs, result);
        }
        return 1;
    }

    auto readInputFile() -> json {
        return JsonUtils::readJsonFile(INPUT_FILE_PATH);
    }
    auto writeOutputFile(const std::vector<float>& in, const std::vector<float>& out) -> bool {
        auto j = JsonUtils::readJsonFile(OUTPUT_FILE_PATH);
        auto obj = JsonUtils::JsonObject;
        obj["input"] = in;
        obj["output"] = out;
        j.insert(j.end(), obj);
        return JsonUtils::writeJsonFile(OUTPUT_FILE_PATH, j);
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
