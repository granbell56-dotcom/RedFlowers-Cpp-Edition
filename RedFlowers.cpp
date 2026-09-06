#include <iostream>
#include <string>

#include <thread>
#include <chrono>

#include "PONT.hpp"
#include "color.hpp"

int main(int argc, char* argv[]) {

    configure_console();
    std::cout << "\033[2J\033[1;1H";

    std::cout << Color::DARK_GRAY <<  "RedFlower's - C++ Edition" << Color::RESET << std::endl;

    std::cout  << "\n" << std::endl;

    std::this_thread::sleep_for(std::chrono::seconds(1));

    liaison(argc, argv);

    return 0;

}
