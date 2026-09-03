#ifndef REDFLOWER_SECURITY_HPP
#define REDFLOWER_SECURITY_HPP

#include <iostream>
#include <windows.h>
#include <tlhelp32.h>
#include <shellapi.h>
#include <iomanip>
#include <string>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <memory>
#include <thread>

// Vérifie si le programme tourne en admin
inline bool IsRunningAsAdmin() {
    BOOL fIsRunAsAdmin = FALSE;
    PSID pAdministratorsGroup = NULL;
    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
    if (AllocateAndInitializeSid(&NtAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS,
                                   0, 0, 0, 0, 0, 0, &pAdministratorsGroup)) {
        CheckTokenMembership(NULL, pAdministratorsGroup, &fIsRunAsAdmin);
        FreeSid(pAdministratorsGroup);
    }
    return fIsRunAsAdmin == TRUE;
}

// Redémarre automatiquement le programme avec UAC si non admin
inline void RequestAdminPrivileges(int argc, char* argv[]) {
    if (!IsRunningAsAdmin()) {
        char szPath[MAX_PATH];
        if (GetModuleFileNameA(NULL, szPath, ARRAYSIZE(szPath))) {
            SHELLEXECUTEINFOA sei = { sizeof(sei) };
            sei.lpVerb = "runas";
            sei.lpFile = szPath;
            sei.nShow = SW_NORMAL;
            if (ShellExecuteExA(&sei)) {
                exit(0); // Quitte l'instance actuelle non-admin
            }
        }
    }
}

// Inspecte les registres d'un thread cible
inline void get_thread_context(const std::string& tid_str) {
    DWORD tid = 0;
    try {
        tid = std::stoul(tid_str, nullptr, 0);
    } catch (...) {
        std::cerr << "Erreur : ID de thread (TID) invalide." << std::endl;
        return;
    }

    struct ContextState {
        CONTEXT context;
        std::mutex mutex;
        std::condition_variable ready;
        bool completed = false;
        bool success = false;
        DWORD errorCode = ERROR_SUCCESS;
    };
    auto state = std::make_shared<ContextState>();
    ZeroMemory(&state->context, sizeof(CONTEXT));

    std::thread worker([state, tid]() {
        HANDLE hThread = OpenThread(
            THREAD_GET_CONTEXT | THREAD_SUSPEND_RESUME | THREAD_QUERY_INFORMATION,
            FALSE,
            tid
        );

        if (!hThread) {
            state->errorCode = GetLastError();
        } else {
            const bool suspended = SuspendThread(hThread) != (DWORD)-1;
            if (!suspended) {
                state->errorCode = GetLastError();
            } else {
                state->context.ContextFlags = CONTEXT_FULL;
                state->success = GetThreadContext(hThread, &state->context) == TRUE;
                if (!state->success) {
                    state->errorCode = GetLastError();
                }
                ResumeThread(hThread);
            }
            CloseHandle(hThread);
        }

        {
            std::lock_guard<std::mutex> lock(state->mutex);
            state->completed = true;
        }
        state->ready.notify_one();
    });

    {
        std::unique_lock<std::mutex> lock(state->mutex);
        if (!state->ready.wait_for(lock, std::chrono::seconds(3), [&]() { return state->completed; })) {
            std::cerr << "[-] Delai depasse : lecture du contexte annulee apres 3 secondes." << std::endl;
            worker.detach();
            return;
        }
    }
    worker.join();

    if (state->success) {
        std::cout << "┌───[REGISTRES DU THREAD (TID: " << std::hex << std::uppercase << tid << ")]──────┐" << std::endl;
        #if defined(_WIN64)
        std::cout << "│ RIP : 0x" << std::setw(16) << std::setfill('0') << state->context.Rip << std::endl;
        std::cout << "│ RSP : 0x" << std::setw(16) << std::setfill('0') << state->context.Rsp << std::endl;
        std::cout << "│ RBP : 0x" << std::setw(16) << std::setfill('0') << state->context.Rbp << std::endl;
        std::cout << "│ RAX : 0x" << std::setw(14) << std::setfill('0') << state->context.Rax << " | RBX : 0x" << state->context.Rbx << std::endl;
        std::cout << "│ RCX : 0x" << std::setw(14) << std::setfill('0') << state->context.Rcx << " | RDX : 0x" << state->context.Rdx << std::endl;
        #else
        std::cout << "│ EIP : 0x" << std::setw(8) << std::setfill('0') << state->context.Eip << std::endl;
        std::cout << "│ ESP : 0x" << std::setw(8) << std::setfill('0') << state->context.Esp << std::endl;
        std::cout << "│ EBP : 0x" << std::setw(8) << std::setfill('0') << state->context.Ebp << std::endl;
        #endif
        std::cout << "└────────────────────────────────────────────────────────┘" << std::endl;
    } else {
        if (state->errorCode == ERROR_ACCESS_DENIED || state->errorCode == ERROR_INVALID_PARAMETER) {
            std::cerr << "[-] Erreur : Impossible d'ouvrir le thread (acces refuse ou TID inexistant)." << std::endl;
        } else {
            std::cerr << "[-] Erreur : Impossible de lire le contexte du thread (code "
                      << std::dec << state->errorCode << ")." << std::endl;
        }
    }
}

#endif