#pragma once

#include <windows.h>
#include <iphlpapi.h>
#include <iostream>
#include <cstdlib>
#include <cstdio>

void adapters_info() {

    // ─────────────────────────────────────────────
    // 1. Récupérer la taille nécessaire
    // ─────────────────────────────────────────────

    ULONG bufferSize = 0;

    if (GetAdaptersInfo(nullptr, &bufferSize) != ERROR_BUFFER_OVERFLOW) {
        std::cerr << "Erreur : impossible de récupérer la taille du buffer."
                  << std::endl;
        return;
    }


    // ─────────────────────────────────────────────
    // 2. Allouer la mémoire
    // ─────────────────────────────────────────────

    PIP_ADAPTER_INFO adapterInfo =
        static_cast<PIP_ADAPTER_INFO>(malloc(bufferSize));

    if (adapterInfo == nullptr) {
        std::cerr << "Erreur : allocation mémoire impossible."
                  << std::endl;
        return;
    }


    // ─────────────────────────────────────────────
    // 3. Récupérer les informations
    // ─────────────────────────────────────────────

    DWORD result = GetAdaptersInfo(adapterInfo, &bufferSize);

    if (result != NO_ERROR) {

        std::cerr << "Erreur lors de la récupération des adaptateurs."
                  << std::endl;

        free(adapterInfo);
        return;
    }


    // ─────────────────────────────────────────────
    // 4. Parcourir les adaptateurs
    // ─────────────────────────────────────────────

    PIP_ADAPTER_INFO currentAdapter = adapterInfo;

    while (currentAdapter != nullptr) {

        std::cout << "┌─/[ ADAPTER ]/─>" << std::endl;
        std::cout << "│" << std::endl;

        // Nom
        std::cout << "│ Name        : "
                  << currentAdapter->AdapterName
                  << std::endl;

        // Description
        std::cout << "│ Description : "
                  << currentAdapter->Description
                  << std::endl;

        // IP
        std::cout << "│ IPv4        : "
                  << currentAdapter->IpAddressList.IpAddress.String
                  << std::endl;

        // Masque
        std::cout << "│ Subnet mask : "
                  << currentAdapter->IpAddressList.IpMask.String
                  << std::endl;

        // Gateway
        std::cout << "│ Gateway     : "
                  << currentAdapter->GatewayList.IpAddress.String
                  << std::endl;


        // MAC
        std::cout << "│ MAC         : ";

        for (UINT i = 0; i < currentAdapter->AddressLength; i++) {

            printf("%02X", currentAdapter->Address[i]);

            if (i + 1 < currentAdapter->AddressLength) {
                printf("-");
            }
        }

        std::cout << std::endl;

        std::cout << "│" << std::endl;
        std::cout << "└──────────────────────────────────────/─>"
                  << std::endl;

        std::cout << std::endl;


        // Passer à l'adaptateur suivant
        currentAdapter = currentAdapter->Next;
    }


    // ─────────────────────────────────────────────
    // 5. Libérer la mémoire
    // ─────────────────────────────────────────────

    free(adapterInfo);
}