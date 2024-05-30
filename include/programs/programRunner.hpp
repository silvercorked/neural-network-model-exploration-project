#pragma once

#include <functional>
#include <iostream>
#include <string>
#include <vector>

#include <sys/wait.h> // for wait

#include "TerminalUtils.hpp"
#include "fullSearch.hpp"
#include "manualFileSearch.hpp"
#include "manualUserSearch.hpp"
#include "calculateHstatistic.hpp"

namespace ProgramRunner {
    const std::vector<std::pair<std::string, std::function<int()>>> programList = {
        std::make_pair<std::string, std::function<int()>>(
            std::string("Full Search"),
            std::function<int()>(FullSearch::program)
        ),
        std::make_pair<std::string, std::function<int()>>(
            std::string("Manual File Search"),
            std::function<int()>(ManualFileSearch::program)
        ),
        std::make_pair<std::string, std::function<int()>>(
            std::string("Manual User Search"),
            std::function<int()>(ManualUserSearch::program)
        ),
        std::make_pair<std::string, std::function<int()>>(
            std::string("Calculate H-Statistic"),
            std::function<int()>(Calculate::HStatistic::program)
        )
    };

    auto programRunner() -> int;
    auto listPrograms() -> void;
    auto selectProgram(uint32_t) -> int;
    auto runProgram(std::function<int()> func) -> int;

    auto programRunner() -> int {
        std::string error = "";
        Start_Label:
        listPrograms();
        if (error != "") {
            std::cout << error << std::endl;
            error = "";
        }
        int input;
        try {
            std::string str = TerminalUtils::awaitInput();
            if (str == "exit") // exit, else attempt to run program
                return 0;
            input = std::stoi(str);
        }
        catch (std::invalid_argument ia) {
            error = "Invalid Option: Input must be a number or 'exit'.";
            goto Start_Label;
        }
        int res = selectProgram(input);
        if (res == -42)
            error = "Invalid Option: Please select a number between 0 and " + std::to_string(programList.size() - 1) + ".";
        else if (res <= 0) // program failed
            error = "Program Failed.";
        goto Start_Label;
        return 1;
    }
    auto listPrograms() -> void {
        //TerminalUtils::clearTerminal();
        std::cout << "Machine Learning Model Analysis Tool" << std::endl;
        std::cout << "\tprograms available to run:" << std::endl;
        for (auto i = 0; i < programList.size(); i++) {
            std::cout << "\t" << i << ": " << programList.at(i).first << std::endl;
        }
        std::cout << "Select program using associated number (or type 'exit' to quit)." << std::endl;
    }
    auto selectProgram(uint32_t programIndex) -> int {
        if (programIndex >= 0 && programIndex < programList.size()) {
            return runProgram(programList.at(programIndex).second);
        }
        return -42; // invalid input
    }
    auto runProgram(std::function<int()> func) -> int {
        int pid = fork();
        if (pid == -1) // failed to create fork
            return false;
        else if (pid == 0) { // child
            int exitCode = func();
            exit(exitCode);
        }
        else { // parent
            int wstatus = 0;
            waitpid(pid, &wstatus, 0);
            return WEXITSTATUS(wstatus);
        }
    }
    
}