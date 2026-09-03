#pragma once

#include <windows.h>
#include <iostream>

void ram_info() {

    MEMORYSTATUSEX mem;
    mem.dwLength = sizeof(mem);

    if (GlobalMemoryStatusEx(&mem)) {

        std::cout << "│ RAM totale : "
                  << mem.ullTotalPhys / (1024 * 1024 * 1024)
                  << " GB" << std::endl;

        std::cout << "│" << std::endl;

        std::cout << "│ RAM disponible : "
                  << mem.ullAvailPhys / (1024 * 1024 * 1024)
                  << " GB" << std::endl;

        std::cout << "│" << std::endl;

        std::cout << "│ RAM utilisée : "
                  << (mem.ullTotalPhys - mem.ullAvailPhys) / (1024 * 1024 * 1024)
                  << " GB" << std::endl;

        std::cout << "│" << std::endl;

        std::cout << "│ RAM utilisée : "
                  << mem.dwMemoryLoad
                  << "%" << std::endl;
    } else {
        std::cerr << "Impossible de récupérer les informations RAM."
                  << std::endl;
    }
}