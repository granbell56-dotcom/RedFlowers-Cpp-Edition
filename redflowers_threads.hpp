#pragma once

#ifndef REDFLOWER_THREADS_HPP
#define REDFLOWER_THREADS_HPP

#include <iostream>
#include <windows.h>
#include <tlhelp32.h>
#include <iomanip>
#include <string>


// Version 2 ( Thread et PID )
// Cette commande prend en charge le Thread PID redflowers_threads.hpp

inline void list_process_threads(const std::string& pid_str = "") {
    DWORD target_pid = 0;
    
    if (pid_str.empty()) {
        target_pid = GetCurrentProcessId();
    } else {
        try {
            target_pid = std::stoul(pid_str, nullptr, 0);
        } catch (...) {
            std::cerr << "Erreur : PID invalide." << std::endl;
            return;
        }
    }

    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        std::cerr << "[-] Erreur : Impossible de creer le snapshot des threads." << std::endl;
        return;
    }

    THREADENTRY32 te32;
    te32.dwSize = sizeof(THREADENTRY32);

    if (!Thread32First(hSnapshot, &te32)) {
        std::cerr << "[-] Erreur : Impossible de lire le premier thread." << std::endl;
        CloseHandle(hSnapshot);
        return;
    }

    std::cout << "┌───[LISTE DES THREADS DU PROCESSUS (PID: " << target_pid << ")]──────────────┐" << std::endl;
    std::cout << "│ ID du Thread (TID)  | PID Proprietaire  | Priorite de Base         │" << std::endl;
    std::cout << "├─────────────────────┼───────────────────┼─────────────────────────┤" << std::endl;

    bool found = false;
    do {
        if (te32.th32OwnerProcessID == target_pid) {
            found = true;
            std::cout << "│ 0x" << std::hex << std::uppercase << std::setw(15) << std::setfill('0') << te32.th32ThreadID 
                      << " │ " << std::dec << std::setw(17) << std::setfill(' ') << te32.th32OwnerProcessID 
                      << " │ " << std::setw(23) << te32.tpBasePri 
                      << " │" << std::endl;
        }
    } while (Thread32Next(hSnapshot, &te32));

    if (!found) {
        std::cout << "│ Aucun thread trouve pour ce PID.                        │" << std::endl;
    }

    std::cout << "└─────────────────────────────────────────────────────────────────┘" << std::endl;
    CloseHandle(hSnapshot);
}

#endif
