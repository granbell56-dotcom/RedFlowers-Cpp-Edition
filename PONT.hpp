#pragma once

#include <iostream>
#include <string>

# include "Menu.hpp"

void liaison(int argc, char** argv) {

    try
    {
    menu(argc, argv);
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what();
    }
    
}