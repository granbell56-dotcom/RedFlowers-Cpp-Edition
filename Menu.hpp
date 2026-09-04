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

extern std::vector<std::string> logger_invalide;
void menu(int argc, char* argv[]) {
    RequestAdminPrivileges(argc, argv);
    // Log

    std::vector<std::string> logger_invalide;
    std::vector<std::string> logger_valide;
    std::vector<std::string> logger_error;
    std::vector<std::string> logger_autre;
    std::vector<std::string> logger_name;


    std::cout << "developed by Luuxo" << std::endl;

    std::cout  << "\n" << std::endl;

    std::cout << "┌───────────────[LIST OF BETA TESTERS]──────────────────/─>" << std::endl;
    std::cout << "│" << std::endl;
    std::cout << "│" << std::endl;
    std::cout << "│ User name ( Discord ): luuxo2455_17235 | .luuxosozen." << std::endl;
    std::cout << "│ User name ( Instagram ) : levraitwisty ( Twisty ) | luuxo43 ( Luuxo La )" << std::endl;
    std::cout << "│" << std::endl;
    std::cout << "│" << std::endl;
    std::cout << "│ [Message from the developer ( LUUXO ) ] Thanks to the beta tester :) " << std::endl;
    std::cout << "│" << std::endl;
    std::cout << "│" << std::endl;
    std::cout << "└────────────────────────────────────────────────────────/─>" << std::endl;
    
    std::cout  << "\n" << std::endl;
    std::cout  << "\n" << std::endl;
    
    std::cout << "Entre ton nom d'utilisateur : ";
    std::string name;
    std::cin >> name;
    
    name = lower(name); // Convertit le nom en minuscules pour une comparaison insensible à la casse
    
    logger_name.push_back(name);

    std::cout << "\033[2J\033[1;1H"; // Efface l'écran et place le curseur en haut à gauche

    while (true) {
        
        try
        {
            std::cout << "\n" << std::endl;
            
            std::cin.exceptions(std::ios::failbit | std::ios::badbit); // Active les exceptions pour std::cin
            std::string command;

            std::cout << "┌───(RedFlower's@"<<name<<")-[~]" << std::endl;
            std::cout << "│" << std::endl;
            std::cout << "└$ ";
            std::cin >> command;

            command = lower(command); // Convertit la commande en minuscules pour une comparaison insensible à la casse

            std::cout << "\n" << std::endl;

            if(command == "get-help") {
                std::cout << "" << std::endl;
                std::cout << "┌─/[ Liste des commandes ]─>" << std::endl;
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
                std::cout << "│ peek [PID] <adresse>      : Identifie ce qu'une adresse mémoire représente (courante ou distante)." << std::endl;
                std::cout << "│ peek-all [PID]            : Traduit et liste toute la mémoire d'un processus." << std::endl;
                std::cout << "│ adresse                   : Affiche les adresses mémoire." << std::endl;
                std::cout << "│ pe [PID] <adr> [fonction] : Analyse une structure PE en mémoire (courante ou distante)." << std::endl;
                std::cout << "│ peb [PID]                 : Liste les DLL chargées via le PEB d'un processus." << std::endl;
                std::cout << "│" << std::endl;
                std::cout << "│============| Processus & Threads |============│" << std::endl;
                std::cout << "│" << std::endl;
                std::cout << "│ threads [PID]             : Énumère et analyse les threads d'un processus." << std::endl;
                std::cout << "│ thread-ctx <TID>          : Affiche le contexte d'un thread (registre, état, CPU)." << std::endl;
                std::cout << "│ ps                        : Affiche les processus actifs avec leur PID et leur nombre de threads." << std::endl;
                std::cout << "│ handles [PID]             : Énumère les ressources du système ou d'un PID." << std::endl;
                std::cout << "│" << std::endl;
                std::cout << "└───────────────────────────────────────────────┘" << std::endl;
            }
            else if(command == "clear") {
                std::cout << "\033[2J\033[1;1H"; // Efface l'écran et place le curseur en haut à gauche
            }
            else if(command == "exit") {
                break; // Sort de la boucle while et termine le programme
            }
            else if(command == "get-system-info") {
                system_info();
            }
            else if(command == "get-adapter-info") {
                adapters_info();
            }
            else if (command == "peek-all") {
                DWORD target_pid = 0;
                std::string pid;

                if (std::cin.peek() != '\n' && std::cin.peek() != EOF) {
                    std::cin >> pid;
                    target_pid = static_cast<DWORD>(std::stoul(pid, nullptr, 0));
                }
                peek_all_memory(target_pid); // ou le nom de ta fonction pour peek-all
            }
            else if(command.rfind("peek", 0) == 0) {
                std::string addr;
                if (std::cin >> addr) {
                    safe_peek_identify(addr);
                }
            }
            else if(command == "adresse") {
                show_memory_maps();
            }
            else if (command == "threads") {
                std::string pid_arg;
                if (std::cin.peek() != '\n' && std::cin.peek() != EOF) {
                    std::cin >> pid_arg;
                }
                list_process_threads(pid_arg);
            }
            else if(command == "ps") {
                list_all_processes();

            }
            else if(command == "thread-ctx") {
                std::string tid = "";
                
                if (std::cin.peek() != '\n' && std::cin.peek() != EOF) {
                    std::cin >> tid;
                
                }
                
                std::string dummy;
                std::getline(std::cin, dummy);
                
                get_thread_context(tid);
            
            }
            else if (command == "pe") {
                std::string addr_str = "";
                std::string func_name = "";
                
                // 1. Lire l'adresse si elle existe
                
                if (std::cin.peek() != '\n' && std::cin.peek() != EOF) {
                    std::cin >> addr_str;
                }
                
                // 2. Lire le nom de la fonction optionnel s'il est présent
                if (std::cin.peek() != '\n' && std::cin.peek() != EOF) {
                    std::cin >> func_name;
                }
                
                // Vider le reste de la ligne
                std::string dummy;
                std::getline(std::cin, dummy);
                
                if (addr_str.empty()) {
                    std::cerr << "Erreur : Veuillez specifier une adresse (ex: pe 0x7FFA07FA2000 [NomFonction])" << std::endl;
                
                }
                else {
                    
                    try {
                        uintptr_t addr = std::stoull(addr_str, nullptr, 16);
                        parse_pe_from_memory(addr, func_name); // On passe le nom (éventuellement vide)
                    
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
            if (command == "peb") {
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
                        pid = GetCurrentProcessId(); // Fallback ou gestion d'erreur
                    }
                }
                
                enumerate_peb_modules(pid);
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
