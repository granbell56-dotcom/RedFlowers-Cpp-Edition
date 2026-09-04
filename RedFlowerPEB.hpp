#ifndef REDFLOWER_PEB_HPP
#define REDFLOWER_PEB_HPP

#include <iostream>
#include <windows.h>
#include <winternl.h>
#include <iomanip>
#include <string>

// Version 3 (Parcours robuste et sécurisé du PEB distant via ReadProcessMemory)

typedef struct _LDR_DATA_TABLE_ENTRY_CUSTOM {
    LIST_ENTRY InLoadOrderLinks;
    LIST_ENTRY InMemoryOrderLinks;
    LIST_ENTRY InInitializationOrderLinks;
    PVOID DllBase;
    PVOID EntryPoint;
    ULONG SizeOfImage;
    UNICODE_STRING FullDllName;
    UNICODE_STRING BaseDllName;
} LDR_DATA_TABLE_ENTRY_CUSTOM, *PLDR_DATA_TABLE_ENTRY_CUSTOM;

typedef struct _PEB_LDR_DATA_CUSTOM {
    ULONG Length;
    BOOLEAN Initialized;
    BYTE Reserved1[3];
    PVOID SsHandle;
    LIST_ENTRY InLoadOrderModuleList;
} PEB_LDR_DATA_CUSTOM, *PPEB_LDR_DATA_CUSTOM;

typedef struct _PROCESS_BASIC_INFORMATION_CUSTOM {
    NTSTATUS Reserved1;
    PPEB PebBaseAddress;
    ULONG_PTR Reserved2[2];
    ULONG_PTR UniqueProcessId;
    ULONG_PTR Reserved3;
} PROCESS_BASIC_INFORMATION_CUSTOM;

inline void enumerate_peb_modules(DWORD target_pid = 0) {
    bool is_remote = (target_pid != 0 && target_pid != GetCurrentProcessId());
    HANDLE hProcess = GetCurrentProcess();

    if (is_remote) {
        hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, target_pid);
        if (!hProcess) {
            std::cout << "[-] Erreur : Impossible d'ouvrir le processus PID " << target_pid << " (Erreur: " << GetLastError() << ")" << std::endl;
            return;
        }
    }

    std::cout << "┌───[ÉNUMÉRATION PEB" << (is_remote ? " (PID: " + std::to_string(target_pid) + ")" : " (Courante)") << "]───────────────────────────────────┐" << std::endl;
    std::cout << "│ Base DLL       │ Taille    │ Nom du Module                     │" << std::endl;
    std::cout << "├────────────────┼───────────┼─────────────────────────────────────┤" << std::endl;

    PPEB pPeb = nullptr;

    if (is_remote) {
        typedef NTSTATUS(NTAPI* pfnNtQueryInformationProcess)(HANDLE, ULONG, PVOID, ULONG, PULONG);
        auto NtQueryInformationProcess = 
            reinterpret_cast<pfnNtQueryInformationProcess>(GetProcAddress(GetModuleHandleA("ntdll.dll"), "NtQueryInformationProcess"));

        if (NtQueryInformationProcess) {
            PROCESS_BASIC_INFORMATION_CUSTOM pbi;
            ULONG returnLength = 0;
            NTSTATUS status = NtQueryInformationProcess(hProcess, 0, &pbi, sizeof(pbi), &returnLength);
            if (status >= 0) {
                pPeb = pbi.PebBaseAddress;
            }
        }
    } else {
#if defined(_WIN64)
        pPeb = reinterpret_cast<PPEB>(__readgsqword(0x60));
#else
        pPeb = reinterpret_cast<PPEB>(__readfsdword(0x30));
#endif
    }

    if (!pPeb) {
        std::cout << "[-] Erreur : Impossible d'accéder au PEB du processus." << std::endl;
        std::cout << "└───────────────────────────────────────────────────────────────────┘" << std::endl;
        if (is_remote) CloseHandle(hProcess);
        return;
    }

    PVOID pLdrRemote = nullptr;
    PPEB remotePebPtr = pPeb;
    if (!ReadProcessMemory(hProcess, &(remotePebPtr->Ldr), &pLdrRemote, sizeof(PVOID), NULL) || !pLdrRemote) {
        std::cout << "[-] Erreur : Impossible de lire la structure Ldr distante." << std::endl;
        std::cout << "└───────────────────────────────────────────────────────────────────┘" << std::endl;
        if (is_remote) CloseHandle(hProcess);
        return;
    }

    PEB_LDR_DATA_CUSTOM ldrData;
    if (!ReadProcessMemory(hProcess, pLdrRemote, &ldrData, sizeof(PEB_LDR_DATA_CUSTOM), NULL)) {
        std::cout << "[-] Erreur : Impossible de lire les données Ldr." << std::endl;
        std::cout << "└───────────────────────────────────────────────────────────────────┘" << std::endl;
        if (is_remote) CloseHandle(hProcess);
        return;
    }

    // Correction V3 : Résolution de l'adresse absolue de la tête de liste dans l'espace distant
    uintptr_t pListHeadRemote = reinterpret_cast<uintptr_t>(pLdrRemote) + offsetof(PEB_LDR_DATA_CUSTOM, InLoadOrderModuleList);
    LIST_ENTRY* pCurrentRemote = ldrData.InLoadOrderModuleList.Flink;

    size_t count = 0;
    while (reinterpret_cast<uintptr_t>(pCurrentRemote) != pListHeadRemote && count < 200) {
        BYTE* pEntryRemote = reinterpret_cast<BYTE*>(pCurrentRemote) - offsetof(LDR_DATA_TABLE_ENTRY_CUSTOM, InLoadOrderLinks);

        LDR_DATA_TABLE_ENTRY_CUSTOM entry;
        if (!ReadProcessMemory(hProcess, pEntryRemote, &entry, sizeof(LDR_DATA_TABLE_ENTRY_CUSTOM), NULL)) {
            break;
        }

        if (entry.DllBase) {
            std::string name = "<Inconnu>";
            if (entry.BaseDllName.Buffer && entry.BaseDllName.Length > 0) {
                std::wstring wName;
                wName.resize(entry.BaseDllName.Length / sizeof(WCHAR));
                if (ReadProcessMemory(hProcess, entry.BaseDllName.Buffer, &wName[0], entry.BaseDllName.Length, NULL)) {
                    name = std::string(wName.begin(), wName.end());
                }
            }

            if (name.length() > 33) {
                name = name.substr(0, 30) + "...";
            }

            std::cout << "│ 0x" << std::hex << std::uppercase << std::setw(12) << std::setfill('0') << reinterpret_cast<uintptr_t>(entry.DllBase)
                      << " │ 0x" << std::setw(7) << entry.SizeOfImage
                      << " │ " << std::setfill(' ') << std::left << std::setw(35) << name << " │" << std::endl;
            count++;
        }

        pCurrentRemote = entry.InLoadOrderLinks.Flink;
    }

    std::cout << "└───────────────────────────────────────────────────────────────────┘" << std::endl;

    if (is_remote) {
        CloseHandle(hProcess);
    }
}

#endif
