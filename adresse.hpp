#ifndef PROCESS_MAPS_HPP
#define PROCESS_MAPS_HPP

#include <iostream>
#include <iomanip>

#if defined(_WIN32)
#include <windows.h>

inline void show_memory_maps() {
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);

    uint8_t* address = static_cast<uint8_t*>(sysInfo.lpMinimumApplicationAddress);
    uint8_t* maxAddress = static_cast<uint8_t*>(sysInfo.lpMaximumApplicationAddress);

    std::cout << "┌───[VRAIES PAGES MEMOIRE (Windows VirtualQuery)]────────────────────────┐" << std::endl;
    std::cout << "│ Adresse Debut  - Fin          | Taille (Ko) | Etat      | Type        │" << std::endl;
    std::cout << "├───────────────────────────────┼─────────────┼───────────┼─────────────┤" << std::endl;

    MEMORY_BASIC_INFORMATION mbi;
    while (address < maxAddress) {
        if (VirtualQuery(address, &mbi, sizeof(mbi)) == sizeof(mbi)) {
            // Filtrer pour n'afficher que les blocs alloués (commit ou reserve)
            if (mbi.State == MEM_COMMIT || mbi.State == MEM_RESERVE) {
                std::cout << "│ 0x" << std::hex << std::uppercase << std::setw(12) << std::setfill('0') 
                          << reinterpret_cast<uintptr_t>(mbi.BaseAddress)
                          << " - 0x" << std::setw(12) << std::setfill('0') 
                          << reinterpret_cast<uintptr_t>(mbi.BaseAddress) + mbi.RegionSize << " │ "
                          << std::dec << std::setw(8) << (mbi.RegionSize / 1024) << " Ko │ ";

                // État de la mémoire
                if (mbi.State == MEM_COMMIT) std::cout << "Commit  │ ";
                else if (mbi.State == MEM_RESERVE) std::cout << "Reserve │ ";
                else std::cout << "Free    │ ";

                // Type de mémoire
                if (mbi.Type == MEM_IMAGE) std::cout << "Image     │";
                else if (mbi.Type == MEM_MAPPED) std::cout << "Mapped    │";
                else if (mbi.Type == MEM_PRIVATE) std::cout << "Private   │";
                else std::cout << "          │";

                std::cout << std::endl;
            }
            // Passage à la région mémoire suivante
            address = static_cast<uint8_t*>(mbi.BaseAddress) + mbi.RegionSize;
        } else {
            break;
        }
    }
    std::cout << "└────────────────────────────────────────────────────────────────────────┘" << std::endl;
    std::cout << "\n[Astuce] Copie une adresse de debut (ex: 0x00007FF6...) et tape : peek <adresse>" << std::endl;
}

#else
inline void show_memory_maps() {
    std::cerr << "Cette fonction necessite Windows." << std::endl;
}
#endif

#endif