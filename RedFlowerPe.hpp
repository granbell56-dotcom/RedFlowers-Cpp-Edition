#ifndef REDFLOWER_PE_HPP
#define REDFLOWER_PE_HPP

#include <iostream>
#include <windows.h>
#include <winnt.h>
#include <iomanip>
#include <string>
#include <algorithm>

inline void parse_pe_from_memory(uintptr_t input_address, const std::string& target_function = "") {
    MEMORY_BASIC_INFORMATION mbi;
    if (VirtualQuery(reinterpret_cast<LPCVOID>(input_address), &mbi, sizeof(mbi)) == 0) {
        std::cerr << "[-] Erreur : Impossible d'interroger la mémoire (VirtualQuery échoué)." << std::endl;
        return;
    }

    uintptr_t base_address = reinterpret_cast<uintptr_t>(mbi.AllocationBase);
    if (!base_address) {
        std::cerr << "[-] Erreur : Adresse de base introuvable." << std::endl;
        return;
    }

    PIMAGE_DOS_HEADER dos_header = reinterpret_cast<PIMAGE_DOS_HEADER>(base_address);
    if (dos_header->e_magic != IMAGE_DOS_SIGNATURE) {
        std::cerr << "[-] Erreur : Ce bloc ne contient pas d'en-tête PE valide (MZ manquant)." << std::endl;
        return;
    }

    PIMAGE_NT_HEADERS nt_headers = reinterpret_cast<PIMAGE_NT_HEADERS>(base_address + dos_header->e_lfanew);
    if (nt_headers->Signature != IMAGE_NT_SIGNATURE) {
        std::cerr << "[-] Erreur : Signature NT invalide (PE manquant)." << std::endl;
        return;
    }

    // Si on cherche une fonction spécifique, on peut alléger l'affichage global
    if (target_function.empty()) {
        std::cout << "┌───[ANALYSE PE AUTOMATIQUE]────────────────────────────────────────┐" << std::endl;
        std::cout << "│ Adresse saisie : 0x" << std::hex << std::uppercase << input_address << std::endl;
        std::cout << "│ Base détectée  : 0x" << base_address 
                  << " | Sections : " << std::dec << nt_headers->FileHeader.NumberOfSections << std::endl;
        std::cout << "├───────────────────┬──────────────┬──────────────┬────────────────┤" << std::endl;
        std::cout << "│ Nom de section    │ VirtAddr     │ Taille Virt  │ Permissions    │" << std::endl;
        std::cout << "├───────────────────┼──────────────┼──────────────┼────────────────┤" << std::endl;

        PIMAGE_SECTION_HEADER section_header = IMAGE_FIRST_SECTION(nt_headers);
        for (WORD i = 0; i < nt_headers->FileHeader.NumberOfSections; ++i) {
            char name[9] = {0};
            memcpy(name, section_header[i].Name, 8);

            DWORD ch = section_header[i].Characteristics;
            std::string flags = "";
            if (ch & IMAGE_SCN_MEM_EXECUTE) flags += "R-X ";
            else if (ch & IMAGE_SCN_MEM_READ) flags += "R-- ";
            if (ch & IMAGE_SCN_MEM_WRITE) flags += "W";

            std::cout << "│ " << std::left << std::setw(17) << name 
                      << " │ 0x" << std::hex << std::setw(8) << std::setfill('0') << section_header[i].VirtualAddress
                      << " │ 0x" << std::setw(8) << section_header[i].Misc.VirtualSize
                      << " │ " << std::left << std::setw(14) << std::setfill(' ') << flags 
                      << std::dec << " │" << std::endl;
        }
        std::cout << "└───────────────────┴──────────────┴──────────────┴────────────────┘" << std::endl;
    }

    IMAGE_DATA_DIRECTORY export_dir_info = nt_headers->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];
    if (export_dir_info.VirtualAddress != 0 && export_dir_info.Size != 0) {
        PIMAGE_EXPORT_DIRECTORY export_dir = reinterpret_cast<PIMAGE_EXPORT_DIRECTORY>(base_address + export_dir_info.VirtualAddress);
        
        DWORD* functions = reinterpret_cast<DWORD*>(base_address + export_dir->AddressOfFunctions);
        DWORD* names = reinterpret_cast<DWORD*>(base_address + export_dir->AddressOfNames);
        WORD* name_ordinals = reinterpret_cast<WORD*>(base_address + export_dir->AddressOfNameOrdinals);

        // MODE RECHERCHE CIBLÉE (Partielle)
        if (!target_function.empty()) {
            std::cout << "┌───[RÉSULTATS DE LA RECHERCHE (Filtre : \"" << target_function << "\")]──────┐" << std::endl;
            std::cout << "│ Nom de la fonction                     │ Adresse mémoire         │" << std::endl;
            std::cout << "├────────────────────────────────────────┼─────────────────────────┤" << std::endl;

            bool found = false;
            int match_count = 0;

            for (DWORD i = 0; i < export_dir->NumberOfNames; ++i) {
                std::string func_name(reinterpret_cast<char*>(base_address + names[i]));
                
                // Utilisation de .find() pour une recherche partielle
                if (func_name.find(target_function) != std::string::npos) {
                    WORD ordinal_index = name_ordinals[i];
                    uintptr_t func_address = base_address + functions[ordinal_index];

                    if (func_name.length() > 38) {
                        func_name = func_name.substr(0, 35) + "...";
                    }

                    std::cout << "│ " << std::left << std::setw(38) << func_name
                              << " │ 0x" << std::hex << std::uppercase << std::setw(16) << std::setfill('0') << func_address 
                              << std::setfill(' ') << " │" << std::endl;
                    
                    found = true;
                    match_count++;
                    
                    // Limiter l'affichage à 30 résultats max pour garder un affichage propre
                    if (match_count >= 30) {
                        std::cout << "├────────────────────────────────────────┴─────────────────────────┤" << std::endl;
                        std::cout << "│ ... (Trop de résultats, affichage limité aux 30 premiers)        │" << std::endl;
                        break;
                    }
                }
            }

            if (!found) {
                std::cout << "│ Aucune fonction ne correspond à \"" << target_function << "\"          │" << std::endl;
            }
            std::cout << "└──────────────────────────────────────────────────────────────────┘" << std::endl;
            return;
        }

        // MODE LISTE CLASSIQUE (si aucun nom n'est spécifié)
        std::cout << "┌───[TABLE D'EXPORT (Fonctions exposées)]──────────────────────────┐" << std::endl;
        std::cout << "│ Total fonctions : " << std::dec << export_dir->NumberOfFunctions 
                  << " | Nommées : " << export_dir->NumberOfNames << std::endl;
        std::cout << "├────────────────────────────────────────┬─────────────────────────┤" << std::endl;
        std::cout << "│ Nom de la fonction                     │ Adresse mémoire         │" << std::endl;
        std::cout << "├────────────────────────────────────────┼─────────────────────────┤" << std::endl;

        DWORD limit = (export_dir->NumberOfNames < 30) ? export_dir->NumberOfNames : 30;

        for (DWORD i = 0; i < limit; ++i) {
            std::string func_name(reinterpret_cast<char*>(base_address + names[i]));
            WORD ordinal_index = name_ordinals[i];
            uintptr_t func_address = base_address + functions[ordinal_index];

            if (func_name.length() > 38) {
                func_name = func_name.substr(0, 35) + "...";
            }

            std::cout << "│ " << std::left << std::setw(38) << func_name
                      << " │ 0x" << std::hex << std::uppercase << std::setw(16) << std::setfill('0') << func_address 
                      << std::setfill(' ') << " │" << std::endl;
        }

        if (export_dir->NumberOfNames > 30) {
            std::cout << "├────────────────────────────────────────┴─────────────────────────┤" << std::endl;
            std::cout << "│ ... (Affichage limité aux 30 premières sur " << std::dec << export_dir->NumberOfNames << " fonctions)      │" << std::endl;
        }
        std::cout << "└──────────────────────────────────────────────────────────────────┘" << std::endl;
    } else if (!target_function.empty()) {
        std::cout << "[-] Ce module ne possède pas de Table d'Export." << std::endl;
    }
}

#endif