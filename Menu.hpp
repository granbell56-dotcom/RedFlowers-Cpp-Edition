#pragma once

#include <vector>
#include <iostream>
#include <string>
#include <thread>
#include <chrono>

#include "utils.hpp"
#include "System.hpp"
#include "Network.hpp"
#include "peek.hpp"
#include "peek_identify.hpp"
#include "adresse.hpp"
#include "redflowers_threads.hpp"
#include "redflowersprocesses.hpp"
#include "threads-ctx.hpp"
#include "RedFlowerPe.hpp"
#include "RedFlowerHandles.hpp"
#include "RedFlowerPEB.hpp"

// V2 de Menu.hpp
// Clarification et mise a niveau PID

extern std::vector<std::string> logger_invalide;

inline void menu(int argc, char* argv[]) {
    RequestAdminPrivileges(argc, argv);

    std::vector<std::string> logger_invalide;
    std::vector<std::string> logger_valide;
    std::vector<std::string> logger_error;
    std::vector<std::string> logger_autre;
    std::vector<std::string> logger_name;

    std::cout << "developed by Luuxo" << std::endl;
    std::cout << "\n" << std::endl;

    std::cout << "┌───────────────[LIST OF BETA TESTERS]──────────────────/─>" << std::endl;
    std::cout << "│" << std::endl;
    std::cout << "│ User name ( Discord ): luuxo2455_17235 | .luuxosozen." << std::endl;
    std::cout << "│ User name ( Instagram ) : levraitwisty ( Twisty ) | luuxo43 ( Luuxo La )" << std::endl;
    std::cout << "│" << std::endl;
    std::cout << "│ [Message from the developer ( LUUXO ) ] Thanks to the beta tester :) " << std::endl;
    std::cout << "│" << std::endl;
    std::cout << "└────────────────────────────────────────────────────────/─>" << std::endl;
    
    std::cout << "\n\n" << std::endl;
    
    std::cout << "Entre ton nom d'utilisateur : ";
    std::string name;
    std::cin >> name;
    
    name = lower(name);
    logger_name.push_back(name);

    std::cout << "\033[2J\033[1;1H";

    while (true) {
        try {
            std::cout << "\n" << std::endl;
            std::cin.exceptions(std::ios::failbit | std::ios::badbit);
            std::string command;

            std::cout << "┌───(RedFlower's@" << name <<")-[~]" << std::endl;
            std::cout << "│" << std::endl;
            std::cout << "└$ ";
            std::cin >> command;

            command = lower(command);
            std::cout << "\n" << std::endl;

            if (command == "get-help") {
                std::cout << "\n┌─/[ Liste des commandes ]─>" << std::endl;
                std::cout << "│" << std::endl;
                std::cout << "│============| Commandes de base |============│" << std::endl;
                std::cout << "│" << std::endl;
                std::cout << "│ clear                     : Nettoie l'écran." << std::endl;
                std::cout << "│ exit                      : Quitte le programme." << std::endl;
                std::cout << "│" << std::endl;
                std::cout << "│============| Informations système |============│" << std::endl;
                std::cout << "│" << std::endl;
                std::cout << "│ get-system-info           : Affiche les informations système." << std::endl;
                std::cout << "│ get-adapter-info          : Affiche les informations des adaptateurs réseau." << std::endl;
                std::cout << "│" << std::endl;
                std::cout << "│============| Analyse mémoire |============│" << std::endl;
                std::cout << "│" << std::endl;
                std::cout << "│ peek [PID] <adresse>      : Identifie ce qu'une adresse mémoire représente." << std::endl;
                std::cout << "│ peek-all [PID]            : Traduit et liste toute la mémoire d'un processus." << std::endl;
                std::cout << "│ adresse                   : Affiche les adresses mémoire." << std::endl;
                std::cout << "│ pe [PID] <adr> [fonction] : Analyse une structure PE en mémoire." << std::endl;
                std::cout << "│ peb [PID]                 : Liste les DLL chargées via le PEB." << std::endl;
                std::cout << "│" << std::endl;
                std::cout << "│============| Processus & Threads |============│" << std::endl;
                std::cout << "│" << std::endl;
                std::cout << "│ threads [PID]             : Énumère et analyse les threads d'un processus." << std::endl;
                std::cout << "│ thread-ctx <TID>          : Affiche le contexte d'un thread." << std::endl;
                std::cout << "│ ps                        : Affiche les processus actifs." << std::endl;
                std::cout << "│ handles [PID]             : Énumère les ressources du système ou d'un PID." << std::endl;
                std::cout << "│" << std::endl;
                std::cout << "└───────────────────────────────────────────────┘" << std::endl;
            }
            else if (command == "clear") {
                std::cout << "\033[2J\033[1;1H";
            }
            else if (command == "exit") {
                break;
            }
            else if (command == "get-system-info") {
                system_info();
            }
            else if (command == "get-adapter-info") {
                adapters_info();
            }
            else if (command == "peek-all") {
                DWORD target_pid = 0;
                std::string pid;
                if (std::cin.peek() != '\n' && std::cin.peek() != EOF) {
                    std::cin >> pid;
                    target_pid = static_cast<DWORD>(std::stoul(pid, nullptr, 0));
                }
                std::string dummy;
                std::getline(std::cin, dummy);
                peek_all_memory(target_pid);
            }
            else if (command.rfind("peek", 0) == 0) {
                std::string arg1, arg2;
                if (std::cin.peek() != '\n' && std::cin.peek() != EOF) std::cin >> arg1;
                if (std::cin.peek() != '\n' && std::cin.peek() != EOF) std::cin >> arg2;

                std::string dummy;
                std::getline(std::cin, dummy);

                DWORD target_pid = 0;
                std::string addr_str = "";

                if (!arg2.empty()) {
                    try {
                        target_pid = std::stoul(arg1, nullptr, 0);
                        addr_str = arg2;
                    } catch (...) {
                        addr_str = arg1;
                    }
                } else {
                    addr_str = arg1;
                }

                if (!addr_str.empty()) {
                    safe_peek_identify(addr_str, target_pid);
                } else {
                    std::cerr << "Erreur : Veuillez spécifier une adresse." << std::endl;
                }
            }
            else if (command == "adresse") {
                show_memory_maps();
            }
            else if (command == "threads") {
                std::string pid_arg;
                if (std::cin.peek() != '\n' && std::cin.peek() != EOF) {
                    std::cin >> pid_arg;
                }
                std::string dummy;
                std::getline(std::cin, dummy);
                list_process_threads(pid_arg);
            }
            else if (command == "ps") {
                list_all_processes();
            }
            else if (command == "thread-ctx") {
                std::string tid = "";
                if (std::cin.peek() != '\n' && std::cin.peek() != EOF) {
                    std::cin >> tid;
                }
                std::string dummy;
                std::getline(std::cin, dummy);
                get_thread_context(tid);
            }
            else if (command == "pe") {
                std::string arg1 = "", arg2 = "", arg3 = "";
                if (std::cin.peek() != '\n' && std::cin.peek() != EOF) std::cin >> arg1;
                if (std::cin.peek() != '\n' && std::cin.peek() != EOF) std::cin >> arg2;
                if (std::cin.peek() != '\n' && std::cin.peek() != EOF) std::cin >> arg3;

                std::string dummy;
                std::getline(std::cin, dummy);

                DWORD target_pid = 0;
                std::string addr_str = "";
                std::string func_name = "";

                if (!arg3.empty()) {
                    try {
                        target_pid = std::stoul(arg1, nullptr, 0);
                        addr_str = arg2;
                        func_name = arg3;
                    } catch (...) {
                        addr_str = arg1;
                        func_name = arg2;
                    }
                } else if (!arg2.empty()) {
                    try {
                        uintptr_t test_val = std::stoull(arg1, nullptr, 16);
                        if (test_val > 0x100000) {
                            addr_str = arg1;
                            func_name = arg2;
                        } else {
                            target_pid = static_cast<DWORD>(std::stoul(arg1, nullptr, 0));
                            addr_str = arg2;
                        }
                    } catch (...) {
                        addr_str = arg1;
                        func_name = arg2;
                    }
                } else {
                    addr_str = arg1;
                }

                if (addr_str.empty()) {
                    std::cerr << "Erreur : Veuillez spécifier une adresse." << std::endl;
                } else {
                    try {
                        uintptr_t addr = std::stoull(addr_str, nullptr, 16);
                        parse_pe_from_memory(addr, func_name, target_pid);
                    } catch (...) {
                        std::cerr << "Erreur : Adresse invalide." << std::endl;
                    }
                }
            }
            else if (command == "handles") {
                std::string pid_str;
                if (std::cin.peek() != '\n' && std::cin.peek() != EOF) {
                    std::cin >> pid_str;
                }
                std::string dummy;
                std::getline(std::cin, dummy);
                DWORD target_pid = 0;
                if (!pid_str.empty()) {
                    try {
                        target_pid = std::stoul(pid_str);
                    } catch (...) {
                        std::cout << "[-] Erreur : PID invalide." << std::endl;
                        continue;
                    }
                }
                enumerate_system_handles(target_pid);
            }
            else if (command == "peb") {
                DWORD pid = 0;
                std::string pid_str;
                if (std::cin.peek() != '\n' && std::cin.peek() != EOF) {
                    std::cin >> pid_str;
                }
                std::string dummy;
                std::getline(std::cin, dummy);

                if (!pid_str.empty()) {
                    try {
                        pid = std::stoul(pid_str, nullptr, 0);
                    } catch (...) {
                        pid = GetCurrentProcessId();
                    }
                }
                enumerate_peb_modules(pid);
            }
            else {
                std::cout << "Erreur la commande '" << command << "' n'existe pas. Tapez 'get-help' pour afficher la liste des commandes." << std::endl;
            }
        }
        catch (const std::exception& e) {
            std::cerr << "Erreur : " << e.what() << std::endl;
            if (std::cin.eof() || std::cin.bad()) {
                break;
            }
            std::cin.clear();
        }
    }
}
