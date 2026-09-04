#ifndef SAFE_PEEK_IDENTIFY_HPP
#define SAFE_PEEK_IDENTIFY_HPP

#include <iostream>
#include <string>
#include <iomanip>
#include <windows.h>
#include <psapi.h>

// Version 2 ( Changement du comportement, effet sur d'autre processus )
// Désolé d'avoir bridé peek_identify.hpp

inline void safe_peek_identify(const std::string& address_str, DWORD target_pid = 0) {
    uintptr_t address = 0;
    try {
        address = std::stoull(address_str, nullptr, 16);
    } catch (...) {
        std::cerr << "Erreur : Format hexadecimal invalide (ex: 0x7FFE...)" << std::endl;
        return;
    }

    HANDLE hProcess = GetCurrentProcess();
    bool is_remote = (target_pid != 0 && target_pid != GetCurrentProcessId());

    if (is_remote) {
        hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, target_pid);
        if (!hProcess) {
            std::cerr << "[-] Erreur : Impossible d'ouvrir le processus PID " << target_pid << " (Erreur: " << GetLastError() << ")" << std::endl;
            return;
        }
    }

    MEMORY_BASIC_INFORMATION mbi;
    if (VirtualQueryEx(hProcess, reinterpret_cast<LPCVOID>(address), &mbi, sizeof(mbi)) == 0) {
        std::cerr << "[-] Erreur : Cette adresse n'est pas mappee en memoire pour ce processus." << std::endl;
        if (is_remote) CloseHandle(hProcess);
        return;
    }

    // Recherche si l'adresse appartient a un module (EXE ou DLL) charge
    std::string module_name = "Inconnu (Memoire dynamique / Heap / Stack)";
    HMODULE hMods[1024];
    DWORD cbNeeded;

    if (EnumProcessModules(hProcess, hMods, sizeof(hMods), &cbNeeded)) {
        for (unsigned int i = 0; i < (cbNeeded / sizeof(HMODULE)); i++) {
            MODULEINFO modInfo;
            if (GetModuleInformation(hProcess, hMods[i], &modInfo, sizeof(modInfo))) {
                uintptr_t mod_start = reinterpret_cast<uintptr_t>(modInfo.lpBaseOfDll);
                uintptr_t mod_end = mod_start + modInfo.SizeOfImage;

                if (address >= mod_start && address < mod_end) {
                    char szFileName[MAX_PATH];
                    if (GetModuleFileNameExA(hProcess, hMods[i], szFileName, sizeof(szFileName))) {
                        module_name = szFileName;
                    }
                    break;
                }
            }
        }
    }

    // Affichage du rapport d'identification
    std::cout << "┌───[IDENTIFICATION DE L'ADRESSE" << (is_remote ? " (PID: " + std::to_string(target_pid) + ")" : " (Courante)") << "]─── 0x" << std::hex << std::uppercase << address << std::dec << std::endl;
    std::cout << "│ [Appartenance] : " << module_name << std::endl;
    std::cout << "│ [Plage memoire]: 0x" << std::hex << std::uppercase << reinterpret_cast<uintptr_t>(mbi.BaseAddress) 
              << " -> 0x" << (reinterpret_cast<uintptr_t>(mbi.BaseAddress) + mbi.RegionSize) << std::dec << std::endl;
    std::cout << "│ [Taille bloc]  : " << (mbi.RegionSize / 1024) << " Ko" << std::endl;
    
    // Type de memoire
    std::string type_str = "Inconnu";
    if (mbi.Type == MEM_IMAGE) type_str = "Image (Code binaire d'un fichier)";
    else if (mbi.Type == MEM_MAPPED) type_str = "Fichier mappe en memoire";
    else if (mbi.Type == MEM_PRIVATE) type_str = "Privee (Heap, Stack ou allocation dynamique)";
    std::cout << "│ [Type de zone] : " << type_str << std::endl;

    // Droits d'acces
    std::string protect_str = "Normal";
    if (mbi.Protect & PAGE_EXECUTE_READWRITE) protect_str = "Execution / Lecture / Ecriture";
    else if (mbi.Protect & PAGE_READWRITE) protect_str = "Lecture / Ecriture";
    else if (mbi.Protect & PAGE_READONLY) protect_str = "Lecture seule";
    else if (mbi.Protect & PAGE_NOACCESS) protect_str = "Protege (Aucun acces)";
    std::cout << "│ [Permissions]  : " << protect_str << std::endl;
    std::cout << "└───────────────────────────────────────────────────────────────┘" << std::endl;

    if (is_remote) {
        CloseHandle(hProcess);
    }
}

#endif
