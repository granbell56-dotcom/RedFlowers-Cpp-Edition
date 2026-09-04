#ifndef PROCESS_MAPS_HPP
#define PROCESS_MAPS_HPP

#include <iostream>
#include <iomanip>
#include <string>

#if defined(_WIN32)
#include <windows.h>

// Version 2 ( Changement du comportement, effet sur d'autre processus )
// Désolé d'avoir bridé adresse.hpp

inline void show_memory_maps(DWORD target_pid = 0) {
    HANDLE hProcess = GetCurrentProcess();
    bool is_remote = (target_pid != 0 && target_pid != GetCurrentProcessId());

    if (is_remote) {
        hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, target_pid);
        if (!hProcess) {
            std::cerr << "[-] Erreur : Impossible d'ouvrir le processus PID " << target_pid << " (Erreur: " << GetLastError() << ")" << std::endl;
            return;
        }
    }

    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);

    uint8_t* address = static_cast<uint8_t*>(sysInfo.lpMinimumApplicationAddress);
    uint8_t* maxAddress = static_cast<uint8_t*>(sysInfo.lpMaximumApplicationAddress);

    std::cout << "┌───[VRAIES PAGES MEMOIRE" << (is_remote ? " (PID: " + std::to_string(target_pid) + ")" : " (Courante)") << "]────────────────────────┐" << std::endl;
    std::cout << "│ Adresse Debut  - Fin           | Taille (Ko) | Etat      | Type        │" << std::endl;
    std::cout << "├───────────────────────────────┼─────────────┼───────────┼─────────────┤" << std::endl;

    MEMORY_BASIC_INFORMATION mbi;
    size_t displayedCount = 0;
    const size_t maxDisplay = 100; // Limite pour éviter le spam console

    while (address < maxAddress) {
        if (VirtualQueryEx(hProcess, address, &mbi, sizeof(mbi)) == sizeof(mbi)) {
            if (mbi.State == MEM_COMMIT || mbi.State == MEM_RESERVE) {
                std::cout << "│ 0x" << std::hex << std::uppercase << std::setw(12) << std::setfill('0') 
                          << reinterpret_cast<uintptr_t>(mbi.BaseAddress)
                          << " - 0x" << std::setw(12) << std::setfill('0') 
                          << reinterpret_cast<uintptr_t>(mbi.BaseAddress) + mbi.RegionSize << " │ "
                          << std::dec << std::setw(8) << (mbi.RegionSize / 1024) << " Ko │ ";

                if (mbi.State == MEM_COMMIT) std::cout << "Commit  │ ";
                else if (mbi.State == MEM_RESERVE) std::cout << "Reserve │ ";
                else std::cout << "Free    │ ";

                if (mbi.Type == MEM_IMAGE) std::cout << "Image     │";
                else if (mbi.Type == MEM_MAPPED) std::cout << "Mapped    │";
                else if (mbi.Type == MEM_PRIVATE) std::cout << "Private   │";
                else std::cout << "          │";

                std::cout << std::endl;
                displayedCount++;
                if (displayedCount >= maxDisplay && is_remote) {
                    std::cout << "│ ... (Affichage limite aux " << maxDisplay << " premieres regions)                              │" << std::endl;
                    break;
                }
            }
            address = static_cast<uint8_t*>(mbi.BaseAddress) + mbi.RegionSize;
        } else {
            break;
        }
    }
    std::cout << "└────────────────────────────────────────────────────────────────────────┘" << std::endl;
    std::cout << "\n[Astuce] Copie une adresse de debut et tape : peek <adresse>" << std::endl;

    if (is_remote) {
        CloseHandle(hProcess);
    }
}

#else
inline void show_memory_maps(DWORD target_pid = 0) {
    std::cerr << "Cette fonction necessite Windows." << std::endl;
}
#endif

#endif
