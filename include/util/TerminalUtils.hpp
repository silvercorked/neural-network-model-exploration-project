#pragma once

#include <string>
#include <iostream>
#include <limits>

namespace TerminalUtils {

    auto awaitInput() -> const std::string;

    auto awaitDouble() -> const double;
    constexpr const double ErrorAwaitingDouble = std::numeric_limits<double>::min();

    auto awaitEnter() -> void;

    auto clearTerminal() -> void;

    auto awaitInput() -> const std::string {
        std::cout << ">> ";
        std::string in = "";
        getline(std::cin, in);
        return in;
    }
    auto awaitDouble() -> const double {
        std::string s = awaitInput();
        double o;
        try {
            o = std::stod(s);
        }
        catch(std::invalid_argument ia) {
            return ErrorAwaitingDouble; // invalid value
        }
        return o;
    }
    auto awaitEnter() -> void {
        std::cin.ignore();
    }
    auto clearTerminal() -> void {
        // CSI[2J clears screen, CSI[H moves the cursor to top-left corner
        std::cout << "\x1B[2J\x1B[H";
    }
}
