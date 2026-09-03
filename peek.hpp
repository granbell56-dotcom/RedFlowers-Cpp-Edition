#ifndef SAFE_PEEK_ALL_HPP
#define SAFE_PEEK_ALL_HPP

#include <iostream>
#include <iomanip>
#include <string>
#include <windows.h>

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

inline void peek_all_memory() {
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);

    uint8_t* address = static_cast<uint8_t*>(sysInfo.lpMinimumApplicationAddress);
    uint8_t* maxAddress = static_cast<uint8_t*>(sysInfo.lpMaximumApplicationAddress);

    std::cout << "┌───[TRADUCTION COMPLETE DE TOUTE LA MEMOIRE]──────────────────────────────┐" << std::endl;
    std::cout << "│ Adresse Debut  -> Fin           | Etat       | Type        | Droits        │" << std::endl;
    std::cout << "├─────────────────────────────────┼────────────┼─────────────┼───────────────┤" << std::endl;

    MEMORY_BASIC_INFORMATION mbi;
    while (address < maxAddress) {
        if (VirtualQuery(address, &mbi, sizeof(mbi)) == sizeof(mbi)) {
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
            }
            address = static_cast<uint8_t*>(mbi.BaseAddress) + mbi.RegionSize;
        } else {
            break;
        }
    }
    std::cout << "└──────────────────────────────────────────────────────────────────────────┘" << std::endl;
}

#endif