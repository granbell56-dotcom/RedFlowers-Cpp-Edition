#ifndef REDFLOWER_HANDLES_HPP
#define REDFLOWER_HANDLES_HPP

#include <iostream>
#include <windows.h>
#include <winternl.h>
#include <iomanip>
#include <string>
#include <vector>

// Déclarations et structures non documentées pour NtQuerySystemInformation
#define SystemExtendedHandleInformation 64

// Certaines versions des en-têtes Windows ne déclarent pas ce statut NT.
#ifndef STATUS_INFO_LENGTH_MISMATCH
#define STATUS_INFO_LENGTH_MISMATCH ((NTSTATUS)0xC0000004L)
#endif

typedef struct _SYSTEM_HANDLE_TABLE_ENTRY_INFO_EX {
    PVOID Object;
    ULONG_PTR UniqueProcessId;
    ULONG_PTR HandleValue;
    ULONG AccessMasks;
    USHORT ObjectTypeIndex;
    USHORT HandleAttributes;
    ULONG Reserved;
} SYSTEM_HANDLE_TABLE_ENTRY_INFO_EX, *PSYSTEM_HANDLE_TABLE_ENTRY_INFO_EX;

typedef struct _SYSTEM_HANDLE_INFORMATION_EX {
    ULONG_PTR NumberOfHandles;
    ULONG_PTR Reserved;
    SYSTEM_HANDLE_TABLE_ENTRY_INFO_EX Handles[1];
} SYSTEM_HANDLE_INFORMATION_EX, *PSYSTEM_HANDLE_INFORMATION_EX;

// Pointeurs de fonctions non documentées
typedef NTSTATUS(NTAPI* pfnNtQuerySystemInformation)(
    ULONG SystemInformationClass,
    PVOID SystemInformation,
    ULONG SystemInformationLength,
    PULONG ReturnLength
);

typedef NTSTATUS(NTAPI* pfnNtQueryObject)(
    HANDLE Handle,
    OBJECT_INFORMATION_CLASS ObjectInformationClass,
    PVOID ObjectInformation,
    ULONG ObjectInformationLength,
    PULONG ReturnLength
);

// Fonction pour activer SeDebugPrivilege
inline bool EnableDebugPrivilege() {
    HANDLE hToken;
    TOKEN_PRIVILEGES tp;
    LUID luid;

    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) {
        return false;
    }

    if (!LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &luid)) {
        CloseHandle(hToken);
        return false;
    }

    tp.PrivilegeCount = 1;
    tp.Privileges[0].Luid = luid;
    tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    if (!AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(tp), NULL, NULL)) {
        CloseHandle(hToken);
        return false;
    }

    CloseHandle(hToken);
    return true;
}

inline void enumerate_system_handles(DWORD target_pid = 0) {
    if (!EnableDebugPrivilege()) {
        std::cerr << "[-] Attention : Impossible d'activer SeDebugPrivilege (certains handles SYSTEM/processus protégés échoueront)." << std::endl;
    }

    HMODULE hNtdll = GetModuleHandleA("ntdll.dll");
    if (!hNtdll) {
        std::cerr << "[-] Erreur : Impossible de charger ntdll.dll." << std::endl;
        return;
    }

    auto NtQuerySystemInformation = (pfnNtQuerySystemInformation)GetProcAddress(hNtdll, "NtQuerySystemInformation");
    auto NtQueryObject = (pfnNtQueryObject)GetProcAddress(hNtdll, "NtQueryObject");

    if (!NtQuerySystemInformation || !NtQueryObject) {
        std::cerr << "[-] Erreur : Résolution des fonctions NT impossible." << std::endl;
        return;
    }

    // Allocation dynamique du buffer pour les handles
    ULONG bufferSize = 0x10000;
    PVOID buffer = malloc(bufferSize);
    NTSTATUS status;

    while ((status = NtQuerySystemInformation(SystemExtendedHandleInformation, buffer, bufferSize, &bufferSize)) == STATUS_INFO_LENGTH_MISMATCH) {
        buffer = realloc(buffer, bufferSize);
    }

    if (status < 0) {
        std::cerr << "[-] Erreur : NtQuerySystemInformation a échoué (Status: 0x" << std::hex << status << ")" << std::endl;
        free(buffer);
        return;
    }

    auto handleInfo = reinterpret_cast<PSYSTEM_HANDLE_INFORMATION_EX>(buffer);

    std::cout << "┌───[ÉNUMÉRATION DES RESSOURCES NOYAU]──────────────────────────────┐" << std::endl;
    if (target_pid != 0) {
        std::cout << "│ Filtre PID : " << std::dec << target_pid << " | Total handles système : " << handleInfo->NumberOfHandles << std::endl;
    } else {
        std::cout << "│ Total handles système brut : " << std::dec << handleInfo->NumberOfHandles << std::endl;
    }
    std::cout << "├────────┬──────────┬────────────────────┬─────────────────────────┤" << std::endl;
    std::cout << "│ PID    │ Handle   │ Type d'Objet       │ Nom / Chemin / Réseau   │" << std::endl;
    std::cout << "├────────┼──────────┼────────────────────┼─────────────────────────┤" << std::endl;

    size_t displayedCount = 0;
    const size_t maxDisplay = 40; // Limite d'affichage pour éviter de saturer la console

    for (ULONG_PTR i = 0; i < handleInfo->NumberOfHandles; ++i) {
        auto& handleEntry = handleInfo->Handles[i];

        if (target_pid != 0 && handleEntry.UniqueProcessId != target_pid) {
            continue;
        }

        // Ouvrir le processus distant pour dupliquer le handle
        HANDLE hProcess = OpenProcess(PROCESS_DUP_HANDLE | PROCESS_QUERY_INFORMATION, FALSE, (DWORD)handleEntry.UniqueProcessId);
        if (!hProcess) continue;

        HANDLE hDupHandle = NULL;
        if (!DuplicateHandle(hProcess, (HANDLE)handleEntry.HandleValue, GetCurrentProcess(), &hDupHandle, 0, FALSE, DUPLICATE_SAME_ACCESS)) {
            CloseHandle(hProcess);
            continue;
        }

        // Récupérer le type d'objet
        BYTE typeNameBuf[256] = {0};
        ULONG returnLength = 0;
        std::wstring objType = L"Inconnu";

        if (NtQueryObject(hDupHandle, ObjectTypeInformation, typeNameBuf, sizeof(typeNameBuf), &returnLength) >= 0) {
            auto objTypeInfo = reinterpret_cast<POBJECT_TYPE_INFORMATION>(typeNameBuf);
            if (objTypeInfo->TypeName.Buffer) {
                objType = std::wstring(objTypeInfo->TypeName.Buffer, objTypeInfo->TypeName.Length / sizeof(WCHAR));
            }
        }

        // Récupérer le nom de l'objet
        BYTE nameBuf[512] = {0};
        std::wstring objName = L"";

        if (NtQueryObject(hDupHandle, ObjectNameInformation, nameBuf, sizeof(nameBuf), &returnLength) >= 0) {
            auto objNameInfo = reinterpret_cast<PUNICODE_STRING>(nameBuf);
            if (objNameInfo && objNameInfo->Buffer) {
                objName = std::wstring(objNameInfo->Buffer, objNameInfo->Length / sizeof(WCHAR));
            }
        }

        std::string strType(objType.begin(), objType.end());

        // Affinement spécifique pour les Sockets via le pilote AFD de Windows
        if (strType == "File" && objName.find(L"\\Device\\Afd") != std::wstring::npos) {
            strType = "Socket (AFD)";
        }

        // Filtrer les types pertinents (Fichiers, Sockets AFD, Mutex, Sections)
        if (strType == "File" || strType == "Socket (AFD)" || strType == "Mutant" || strType == "Section") {
            std::string strName(objName.begin(), objName.end());

            if (strName.length() > 23) {
                strName = strName.substr(0, 20) + "...";
            }

            std::cout << "│ " << std::left << std::setw(6) << std::dec << handleEntry.UniqueProcessId
                      << " │ 0x" << std::hex << std::uppercase << std::setw(6) << std::setfill('0') << handleEntry.HandleValue
                      << " │ " << std::setfill(' ') << std::left << std::setw(18) << strType
                      << " │ " << std::left << std::setw(23) << (strName.empty() ? "<Sans Nom>" : strName) << " │" << std::endl;

            displayedCount++;
            if (displayedCount >= maxDisplay && target_pid == 0) {
                std::cout << "├────────┴──────────┴────────────────────┴─────────────────────────┤" << std::endl;
                std::cout << "│ ... (Affichage limité aux " << maxDisplay << " premiers résultats. Utilise un PID) │" << std::endl;
                CloseHandle(hDupHandle);
                CloseHandle(hProcess);
                break;
            }
        }

        CloseHandle(hDupHandle);
        CloseHandle(hProcess);
    }

    std::cout << "└────────┴──────────┴────────────────────┴─────────────────────────┘" << std::endl;
    free(buffer);
}

#endif