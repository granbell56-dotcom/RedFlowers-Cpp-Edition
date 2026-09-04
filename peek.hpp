#ifndef SAFE_PEEK_ALL_HPP
#define SAFE_PEEK_ALL_HPP

#include <iostream>
#include <iomanip>
#include <string>
#include <windows.h>

// Version 2 ( Changement du comportement, effet sur d'autre processus )
// Désolé d'avoir bridé peek.hpp

inline std::string translate_state(DWORD state) {
    switch (state) {
        case MEM_COMMIT:  return "Engagee";
        case MEM_RESERVE: return "Reservee";
        case MEM_FREE:    return "Libre";
        default:          return "Inconnu";
    }
}

inline std::string translate_type(DWORD type) {
    switch (type) {
        case MEM_IMAGE:   return "Image (Exe)";
        case MEM_MAPPED:  return "Mappe";
        case MEM_PRIVATE: return "Privee";
        default:          return "Aucun";
    }
}

inline std::string translate_protection(DWORD protect) {
    DWORD base_protect = protect & 0xFF;
    switch (base_protect) {
        case PAGE_NOACCESS:          return "Aucun acces";
        case PAGE_READONLY:          return "Lecture";
        case PAGE_READWRITE:         return "Lect/Ecrit";
        case PAGE_EXECUTE_READ:      return "Exec/Lect";
        case PAGE_EXECUTE_READWRITE: return "Exec/L/E";
        default:                     return "Protege";
    }
}

inline void peek_all_memory(DWORD target_pid = 0) {
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

    std::cout << "┌───[MEMOIRE VIRTUELLE" << (is_remote ? " (PID: " + std::to_string(target_pid) + ")" : " (Courante)") << "]────────────────────────────────────────┐" << std::endl;
    std::cout << "│ Adresse Debut  -> Fin           | Etat       | Type        | Droits        │" << std::endl;
    std::cout << "├─────────────────────────────────┼────────────┼─────────────┼───────────────┤" << std::endl;

    MEMORY_BASIC_INFORMATION mbi;
    size_t displayedCount = 0;
    const size_t maxDisplay = 50; // Limite d'affichage pour éviter le spam console

    while (address < maxAddress) {
        if (VirtualQueryEx(hProcess, address, &mbi, sizeof(mbi)) == sizeof(mbi)) {
            if (mbi.State == MEM_COMMIT || mbi.State == MEM_RESERVE) {
                uintptr_t start = reinterpret_cast<uintptr_t>(mbi.BaseAddress);
                uintptr_t end = start + mbi.RegionSize;

                std::cout << "│ 0x" << std::hex << std::uppercase << std::setw(12) << std::setfill('0') << start 
                          << " -> 0x" << std::setw(8) << std::setfill('0') << end << " │ "
                          << std::dec << std::left << std::setfill(' ')
                          << std::setw(10) << translate_state(mbi.State) << " │ "
                          << std::setw(11) << translate_type(mbi.Type) << " │ "
                          << std::setw(13) << translate_protection(mbi.Protect) << " │"
                          << std::right << std::endl;

                displayedCount++;
                if (displayedCount >= maxDisplay && is_remote) {
                    std::cout << "├─────────────────────────────────┼────────────┼─────────────┼───────────────┤" << std::endl;
                    std::cout << "│ ... (Affichage limite aux " << maxDisplay << " premieres regions memoire)                │" << std::endl;
                    break;
                }
            }
            address = static_cast<uint8_t*>(mbi.BaseAddress) + mbi.RegionSize;
        } else {
            break;
        }
    }
    std::cout << "└──────────────────────────────────────────────────────────────────────────┘" << std::endl;

    if (is_remote) {
        CloseHandle(hProcess);
    }
}

#endif
