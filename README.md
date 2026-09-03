# RedFlowers C++ Edition

RedFlowers C++ Edition est un terminal interactif Windows en C++17. Il regroupe des outils d'information système, de diagnostic mémoire et d'observation des processus et threads.

> Projet expérimental destiné au diagnostic sur une machine de test ou sur des processus que vous êtes autorisé à analyser. Certaines opérations, comme la suspension temporaire d'un thread, peuvent perturber un programme.

> **Statut : version de test.** Le projet n'est pas encore terminé et continuera d'être ajusté, amélioré et complété au fil du développement.

## Fonctionnalités

- Affichage des informations CPU, RAM et espace d'adressage Windows.
- Affichage des adaptateurs réseau, adresses IPv4, masques, passerelles et adresses MAC.
- Énumération des processus actifs avec leur PID et leurs TID.
- Énumération des threads d'un processus avec leur priorité de base.
- Lecture du contexte d'un thread : RIP, RSP, RBP et registres généraux.
- Inspection de l'état, du type et des permissions des régions mémoire du processus courant.
- Identification d'une adresse mémoire et du module auquel elle appartient.
- Élévation UAC automatique lorsque le programme n'est pas lancé en administrateur.

## Prérequis

- Windows.
- MinGW-w64 avec `g++`.
- C++17 ou version plus récente.
- Visual Studio Code est recommandé, mais pas obligatoire.

Le projet utilise principalement les API Windows suivantes :

- `Windows.h`
- `TlHelp32.h`
- `Psapi.h`
- `Iphlpapi.h`

## Compilation

Depuis un terminal configuré avec MinGW-w64 :

```powershell
g++ -std=c++17 -finput-charset=UTF-8 -fexec-charset=UTF-8 RedFlowers.cpp -o RedFlowers.exe -liphlpapi
```

Dans Visual Studio Code, ouvrir `RedFlowers.cpp`, puis lancer la tâche de compilation avec `Ctrl+Shift+B`.

## Lancement

```powershell
.\RedFlowers.exe
```

Le programme peut demander une élévation UAC afin d'obtenir les droits nécessaires à certaines opérations de diagnostic.

## Commandes disponibles

### Commandes générales

| Commande | Description |
|---|---|
| `get-help` | Affiche la liste des commandes. |
| `clear` | Nettoie la console. |
| `exit` | Ferme le programme. |

### Informations système

| Commande | Description |
|---|---|
| `get-system-info` | Affiche le nom de l'ordinateur, le CPU, la RAM et les paramètres d'adressage. |
| `get-adapter-info` | Affiche les informations des adaptateurs réseau. |

### Mémoire

| Commande | Description |
|---|---|
| `adresse` | Liste les régions mémoire engagées ou réservées du processus courant. |
| `peek-all` | Affiche l'état, le type et les permissions des régions mémoire du processus courant. |
| `peek <adresse>` | Identifie une adresse hexadécimale, par exemple `peek 0x7FF600000000`. |

### Processus et threads

| Commande | Description |
|---|---|
| `ps` | Liste les processus actifs avec leur PID, leurs TID et leur nom. |
| `threads <PID>` | Liste les threads appartenant au PID indiqué. |
| `thread-ctx <TID>` | Suspend brièvement le thread, lit son contexte puis le reprend. |

La commande `thread-ctx` annule la lecture après trois secondes si aucune réponse n'est obtenue.

## Exemple

```text
$ ps
PID         | TID(s)                         | Nom du Processus
23984       | 18688, 17312, 11276, 13620    | RedFlowers.exe

$ thread-ctx 18688
```

`thread-ctx` attend un **TID**, pas un PID.

## Structure principale

| Fichier | Rôle |
|---|---|
| `RedFlowers.cpp` | Point d'entrée du programme. |
| `PONT.hpp` | Liaison entre le point d'entrée et le menu. |
| `Menu.hpp` | Menu interactif et routage des commandes. |
| `System.hpp` | Informations système Windows. |
| `CPU.hpp`, `RAM.hpp` | Informations CPU et RAM. |
| `Network.hpp` | Informations des adaptateurs réseau. |
| `redflowersprocesses.hpp` | Énumération des processus et de leurs TID. |
| `redflowers_threads.hpp` | Énumération des threads d'un PID. |
| `threads-ctx.hpp` | Lecture des registres d'un thread. |
| `adresse.hpp` | Cartographie des régions mémoire. |
| `peek.hpp` | Analyse détaillée des régions mémoire. |
| `peek_identify.hpp` | Identification d'une adresse et de son module. |
| `utils.hpp` | Configuration UTF-8 de la console et fonctions utilitaires. |

## Limitations et erreurs possibles

- Windows peut refuser l'accès aux processus protégés, même avec une élévation administrateur.
- Un TID peut disparaître entre la commande `ps` et `thread-ctx`.
- `RIP` est une adresse d'instruction ; les registres seuls ne constituent pas un désassemblage.
- Les résultats mémoire concernent le processus courant, pas un processus arbitraire.
- L'architecture de compilation doit correspondre à l'environnement utilisé, notamment pour l'affichage des registres 32 bits ou 64 bits.

## Licence

Aucune licence open source n'est actuellement indiquée dans le projet. Ajoutez une licence avant toute redistribution publique.
