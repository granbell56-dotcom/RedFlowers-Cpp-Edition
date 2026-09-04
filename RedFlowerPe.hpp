#ifndef REDFLOWER_PE_HPP
#define REDFLOWER_PE_HPP

#include <iostream>
#include <windows.h>
#include <winnt.h>
#include <iomanip>
#include <string>
#include <algorithm>


// Version 3 mise a niveau
// Désolé d'avoir mal fixé la V2 de RedFlowerPe.hpp 

inline void parse_pe_from_memory(uintptr_t input_address, const std::string& target_function = "", DWORD target_pid = 0) {
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
    if (VirtualQueryEx(hProcess, reinterpret_cast<LPCVOID>(input_address), &mbi, sizeof(mbi)) == 0) {
        std::cerr << "[-] Erreur : Impossible d'interroger la mémoire (VirtualQueryEx échoué)." << std::endl;
        if (is_remote) CloseHandle(hProcess);
        return;
    }

    uintptr_t base_address = reinterpret_cast<uintptr_t>(mbi.AllocationBase);
    if (!base_address) {
        std::cerr << "[-] Erreur : Adresse de base introuvable." << std::endl;
        if (is_remote) CloseHandle(hProcess);
        return;
    }

    IMAGE_DOS_HEADER dos_header;
    if (!ReadProcessMemory(hProcess, reinterpret_cast<LPCVOID>(base_address), &dos_header, sizeof(dos_header), NULL) || dos_header.e_magic != IMAGE_DOS_SIGNATURE) {
        std::cerr << "[-] Erreur : Ce bloc ne contient pas d'en-tête PE valide (MZ manquant)." << std::endl;
        if (is_remote) CloseHandle(hProcess);
        return;
    }

    IMAGE_NT_HEADERS nt_headers;
    uintptr_t nt_headers_addr = base_address + dos_header.e_lfanew;
    if (!ReadProcessMemory(hProcess, reinterpret_cast<LPCVOID>(nt_headers_addr), &nt_headers, sizeof(nt_headers), NULL) || nt_headers.Signature != IMAGE_NT_SIGNATURE) {
        std::cerr << "[-] Erreur : Signature NT invalide (PE manquant)." << std::endl;
        if (is_remote) CloseHandle(hProcess);
        return;
    }

    if (target_function.empty()) {
        std::cout << "┌───[ANALYSE PE AUTOMATIQUE" << (is_remote ? " (PID: " + std::to_string(target_pid) + ")" : " (Courante)") << "]──────────────────────────────────┐" << std::endl;
        std::cout << "│ Adresse saisie : 0x" << std::hex << std::uppercase << input_address << std::endl;
        std::cout << "│ Base détectée  : 0x" << base_address 
                  << " | Sections : " << std::dec << nt_headers.FileHeader.NumberOfSections << std::endl;
        std::cout << "├───────────────────┬──────────────┬──────────────┬────────────────┤" << std::endl;
        std::cout << "│ Nom de section    │ VirtAddr     │ Taille Virt  │ Permissions    │" << std::endl;
        std::cout << "├───────────────────┼──────────────┼──────────────┼────────────────┤" << std::endl;

        size_t sections_size = nt_headers.FileHeader.NumberOfSections * sizeof(IMAGE_SECTION_HEADER);
        auto* section_headers = new IMAGE_SECTION_HEADER[nt_headers.FileHeader.NumberOfSections];
        uintptr_t sections_addr = nt_headers_addr + FIELD_OFFSET(IMAGE_NT_HEADERS, OptionalHeader) + nt_headers.FileHeader.SizeOfOptionalHeader;

        if (ReadProcessMemory(hProcess, reinterpret_cast<LPCVOID>(sections_addr), section_headers, sections_size, NULL)) {
            for (WORD i = 0; i < nt_headers.FileHeader.NumberOfSections; ++i) {
                char name[9] = {0};
                memcpy(name, section_headers[i].Name, 8);

                DWORD ch = section_headers[i].Characteristics;
                std::string flags = "";
                if (ch & IMAGE_SCN_MEM_EXECUTE) flags += "R-X ";
                else if (ch & IMAGE_SCN_MEM_READ) flags += "R-- ";
                if (ch & IMAGE_SCN_MEM_WRITE) flags += "W";

                std::cout << "│ " << std::left << std::setw(17) << name 
                          << " │ 0x" << std::hex << std::setw(8) << std::setfill('0') << section_headers[i].VirtualAddress
                          << " │ 0x" << std::setw(8) << section_headers[i].Misc.VirtualSize
                          << " │ " << std::left << std::setw(14) << std::setfill(' ') << flags 
                          << std::dec << " │" << std::endl;
            }
        }
        delete[] section_headers;
        std::cout << "└───────────────────┴──────────────┴──────────────┴────────────────┘" << std::endl;
    }

    IMAGE_DATA_DIRECTORY export_dir_info = nt_headers.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];
    if (export_dir_info.VirtualAddress != 0 && export_dir_info.Size != 0) {
        IMAGE_EXPORT_DIRECTORY export_dir;
        if (ReadProcessMemory(hProcess, reinterpret_cast<LPCVOID>(base_address + export_dir_info.VirtualAddress), &export_dir, sizeof(export_dir), NULL)) {
            
            auto* functions = new DWORD[export_dir.NumberOfFunctions];
            auto* names = new DWORD[export_dir.NumberOfNames];
            auto* name_ordinals = new WORD[export_dir.NumberOfNames];

            ReadProcessMemory(hProcess, reinterpret_cast<LPCVOID>(base_address + export_dir.AddressOfFunctions), functions, export_dir.NumberOfFunctions * sizeof(DWORD), NULL);
            ReadProcessMemory(hProcess, reinterpret_cast<LPCVOID>(base_address + export_dir.AddressOfNames), names, export_dir.NumberOfNames * sizeof(DWORD), NULL);
            ReadProcessMemory(hProcess, reinterpret_cast<LPCVOID>(base_address + export_dir.AddressOfNameOrdinals), name_ordinals, export_dir.NumberOfNames * sizeof(WORD), NULL);

            if (!target_function.empty()) {
                std::cout << "┌───[RÉSULTATS DE LA RECHERCHE (Filtre : \"" << target_function << "\")]──────┐" << std::endl;
                std::cout << "│ Nom de la fonction                     │ Adresse mémoire         │" << std::endl;
                std::cout << "├────────────────────────────────────────┼─────────────────────────┤" << std::endl;

                bool found = false;
                int match_count = 0;

                for (DWORD i = 0; i < export_dir.NumberOfNames; ++i) {
                    char func_name_buf[256] = {0};
                    ReadProcessMemory(hProcess, reinterpret_cast<LPCVOID>(base_address + names[i]), func_name_buf, sizeof(func_name_buf) - 1, NULL);
                    std::string func_name(func_name_buf);

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
            } else {
                std::cout << "┌───[TABLE D'EXPORT (Fonctions exposées)]──────────────────────────┐" << std::endl;
                std::cout << "│ Total fonctions : " << std::dec << export_dir.NumberOfFunctions 
                          << " | Nommées : " << export_dir.NumberOfNames << std::endl;
                std::cout << "├────────────────────────────────────────┬─────────────────────────┤" << std::endl;
                std::cout << "│ Nom de la fonction                     │ Adresse mémoire         │" << std::endl;
                std::cout << "├────────────────────────────────────────┼─────────────────────────┤" << std::endl;

                DWORD limit = (export_dir.NumberOfNames < 30) ? export_dir.NumberOfNames : 30;

                for (DWORD i = 0; i < limit; ++i) {
                    char func_name_buf[256] = {0};
                    ReadProcessMemory(hProcess, reinterpret_cast<LPCVOID>(base_address + names[i]), func_name_buf, sizeof(func_name_buf) - 1, NULL);
                    std::string func_name(func_name_buf);

                    WORD ordinal_index = name_ordinals[i];
                    uintptr_t func_address = base_address + functions[ordinal_index];

                    if (func_name.length() > 38) {
                        func_name = func_name.substr(0, 35) + "...";
                    }

                    std::cout << "│ " << std::left << std::setw(38) << func_name
                              << " │ 0x" << std::hex << std::uppercase << std::setw(16) << std::setfill('0') << func_address 
                              << std::setfill(' ') << " │" << std::endl;
                }

                if (export_dir.NumberOfNames > 30) {
                    std::cout << "├────────────────────────────────────────┴─────────────────────────┤" << std::endl;
                    std::cout << "│ ... (Affichage limité aux 30 premières sur " << std::dec << export_dir.NumberOfNames << " fonctions)      │" << std::endl;
                }
                std::cout << "└──────────────────────────────────────────────────────────────────┘" << std::endl;
            }

            delete[] functions;
            delete[] names;
            delete[] name_ordinals;
        }
    } else if (!target_function.empty()) {
        std::cout << "[-] Ce module ne possède pas de Table d'Export." << std::endl;
    }

    if (is_remote) {
        CloseHandle(hProcess);
    }
}

#endif
