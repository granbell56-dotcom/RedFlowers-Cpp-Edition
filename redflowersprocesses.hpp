#ifndef REDFLOWER_PROCESSES_HPP
#define REDFLOWER_PROCESSES_HPP

#include <iostream>
#include <windows.h>
#include <tlhelp32.h>
#include <iomanip>
#include <sstream>
#include <vector>

inline void list_all_processes() {
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        std::cerr << "[-] Erreur : Impossible de creer le snapshot des processus." << std::endl;
        return;
    }

    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);

    if (!Process32First(hSnapshot, &pe32)) {
        std::cerr << "[-] Erreur : Impossible de lire le premier processus." << std::endl;
        CloseHandle(hSnapshot);
        return;
    }

    HANDLE hThreadSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    if (hThreadSnapshot == INVALID_HANDLE_VALUE) {
        std::cerr << "[-] Erreur : Impossible de creer le snapshot des threads." << std::endl;
        CloseHandle(hSnapshot);
        return;
    }

    std::cout << "┌───[LISTE DES PROCESSUS ACTIFS]──────────────────────────────────────┐" << std::endl;
    std::cout << "│ PID         | TID(s)                         | Nom du Processus     │" << std::endl;
    std::cout << "├─────────────┼───────────────────────────────┼──────────────────────┤" << std::endl;

    do {
        std::ostringstream tids;
        THREADENTRY32 te32;
        te32.dwSize = sizeof(THREADENTRY32);

        if (Thread32First(hThreadSnapshot, &te32)) {
            do {
                if (te32.th32OwnerProcessID == pe32.th32ProcessID) {
                    if (tids.tellp() > 0) {
                        tids << ", ";
                    }
                    tids << te32.th32ThreadID;
                }
            } while (Thread32Next(hThreadSnapshot, &te32));
        }

        std::string tidList = tids.str();
        if (tidList.empty()) {
            tidList = "-";
        }
        if (tidList.size() > 29) {
            tidList.resize(26);
            tidList += "...";
        }

        std::cout << "│ " << std::left << std::setw(11) << pe32.th32ProcessID << std::right
                  << " │ " << std::left << std::setw(29) << tidList << std::right
                  << " │ " << std::left << std::setw(20) << pe32.szExeFile << std::right << " │" << std::endl;
    } while (Process32Next(hSnapshot, &pe32));

    std::cout << "└─────────────┴───────────────────────────────┴──────────────────────┘" << std::endl;
    CloseHandle(hThreadSnapshot);
    CloseHandle(hSnapshot);
}

#endif