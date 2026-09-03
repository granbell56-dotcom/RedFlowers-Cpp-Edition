#pragma once

#include <windows.h>
#include <iostream>

#include "CPU.hpp"
#include "RAM.hpp"

void system_info() {

    // Nom du PC
    char nom[256];
    DWORD taille = sizeof(nom);

    if (GetComputerNameA(nom, &taille)) {
        nom[taille] = '\0'; // Ajoute un caractère nul à la fin du nom
    } else {
        std::cerr << "Erreur lors de la récupération du nom du PC." << std::endl;
    }
    SYSTEM_INFO info;
    GetSystemInfo(&info);


    std::cout << "┌─/[ "<< nom <<" ]/─>" << std::endl;
    std::cout << "│" << std::endl;
    std::cout << "│=========[ CPU ]===========" << std::endl;
    std::cout << "│" << std::endl;
    cpu_info();
    std::cout << "│" << std::endl;
    std::cout << "│ CPU threads                 : " << info.dwNumberOfProcessors << std::endl;
    std::cout << "│" << std::endl;
    std::cout << "│ Architecture CPU            : " << info.wProcessorArchitecture << std::endl;
    std::cout << "│" << std::endl;
    std::cout << "│===========[ RAM / MÉMOIRE ( Virtuelle ) / Espace d'adressage ]===========" << std::endl;
    std::cout << "│" << std::endl;
    ram_info();
    std::cout << "│" << std::endl;
    std::cout << "│ Taille page ( RAM )         : " << info.dwPageSize << " octets" << std::endl;
    std::cout << "│" << std::endl;
    std::cout << "│ Granularité d'allocation    : " << info.dwAllocationGranularity << std::endl;
    std::cout << "│" << std::endl;
    std::cout << "│ Adresse minimum application : " << info.lpMinimumApplicationAddress << std::endl;
    std::cout << "│" << std::endl;
    std::cout << "│ Adresse maximum application : " << info.lpMaximumApplicationAddress << std::endl;
    std::cout << "│" << std::endl;
    std::cout << "└────────────────────────────────────────────────────────/─>" << std::endl;


}