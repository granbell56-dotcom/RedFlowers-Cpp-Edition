# RedFlower's - C++ Edition

Outil Windows en ligne de commande pour observer le système, les processus et
certaines structures bas niveau d'un processus. L'application fonctionne comme
un shell interactif et affiche ses résultats directement dans le terminal.

## Fonctionnalités

- Affichage des informations système : nom du PC, processeur, architecture,
  nombre de processeurs logiques, mémoire physique et espace d'adressage.
- Énumération des adaptateurs réseau avec leur nom, description, IPv4, masque,
  passerelle et adresse MAC.
- Cartographie de la mémoire virtuelle avec `VirtualQueryEx`, pour le processus
  courant ou un PID spécifié.
- Identification d'une adresse mémoire, de sa région, de ses permissions, de
  son type et du module auquel elle appartient.
- Analyse d'une image PE chargée en mémoire, de ses sections et de ses exports,
  avec recherche partielle d'une fonction.
- Énumération des DLL/modules chargés via le PEB, pour le processus courant ou
  un PID spécifié.
- Liste des processus actifs avec leurs PID et leurs TID.
- Liste des threads d'un processus avec leur priorité et leur accessibilité.
- Lecture du contexte et des registres d'un thread.
- Énumération des handles système, avec filtrage optionnel par PID.
- Analyse des arguments avec prise en charge des formes courante et distante
  pour les commandes mémoire, PE et PEB, ainsi que des messages d'erreur
  dédiés pour les PID, TID et adresses invalides.
- Demande automatique d'élévation administrateur via l'UAC.

## Prérequis

- Windows 10 ou version ultérieure recommandé.
- [MSYS2](https://www.msys2.org/) avec l'environnement UCRT64.
- `g++` et `gdb` installés dans `C:\msys64\ucrt64\bin`, ou un chemin adapté
  dans les fichiers `.vscode`.
- Un terminal capable d'afficher l'UTF-8 et les séquences ANSI.
- Des droits administrateur pour certaines opérations sur les processus, les
  threads et les handles.

Le projet utilise l'API Win32, Tool Help, PSAPI, IP Helper et plusieurs API
internes de Windows (`NtQuerySystemInformation`, `NtQueryObject` et le PEB).

## Compilation

Depuis la racine du projet, avec MSYS2 UCRT64 :

```powershell
C:\msys64\ucrt64\bin\g++.exe -std=c++17 -finput-charset=UTF-8 -fexec-charset=UTF-8 RedFlowers.cpp -o RedFlowers.exe -liphlpapi
```

La bibliothèque `iphlpapi` est nécessaire à la récupération des informations
réseau.

### Avec Visual Studio Code

- La tâche **Build C++ file** de `.vscode/tasks.json` compile le fichier ouvert
  en C++17 avec `g++`.
- La configuration **C++: gdb** de `.vscode/launch.json` compile le fichier
  avant de le lancer avec GDB.
- `.vscode/run-cpp.js` compile puis exécute un fichier C++ passé en argument :

  ```powershell
  node .vscode\run-cpp.js RedFlowers.cpp
  ```

Les chemins du compilateur et de GDB sont configurés pour une installation
standard dans `C:\msys64`.

## Lancement

```powershell
.\RedFlowers.exe
```

Le programme demande un nom d'utilisateur puis ouvre l'invite :
`RedFlower's@<nom>`. L'élévation administrateur est demandée automatiquement
si l'application n'est pas déjà lancée avec ces droits.

## Commandes

Les commandes ne tiennent pas compte de la casse. Les paramètres sont séparés
par des espaces. Les PID et TID sont numériques ; les adresses sont
hexadécimales, avec ou sans préfixe `0x`.

| Commande | Description |
| --- | --- |
| `get-help` | Affiche la liste des commandes. |
| `clear` | Efface le terminal. |
| `exit` | Quitte le programme. |
| `get-system-info` | Affiche les informations système. |
| `get-adapter-info` | Affiche les adaptateurs réseau. |
| `adresse` | Affiche les régions mémoire du processus courant. |
| `peek <adresse>` | Identifie une adresse dans le processus courant. |
| `peek <PID> <adresse>` | Identifie une adresse dans un autre processus accessible. |
| `peek-all` | Liste les régions mémoire du processus courant. |
| `peek-all <PID>` | Liste les régions mémoire d'un autre processus accessible. |
| `pe <adresse> [fonction]` | Analyse une image PE du processus courant et peut rechercher un export. |
| `pe <PID> <adresse> [fonction]` | Analyse une image PE dans un autre processus accessible et recherche éventuellement un export. |
| `peb` | Liste les modules chargés du processus courant via le PEB. |
| `peb <PID>` | Liste les modules chargés via le PEB pour un PID. |
| `ps` | Liste les processus actifs et leurs TID. |
| `threads` | Liste les threads du processus courant. |
| `threads <PID>` | Liste les threads d'un processus et indique si chaque thread est accessible. |
| `thread-ctx <TID>` | Lit le contexte et les registres d'un thread. |
| `handles` | Énumère les handles système. |
| `handles <PID>` | Filtre l'énumération des handles sur un PID. |

Les commandes d'inspection d'un autre processus nécessitent les droits
appropriés. `peek-all` limite l'affichage à 100 régions lorsqu'un PID distant
est utilisé afin d'éviter de saturer le terminal. Pour `pe`, la forme
`pe <adresse> [fonction]` reste disponible pour le processus courant ; la forme
`pe <PID> <adresse> [fonction]` sélectionne explicitement un processus distant.
De même, `peb` sans PID cible le processus courant et `peb <PID>` cible le
processus indiqué.

## Structure du projet

| Élément | Rôle |
| --- | --- |
| `RedFlowers.cpp` | Point d'entrée et initialisation de la console. |
| `PONT.hpp` | Liaison entre le point d'entrée et le menu. |
| `Menu.hpp` | Boucle interactive et routage des commandes. |
| `System.hpp`, `CPU.hpp`, `RAM.hpp` | Informations système et mémoire physique. |
| `Network.hpp` | Énumération des adaptateurs réseau. |
| `adresse.hpp`, `peek.hpp`, `peek_identify.hpp` | Cartographie et inspection de la mémoire virtuelle. |
| `RedFlowerPe.hpp` | Analyse des images PE et de leurs exports. |
| `RedFlowerPEB.hpp` | Énumération des modules via le PEB. |
| `redflowersprocesses.hpp` | Énumération des processus et de leurs threads. |
| `redflowers_threads.hpp`, `threads-ctx.hpp` | Énumération des threads et lecture de leur contexte. |
| `RedFlowerHandles.hpp` | Énumération des handles et activation de `SeDebugPrivilege`. |
| `utils.hpp` | Fonctions utilitaires de l'application. |
| `.vscode/` | Compilation, lancement GDB et exécution via Code Runner. |

## Limitations et précautions

- Le projet est spécifique à Windows.
- Certaines opérations échouent pour les processus protégés, les threads
  terminés ou les ressources auxquelles l'utilisateur n'a pas accès.
- Les inspections distantes ouvrent le processus avec des droits limités
  (`PROCESS_QUERY_INFORMATION` et, selon la commande, `PROCESS_VM_READ`) :
  l'accès peut donc être refusé.
- La commande `thread-ctx` suspend brièvement le thread ciblé et annule la
  lecture après trois secondes en cas de délai dépassé. Utilisez-la avec
  prudence.
- Le PEB, `NtQuerySystemInformation` et `NtQueryObject` sont des interfaces
  internes ou non documentées ; leur comportement peut varier selon la version
  de Windows.
- L'affichage des régions distantes est limité à 100 entrées. L'affichage des
  handles et des exports peut également être limité dans les implémentations
  correspondantes.
- Il s'agit d'un outil d'observation et de diagnostic en développement, pas
  d'un débogueur complet ni d'un outil de supervision.

## Licence

Aucune licence n'est actuellement indiquée dans le dépôt.
