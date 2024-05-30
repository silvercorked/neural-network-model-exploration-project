#pragma once

#include <assert.h>
#include <float.h>

#include <algorithm>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <type_traits>
#include <variant>
#include <vector>
#include <unordered_map>

#include <fplus/fplus.hpp>
#include <fdeep/fdeep.hpp>
#include <nlohmann/json.hpp>

#include "Domain.hpp"
#include "TrialManager.hpp"
#include "mt19937Singleton.hpp"
#include "stats.hpp"
#include "TimeManager.hpp"
#include "ThreadPool.hpp"
#include "StatsTracker.hpp"
#include "JsonUtils.hpp"
#include "ModelFeatureJsonUtils.hpp"

namespace FullSearch {

    constexpr const uint32_t    SAMPLES_PER_POINT               = 3000;
    constexpr const uint32_t    STARTN                          = 7; // min 1
    constexpr const uint32_t    MAXN                            = 7;
    constexpr const uint32_t    MAX_NONRUNNING_TASKS            = 16;
    constexpr const uint32_t    BATCH_WRITE_SIZE                = 32;
    constexpr const uint32_t    MAX_THREADS_IN_THREADPOOL       = 8;

    constexpr const bool        PREDICTION_DEBUG                = false;

    constexpr const bool        ROUND_PREDICTION_RESULTS        = false;
    constexpr const bool        TEMP_DECODING_STAGE             = false;
    constexpr const bool        TEMP_DECODING_STAGE_2           = false;

    constexpr const bool        IRIS_MODEL                      = false;
    constexpr const bool        NBI_MODEL                       = true;
    constexpr const bool        WINE_MODEL                      = false;

    constexpr const auto        MODEL_PATH                      = "../in/saved_model.json";
    constexpr const auto        FEATURE_DOMAIN_CONSTRAINT_PATH  = "../in/features.json";

    constexpr const bool        VALID                           = STARTN >= 1 && STARTN <= MAXN;

    const auto model = fdeep::load_model(MODEL_PATH); // load model once
    std::vector<std::string> STATS_KEYS;

    auto program() -> int;
    auto allDiscrete(const std::vector<std::string>&, const std::unordered_map<std::string, std::variant<Domain<double>, Domain<int64_t>>>&) -> bool;
    auto thread_start(
        const std::vector<std::string>&,
        const std::vector<std::string>&,
        const std::unordered_map<std::string, std::variant<Domain<double>, Domain<int64_t>>>&,
        const std::unordered_map<CONSTRAINT_TYPE, std::vector<std::vector<std::string>>>&,
        uint32_t
    ) -> void;
    auto iterate(TrialManager&, std::pair<std::vector<double>, Stats::StatsTracker>&) -> bool;
    auto appendToJsonFile(const std::string&, const std::vector<std::pair<std::vector<double>, Stats::StatsTracker>>&) -> bool;
    auto getPrediction(std::vector<std::variant<double, int64_t>>&, Stats::StatsTracker&) -> double;

    auto program() -> int {
        static_assert(VALID, "Invalid configuration. STARTN must be greater than 0 but less than MAXN");
        std::cout << "Hello World!" << std::endl;
        if constexpr (IRIS_MODEL)
            STATS_KEYS = std::vector<std::string> {"setosa", "versicolor", "virginica"};
        else if constexpr (NBI_MODEL)
            STATS_KEYS = std::vector<std::string> {"repair", "not_repair"};
        else if constexpr (WINE_MODEL)
            STATS_KEYS = std::vector<std::string> {"quality"};
        auto tm = TimeManagers::TimeManager();
        tm.printCurrentTimeAndDate();

        const auto input = ModelFeatureJsonUtils::readInputFile(std::string(FEATURE_DOMAIN_CONSTRAINT_PATH));
        auto features = ModelFeatureJsonUtils::getFeaturesFromInput(input);
        auto featuresAndDomains = ModelFeatureJsonUtils::getFeaturesAndDomainsFromInput(input);
        auto constrainedFeatures = ModelFeatureJsonUtils::getConstraintedFeaturesFromInput(input);

        std::cout << "feature input order in model:" << std::endl;
        for (const auto f : features) {
            std::cout << f << ", ";
        }
        std::cout << std::endl;

        // a set of discrete values only needs to be computed once, as changes to N don't affect them
        auto discreteSets = std::vector<std::vector<std::string>>();
        ThreadManagement::ThreadPool* tp = new ThreadManagement::ThreadPool(MAX_THREADS_IN_THREADPOOL);
        const size_t len = features.size();
        tm.markTime();
        //return;
        // n 1-MAXN (inclusive) across a set of linears are seperate jobs. Currently race condition on which n completes first
        // if num_features pick 2 > number of threads in ThreadPool, this race condition disappears as nth jobs
        // will be tasked and completed before n+1th jobs are tasked
        for (size_t n = STARTN; n <= MAXN; n++) {
            // per n, creates binomial coefficient of (F+1 choose 2) tasks. Linear additional jobs per n
            for (size_t i = 0; i < len; i++) { // first lin index
                for (size_t j = i + 1; j < len; j++) { // second lin index
                    const std::vector<std::string> linears { features[i], features[j] };
                    bool foundInDiscreteSets = false; // catches if all are discrete,
                    for (const auto& ss : discreteSets) { // if so, can continue. no need to recompute
                        bool matchFailed = false;
                        for (const auto& lin : linears) {
                            bool linMatchInSet = false;
                            for (const auto& s : ss) {
                                if (s == lin) {
                                    linMatchInSet = true;
                                    break;
                                }
                            }
                            if (!linMatchInSet) {
                                matchFailed = true;
                                break; // if one fails to match, break
                            }
                        }
                        if (!matchFailed) { // if match never failed, has been found and can end search early
                            foundInDiscreteSets = true;
                            break;
                        }
                    }
                    if (foundInDiscreteSets) {
                        std::cout << "Found features: ";
                        for (const auto& l : linears)
                            std::cout << l << ", ";
                        std::cout << " in DiscreteSets. Skipping to avoid recompute." << std::endl;
                        continue; // don't need to recompute
                    }
                    if (allDiscrete(linears, featuresAndDomains))
                        discreteSets.push_back(linears);
                    std::cout << linears[0] << linears[1] << std::endl;
                    // pointer required because TrailManager's Copy constructor is wrong. Pointer avoids the copy to new thread.

                    while (tp->unassignedTasks() >= MAX_NONRUNNING_TASKS) {
                        std::this_thread::yield(); // if too many tasks, yield cpu time to avoid overflowing ram with tasks data
                    }
                    std::cout << "\tqueueing task to threadpool" << "\n\tn: " << n << std::endl;

                    tp->queueTask([linears, features, featuresAndDomains, constrainedFeatures, n]() {
                        thread_start(linears, features, featuresAndDomains, constrainedFeatures, n);
                    });
                }
            }
        }
        std::cout << "All Tasks Created." << std::endl;
        tm.printTimeSinceLastMark();
        while (tp->busy()) { std::this_thread::yield(); } // wait till all tasks are allocated to threads
        delete tp; // await allocated tasks to finish
        std::cout << "all threads complete" << std::endl;
        tm.printTimeSinceLastMark();
        tm.printCurrentTimeAndDate();
        tm.printTimeSinceStart();
        return 1;
    }
    auto allDiscrete(
        const std::vector<std::string>& selected,
        const std::unordered_map<std::string, std::variant<Domain<double>, Domain<int64_t>>>& map
    ) -> bool {
        for (const auto& s : selected) {
            if (!std::holds_alternative<Domain<int64_t>>(map.at(s))) {
                return false;
            }
        }
        return true;
    }
    auto thread_start(
        const std::vector<std::string>& linears,
        const std::vector<std::string>& features,
        const std::unordered_map<std::string, std::variant<Domain<double>, Domain<int64_t>>>& featuresAndDomains,
        const std::unordered_map<CONSTRAINT_TYPE, std::vector<std::vector<std::string>>>& constrainedFeatures,
        uint32_t n
    ) -> void {
        std::cout << "starting thread job" << std::endl;
        TrialManager set = TrialManager(linears, features, featuresAndDomains, constrainedFeatures);
        set.setContinuousN(n);
        std::mt19937* gen = new std::mt19937(
            std::chrono::system_clock::now()
            .time_since_epoch()
            .count()
        );
        //std::cout << "size (i, l, r): (" << indexMap.size() << ", " << linearFeatureNames.size() << "," << randomFeatureNames.size() << ")" << std::endl;
        set.setRandomGen(gen);
        std::vector<std::string> countingNames = set.getCountingFeatureNames();
        std::string linNames = "";
        for (const auto& lin : countingNames)
            linNames += lin + "-";
        linNames = linNames.substr(0, linNames.length() - 1);

        const std::string fileName = "../out/working/" + std::to_string(n) + "_" + linNames + "_" + std::to_string(SAMPLES_PER_POINT) + ".json"; // working destination
        const std::string finalFileName = "../out/" + std::to_string(n) + "_" + linNames + "_" + std::to_string(SAMPLES_PER_POINT) + ".json"; // final destination

        std::cout << "thread: setting up file info" << std::endl;

        std::ifstream checkIfAlreadyExists(fileName); // check if file already exists
        if (!checkIfAlreadyExists.good()) { // doesn't exist
            checkIfAlreadyExists.close();
            std::ofstream initJson(fileName);
            initJson << "[]" << std::endl;
            initJson.close(); // guarentee out file exists
            std::cout << "\tshould have written file: " << fileName << std::endl;
        }
        else { // exists, no change required
            checkIfAlreadyExists.close();
        }

        std::cout << "thread: starting to collect data" << std::endl;
        auto data = std::vector<std::pair<std::vector<double>, Stats::StatsTracker>>();
        data.reserve(BATCH_WRITE_SIZE);
        size_t bufferUsage = 0;
        bool done = false;
        while (!done) {
            if (bufferUsage >= BATCH_WRITE_SIZE) { // only save on passing BATCH_WRITE_SIZE
                appendToJsonFile(fileName, data); // assume success for now
                data.clear();
                bufferUsage = 0;
            } // done set true when n increment is needed, ie, task done
            data.push_back(std::pair<std::vector<double>, Stats::StatsTracker>(
                std::move(std::vector<double>()),
                std::move(Stats::StatsTracker(STATS_KEYS))
            )); // initialize elem
            //std::cout << "thread: iterating" << std::endl;
            done = iterate(set, data[bufferUsage]);
            //std::cout << "thread: iterated" << std::endl;
            bufferUsage++;
        }
        appendToJsonFile(fileName, data); // assume success for now // last write
        data.clear();
        int code = rename(fileName.c_str(), finalFileName.c_str()); // attempt to move from working to out
        std::cout << "\t" << linNames << " writing on n change. Write Code: " << std::to_string(code) << std::endl;

        std::cout << "\tthread " << std::this_thread::get_id() << " complete. " << std::endl
            << "\t\t" << linNames << std::endl;

        delete gen;
    }
    auto iterate(TrialManager& set, std::pair<std::vector<double>, Stats::StatsTracker>& outData) -> bool {
        double runningMean = 0.0;
        //std::cout << "iterate: starting iteration" << std::endl;
        for (size_t i = 0; i < SAMPLES_PER_POINT; i++) {
            set.iterateRandomFeatures(); // generate sample w/ linears static
            //std::cout << "iterate: iterated randoms" << std::endl;
            //iterateRandoms(randoms); 
            //std::cout << "Sample " << i << std::endl;
            //std::vector<float_t> curr = getCurrs(linears, randoms, indexMap);
            auto currs = set.getCurrent();
            //std::cout << "iterate: get currents" << std::endl;
            auto prediction = getPrediction(currs, outData.second);
            //std::cout << "iterate: made prediction" << std::endl;
            //std::cout << "predicted: " << prediction << std::endl;
            runningMean = Stats::arithmeticMeanStep(i + 1, prediction, runningMean);
        }

        outData.first = std::vector<double>();
        for (const auto& variant : set.getCountingCurrent()) {
            if (std::holds_alternative<double>(variant)) {
                double val = std::get<double>(variant);
                outData.first.push_back(val);
            }
            else if (std::holds_alternative<int64_t>(variant)) {
                int64_t val = std::get<int64_t>(variant);
                outData.first.push_back(val);
            }
            else {
                std::cout << "ERROR:: MAP KEY FAILURE";
                throw std::exception();
            }
        }
        //outData.second = runningMean;
        //std::cout << "iterate: iterating coutings" << std::endl;
        return set.iterateCountingFeatures();
    }
    auto appendToJsonFile(const std::string& fileName, const std::vector<std::pair<std::vector<double>, Stats::StatsTracker>>& dataToAppend) -> bool {
        std::ifstream i(fileName);
        json oldData = json::parse(i);
        i.close();
        for (const auto& outData : dataToAppend) {
            json dataObj = JsonUtils::JsonObject;
            dataObj["coords"] = outData.first;
            dataObj["v"] = JsonUtils::JsonObject;
            for (const auto k : STATS_KEYS) {
                dataObj["v"][k] = JsonUtils::JsonObject;
                dataObj["v"][k]["m"] = outData.second.getMean(k);
                dataObj["v"][k]["sv"] = outData.second.getSampleVariance(k);
                dataObj["v"][k]["tp"] = outData.second.getTallyPercentage(k);
                dataObj["v"][k]["tc"] = outData.second.getTallyCount(k);
                dataObj["v"][k]["n"] = outData.second.getN(k);
            }
            oldData.emplace_back(dataObj);
        }
        //oldData.merge_patch(dataToAppend); // merge 2 jsons (should have not overwritting keys)
        std::ofstream o(fileName); // if there are overwritten keys, then value should be the same
        o << oldData << std::endl;
        o.close();
        return true;
    }
    auto getPrediction(std::vector<std::variant<double, int64_t>>& inputValues, Stats::StatsTracker& tracker) -> double {
        static double pastPred = 0;
        static uint64_t predCount = 0;
        static double avgPastPred = 0;
        static double avgPred = 0;
        // above is used for debugging, but not used for normal predictions
        auto alignedInput = fdeep::float_vec();
        alignedInput.reserve(inputValues.size());
        for (const auto& i : inputValues) {
            alignedInput.push_back(
                (float)(std::holds_alternative<double>(i)
                    ? std::get<double>(i)
                    : (std::holds_alternative<int64_t>(i)
                        ? std::get<int64_t>(i)
                        : -1.0
                    )
                )
            );
        }
        const auto sharedAlignedInput = fplus::make_shared_ref<fdeep::float_vec>(alignedInput);
        const auto tensorInput = fdeep::tensor(fdeep::tensor_shape(sharedAlignedInput->size()), sharedAlignedInput);
        auto result = model.predict({tensorInput});
        std::vector<float> res = result.at(0).to_vector(); // model outputs 2 proabilities [repair, not repair]. Sum is 1.0
        const float repairProbability = res.at(0); // only care about repair (positive) probability

        //std::cout << "res: ";
        //for (size_t i = 0; i < res.size(); i++) {
        //    std::cout << std::to_string(res.at(i)) << ", ";
        //}
        //std::cout << std::endl;

        if constexpr (PREDICTION_DEBUG) {
            avgPred = Stats::arithmeticMeanStep(++predCount, repairProbability, avgPred);
            avgPastPred = Stats::arithmeticMeanStep(predCount, std::abs(repairProbability - pastPred), avgPastPred);
            if (predCount % (SAMPLES_PER_POINT / 2) == 0) {
                std::cout << "Prediction: ";
                for (const auto& i : alignedInput) {
                    std::cout << std::to_string(i) << ", ";
                }
                std::cout << std::endl;
                std::cout << "result: " << std::to_string(repairProbability) << std::endl
                    << "avg: " << std::to_string(avgPred) << std::endl 
                    << "diffFromLast: " << std::to_string(repairProbability - pastPred) << std::endl
                    << "avgDiff: " << std::to_string(avgPastPred) << std::endl
                    << "predictions so far: " << std::to_string(predCount) << std::endl;
            }
            pastPred = repairProbability;
        }
        
        // Decoding Stage
        const auto resSize = res.size();
        for (auto i = 0; i < resSize; i++) {
            if constexpr (ROUND_PREDICTION_RESULTS) {
                res[i] = (uint64_t) (res[i] + 0.5f);
            }
            else if constexpr (TEMP_DECODING_STAGE) {
                if (i == 0)
                    res[i] = res[i] >= 0.9 ? 1.0 : 0.0;
                else // i == 1
                    res[i] = res[i] > 0.1 ? 1.0 : 0.0;
            }
            else if constexpr (TEMP_DECODING_STAGE_2) {
                if (i == 0)
                    res[i] = res[i] >= 0.9f
                        ? ((res[i] - 0.9f) * 5.0f) + 0.5f // 0.9-1 -> 0-0.1 -> 0-0.5 -> 0.5-1
                        : (res[i] / 9.0f) * 5.0f; // 0-0.9 -> 0-0.1 -> 0-0.5
                else // i == 1
                    res[i] = res[i] > 0.1f
                        ? (((res[i] - 0.1f) / 9.0f) * 5.0f) + 0.5f // 0.1-1 -> 0-0.9 -> 0-0.1 -> 0-0.5 -> 0.5-1
                        : res[i] * 5.0f; // 0-0.1 -> 0-0.5
            }
            else { // this else needed explicitly cause its part of constexpr.
                // res[i] = res[i];
            }
        }

        if constexpr (IRIS_MODEL) {
            auto setosa = res.at(0);
            auto versi = res.at(1);
            auto virgi = res.at(2);
            tracker.addNewValue("setosa", setosa);
            tracker.addNewValue("versicolor", versi);
            tracker.addNewValue("virginica", virgi);
            if (setosa >= versi && setosa >= virgi)
                tracker.addTally("setosa");
            else if (versi >= setosa && versi >= virgi)
                tracker.addTally("versicolor");
            else if (virgi >= setosa && virgi >= versi)
                tracker.addTally("virginica");
        }
        else if constexpr (NBI_MODEL) {
            auto repair = res.at(0);
            auto nRepair = res.at(1);
            tracker.addNewValue("repair", repair);
            tracker.addNewValue("not_repair", nRepair);
            if (repair > nRepair)
                tracker.addTally("repair");
            else
                tracker.addTally("not_repair");
        }
        else if constexpr (WINE_MODEL) {
            auto quality = res.at(0);
            tracker.addNewValue("quality", quality);
        }

        return 0; // currently unused
        // auto input = std::vector<double>();
        // input.reserve(inputValues.size());
        // for (const auto& i : inputValues) {
        //     double v = (double)(std::holds_alternative<double>(i)
        //         ? std::get<double>(i)
        //         : (std::holds_alternative<int64_t>(i)
        //             ? std::get<int64_t>(i)
        //             : -1.0)
        //     );
        //     input.push_back(v);
        // }
        // double mean = Stats::arithmeticMean(input);

        // auto alignedInput = fdeep::float_vec();
        // alignedInput.reserve(input.size());
        // for (const auto& i : input)
        //     alignedInput.push_back(i);
        // auto sharedAlignedInput = fplus::make_shared_ref<fdeep::float_vec>(alignedInput);
        // auto tensorInput = fdeep::tensor(fdeep::tensor_shape(sharedAlignedInput->size()), sharedAlignedInput);
        // std::cout << fdeep::show_tensor(tensorInput) << std::endl;
        // std::cout << fdeep::show_tensor_shape(tensorInput.shape()) << std::endl;
        // const auto result = model.predict({tensorInput});
        // std::cout << fdeep::show_tensor_shape(result.front().shape()) << std::endl;
        // std::cout << fdeep::show_tensors(result) << std::endl;
        // const fdeep::tensor rez = result.at(0);
        // std::cout << "rez: " << fdeep::show_tensor(rez) << ", " << rez.get(fdeep::tensor_pos(0)) << ", " << rez.get(fdeep::tensor_pos(1)) << std::endl;
        // std::vector<float> res = rez.to_vector();
        // std::cout << "res: ";
        // for (size_t i = 0; i < res.size(); i++) {
        //     std::cout << std::to_string(res.at(i)) << ", ";
        // }
        // std::cout << std::endl;
        // return ((int) mean) % 2 == 0; // faker
    }
}
