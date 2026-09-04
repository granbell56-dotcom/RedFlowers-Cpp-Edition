# RedFlower's - C++ Edition

Outil Windows en ligne de commande pour observer le système courant et explorer
certaines structures bas niveau d'un processus. Le programme affiche les
informations directement dans le terminal et fonctionne sous la forme d'un
shell interactif.

## Fonctionnalités

- Informations système : nom du PC, processeur, nombre de threads CPU,
  architecture, mémoire physique et espace d'adressage.
- Informations des adaptateurs réseau : nom, description, IPv4, masque,
  passerelle et adresse MAC.
- Cartographie de la mémoire virtuelle avec `VirtualQuery`.
- Identification d'une adresse mémoire et du module auquel elle appartient.
- Analyse d'une image PE chargée en mémoire, de ses sections et de sa table
  d'exports, avec recherche partielle d'une fonction.
- Énumération des DLL/modules chargés via le PEB.
- Liste des processus actifs et de leurs threads.
- Lecture du contexte et de registres d'un thread.
- Énumération de handles système (fichiers, sockets AFD, mutants et sections).
- Relance automatique avec les privilèges administrateur lorsque l'UAC le
  permet.

## Prérequis

- Windows (API Win32, Tool Help, PSAPI et API natives utilisées par le projet).
- [MSYS2](https://www.msys2.org/) avec l'environnement UCRT64.
- `g++` et `gdb` accessibles dans `C:\msys64\ucrt64\bin`.
- Un terminal capable d'afficher l'UTF-8 et les séquences ANSI.

Le chemin du compilateur configuré dans le projet est :
`C:\msys64\ucrt64\bin\g++.exe`. Si MSYS2 est installé ailleurs, adaptez les
commandes ou les fichiers `.vscode`.

## Compilation

Depuis la racine du projet :

```powershell
C:\msys64\ucrt64\bin\g++.exe -std=c++17 -finput-charset=UTF-8 -fexec-charset=UTF-8 RedFlowers.cpp -o RedFlowers.exe -liphlpapi
```

La bibliothèque `iphlpapi` est nécessaire à la récupération des adaptateurs
réseau.

### Avec Visual Studio Code

La tâche **Build C++ file** (`.vscode/tasks.json`) compile le fichier ouvert
avec `g++` en C++17. La configuration **C++: gdb** (`.vscode/launch.json`)
compile avant de lancer le programme. Le script `.vscode/run-cpp.js` permet
également de compiler puis d'exécuter un fichier C++ passé en argument.

## Lancement

```powershell
.\RedFlowers.exe
```

L'application demande d'abord un nom d'utilisateur, puis affiche une invite
`RedFlower's@<nom>`. Aucun argument de ligne de commande n'est documenté :
l'interface utilisateur est le shell interactif.

## Commandes

Les commandes ne tiennent pas compte de la casse. Utilisez `get-help` dans le
programme pour réafficher cette liste.

| Commande | Description |
| --- | --- |
| `get-help` | Affiche l'aide des commandes. |
| `clear` | Efface le terminal. |
| `exit` | Quitte le programme. |
| `get-system-info` | Affiche le nom du PC, les informations CPU, la RAM et l'espace d'adressage. |
| `get-adapter-info` | Liste les informations des adaptateurs réseau. |
| `adresse` | Liste les régions mémoire virtuelles réservées ou engagées. |
| `peek <adresse>` | Identifie une adresse hexadécimale, sa région, ses permissions et son module éventuel. Exemple : `peek 0x00007FF6...` |
| `peek-all` | Liste les régions mémoire réservées ou engagées avec leur état, type et protection. |
| `pe <adresse> [fonction]` | Analyse l'image PE à l'adresse indiquée. Le nom de fonction optionnel effectue une recherche partielle dans les exports. |
| `peb` | Liste les modules chargés en parcourant le PEB. |
| `ps` | Liste les processus actifs avec leur PID et les TID associés. |
| `threads [PID]` | Liste les threads du PID indiqué ; sans PID, utilise le processus courant. |
| `thread-ctx <TID>` | Suspend brièvement le thread, lit son contexte (registres) puis le reprend. |
| `handles [PID]` | Énumère les handles système, ou filtre sur un PID. |

Les paramètres se saisissent après la commande, séparés par des espaces. Les
adresses sont attendues au format hexadécimal ; les PID et TID doivent être
numériques.

## Structure du projet

| Élément | Rôle |
| --- | --- |
| `RedFlowers.cpp` | Point d'entrée et initialisation de la console. |
| `Menu.hpp` | Boucle interactive, lecture des commandes et routage vers les fonctionnalités. |
| `System.hpp`, `CPU.hpp`, `RAM.hpp` | Informations système et mémoire physique. |
| `Network.hpp` | Énumération des adaptateurs réseau. |
| `adresse.hpp`, `peek.hpp`, `peek_identify.hpp` | Cartographie et inspection de la mémoire virtuelle. |
| `RedFlowerPe.hpp` | Analyse des images PE et des exports. |
| `RedFlowerPEB.hpp` | Énumération des modules via le PEB. |
| `redflowersprocesses.hpp`, `redflowers_threads.hpp`, `threads-ctx.hpp` | Processus, threads et contexte des threads. |
| `RedFlowerHandles.hpp` | Énumération des handles et activation de `SeDebugPrivilege`. |
| `PONT.hpp`, `utils.hpp` | Liaison vers le menu et configuration de la console. |
| `.vscode/` | Tâche de compilation, lancement GDB et configuration Code Runner. |

## Limitations et précautions

- Le projet est spécifique à Windows et ne fournit pas de portabilité Linux ou
  macOS.
- Plusieurs fonctionnalités nécessitent une élévation administrateur et peuvent
  échouer pour les processus protégés ou les threads auxquels l'accès est
  refusé.
- Les commandes mémoire inspectent l'espace du processus RedFlower courant ;
  elles ne lisent ni ne modifient directement la mémoire d'un autre processus.
- Le PEB, `NtQuerySystemInformation` et `NtQueryObject` sont des interfaces
  internes/non documentées : leur disposition ou leur comportement peut varier
  selon la version de Windows.
- L'affichage des handles et des exports est volontairement limité pour éviter
  de saturer le terminal (40 handles dans le mode global et 30 résultats pour
  une recherche d'export).
- `thread-ctx` suspend le thread ciblé pendant la lecture ; utilisez cette
  commande avec prudence.
- Il s'agit d'un outil d'observation et de diagnostic en cours de développement,
  pas d'un produit de supervision ou d'un débogueur complet.

## Licence

Aucune licence n'est actuellement indiquée dans le dépôt.
