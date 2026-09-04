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
                std::cout << "┌─/[Liste des commandes]/─>" << std::endl;
                std::cout << "│" << std::endl;
                std::cout << "│============| Commandes de base |============│" << std::endl;
                std::cout << "│" << std::endl;
                std::cout << "│ clear            : Nettoyer l'écran." << std::endl;
                std::cout << "│ exit             : Sortir du programme." << std::endl;
                std::cout << "│" << std::endl;
                std::cout << "│============| Informations système |============│" << std::endl;
                std::cout << "│" << std::endl;
                std::cout << "│ get-system-info  : Afficher les informations système" << std::endl;
                std::cout << "│" << std::endl;
                std::cout << "│ get-adapter-info : Afficher les informations sur les adaptateurs réseau" << std::endl;
                std::cout << "│" << std::endl;
                std::cout << "│" << std::endl;
                std::cout << "│============| Commande peek |============│" << std::endl;
                std::cout << "│" << std::endl;
                std::cout << "│ peek <adresse>   : Identifie à quoi correspond une adresse mémoire" << std::endl;
                std::cout << "│" << std::endl;
                std::cout << "│ peek-all         : Traduit et liste toute la mémoire du processus" << std::endl;
                std::cout << "│" << std::endl;
                std::cout << "│ adresse : Affiche les adresse memoire" << std::endl;
                std::cout << "│" << std::endl;
                std::cout << "│============| Commande Threads/Processus|============│" << std::endl;
                std::cout << "│" << std::endl;
                std::cout << "│ threads <PID>     : Énumération et analyse du Threads" << std::endl;
                std::cout << "│" << std::endl;
                std::cout << "│ ps                : Affiche tous les programmes en cours d'exécution avec leur PID et leur nombre de threads" << std::endl;
                std::cout << "│" << std::endl;
                std::cout << "│ thread-ctx" << std::endl;
                std::cout << "└───────────────────────────────────────────│" << std::endl;
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
            else if(command == "peek-all") {
                peek_all_memory();
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
            else if(command == "threads") {
            std::string pid = "";

            // Récupère l'argument s'il y en a un dans la ligne
            if (std::cin.peek() != '\n' && std::cin.peek() != EOF) {
                std::cin >> pid;

            }

            // Nettoie le reste de la ligne pour éviter les résidus
            std::string dummy;
            std::getline(std::cin, dummy);
            list_process_threads(pid);

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
        }
        catch (const std::exception& e)
        {
            std::cerr << "Error: " << e.what() << std::endl;
        }
    
    }

}
