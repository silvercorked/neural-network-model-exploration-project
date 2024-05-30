#pragma once

#include <string>
#include <fstream>

#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace JsonUtils {
    const json JsonObject = json::parse("{}");
    const json JsonArray = json::parse("[]");

    auto readJsonFile(const std::string& path) -> json {
        if (path.substr(path.find_last_of(".") + 1) != "json")
            throw std::invalid_argument("Invalid file path. File must have .json extension!");
        std::ifstream file(path);
        if (!file.good()) // check for file existance
            throw std::ifstream::failure("File at path (" + path + ") was not found!");
        return json::parse(file); // will throw exceptions if file isn't actually a json file
    }
    auto writeJsonFile(const std::string& path, const json& data) -> bool {
        if (path.substr(path.find_last_of(".") + 1) != "json")
            throw std::invalid_argument("Invalid file path. File must have .json extension!");
        std::ofstream o(path);
        o << data << std::endl;
        o.close();
        return true;
    }
}

