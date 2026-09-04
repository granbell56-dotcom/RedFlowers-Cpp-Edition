#pragma once

#ifndef REDFLOWER_THREADS_HPP
#define REDFLOWER_THREADS_HPP

#include <iostream>
#include <windows.h>
#include <tlhelp32.h>
#include <iomanip>
#include <string>

// Version 3.2 ( Thread, PID unifiés, accessibilité vérifiée et anti-crash blindé )
inline void list_process_threads(const std::string& pid_str = "") {
    try {
        DWORD target_pid = 0;
        
        if (pid_str.empty()) {
            target_pid = GetCurrentProcessId();
        } else {
            try {
                target_pid = static_cast<DWORD>(std::stoul(pid_str, nullptr, 0));
            } catch (...) {
                std::cerr << "[-] Erreur : Format de PID invalide ('" << pid_str << "')." << std::endl;
                return;
            }
        }

        HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
        if (hSnapshot == INVALID_HANDLE_VALUE) {
            std::cerr << "[-] Erreur : Impossible de creer le snapshot des threads (Code: " << GetLastError() << ")." << std::endl;
            return;
        }

        THREADENTRY32 te32;
        te32.dwSize = sizeof(THREADENTRY32);

        if (!Thread32First(hSnapshot, &te32)) {
            std::cerr << "[-] Erreur : Impossible de lire le premier thread." << std::endl;
            CloseHandle(hSnapshot);
            return;
        }

        std::cout << "\n┌───[LISTE DES THREADS DU PROCESSUS (PID: " << target_pid << ")]──────────────┐" << std::endl;
        std::cout << "│ ID du Thread (TID)  | Priorite de Base  | Accessibilité           │" << std::endl;
        std::cout << "├─────────────────────┼───────────────────┼─────────────────────────┤" << std::endl;

        bool found = false;
        DWORD thread_count = 0;

        do {
            if (te32.th32OwnerProcessID == target_pid) {
                found = true;
                thread_count++;

                // Sécurisation de l'ouverture du handle (gère les threads terminés à la volée)
                HANDLE hThread = OpenThread(THREAD_QUERY_INFORMATION, FALSE, te32.th32ThreadID);
                std::string status_str = "Accessible";
                if (hThread != NULL) {
                    CloseHandle(hThread);
                } else {
                    status_str = "Inaccessible";
                }

                std::cout << "│ 0x" << std::hex << std::uppercase << std::setw(15) << std::setfill('0') << te32.th32ThreadID 
                          << " │ " << std::dec << std::setw(17) << std::setfill(' ') << te32.tpBasePri 
                          << " │ " << std::left << std::setw(23) << status_str << std::right << " │" << std::endl;
            }
        } while (Thread32Next(hSnapshot, &te32));

        if (!found) {
            std::cout << "│ Aucun thread trouve ou processus introuvable pour ce PID.       │" << std::endl;
        }

        std::cout << "├─────────────────────────────────────────────────────────────────┤" << std::endl;
        std::cout << "│ Total des threads listés : " << std::dec << thread_count << std::setw(34) << "│" << std::endl;
        std::cout << "└─────────────────────────────────────────────────────────────────┘" << std::endl;
        
        CloseHandle(hSnapshot);
    }
    catch (const std::exception& e) {
        std::cerr << "[-] Erreur critique interceptée dans list_process_threads : " << e.what() << std::endl;
    }
    catch (...) {
        std::cerr << "[-] Erreur critique inconnue interceptée dans list_process_threads." << std::endl;
    }
}

#endif
