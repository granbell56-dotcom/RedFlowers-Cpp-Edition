#ifndef REDFLOWER_PEB_HPP
#define REDFLOWER_PEB_HPP

#include <iostream>
#include <windows.h>
#include <winternl.h>
#include <iomanip>
#include <string>

// Structures non documentées pour la liste de chargement du PEB (LDR)
typedef struct _LDR_DATA_TABLE_ENTRY_CUSTOM {
    LIST_ENTRY InLoadOrderLinks;
    LIST_ENTRY InMemoryOrderLinks;
    LIST_ENTRY InInitializationOrderLinks;
    PVOID DllBase;
    PVOID EntryPoint;
    ULONG SizeOfImage;
    UNICODE_STRING FullDllName;
    UNICODE_STRING BaseDllName;
    // ... d'autres champs non nécessaires ici
} LDR_DATA_TABLE_ENTRY_CUSTOM, *PLDR_DATA_TABLE_ENTRY_CUSTOM;

// winternl.h intentionally exposes only a partial PEB_LDR_DATA definition.
// Use the documented initial layout to access the load-order list.
typedef struct _PEB_LDR_DATA_CUSTOM {
    ULONG Length;
    BOOLEAN Initialized;
    BYTE Reserved1[3];
    PVOID SsHandle;
    LIST_ENTRY InLoadOrderModuleList;
} PEB_LDR_DATA_CUSTOM, *PPEB_LDR_DATA_CUSTOM;

inline void enumerate_peb_modules() {
    std::cout << "┌───[ÉNUMÉRATION PEB / MODULES CHARGÉS (Interne)]────────────────────┐" << std::endl;
    std::cout << "│ Base DLL       │ Taille    │ Nom du Module                       │" << std::endl;
    std::cout << "├────────────────┼───────────┼─────────────────────────────────────┤" << std::endl;

#if defined(_WIN64)
    PPEB pPeb = reinterpret_cast<PPEB>(__readgsqword(0x60));
#else
    PPEB pPeb = reinterpret_cast<PPEB>(__readfsdword(0x30));
#endif

    if (!pPeb || !pPeb->Ldr) {
        std::cout << "[-] Erreur : Impossible d'accéder au PEB ou à la structure Ldr." << std::endl;
        std::cout << "└───────────────────────────────────────────────────────────────────┘" << std::endl;
        return;
    }

    PPEB_LDR_DATA_CUSTOM pLdr = reinterpret_cast<PPEB_LDR_DATA_CUSTOM>(pPeb->Ldr);
    PLIST_ENTRY pListHead = &pLdr->InLoadOrderModuleList;
    PLIST_ENTRY pListEntry = pListHead->Flink;

    size_t count = 0;
    while (pListEntry != pListHead) {
        PLDR_DATA_TABLE_ENTRY_CUSTOM pEntry = CONTAINING_RECORD(pListEntry, LDR_DATA_TABLE_ENTRY_CUSTOM, InLoadOrderLinks);

        if (pEntry->DllBase) {
            std::wstring wName(pEntry->BaseDllName.Buffer, pEntry->BaseDllName.Length / sizeof(WCHAR));
            std::string name(wName.begin(), wName.end());

            if (name.empty()) {
                name = "<Inconnu>";
            }

            if (name.length() > 33) {
                name = name.substr(0, 30) + "...";
            }

            std::cout << "│ 0x" << std::hex << std::uppercase << std::setw(12) << std::setfill('0') << reinterpret_cast<uintptr_t>(pEntry->DllBase)
                      << " │ 0x" << std::setw(7) << pEntry->SizeOfImage
                      << " │ " << std::setfill(' ') << std::left << std::setw(35) << name << " │" << std::endl;
            count++;
        }

        pListEntry = pListEntry->Flink;
        // Sécurité pour éviter les boucles infinies ou listes corrompues
        if (count > 200) break; 
    }

    std::cout << "└───────────────────────────────────────────────────────────────────┘" << std::endl;
}

#endif