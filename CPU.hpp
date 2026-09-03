#include <windows.h>
#include <iostream>

void cpu_info() {
    HKEY hKey;

    if (RegOpenKeyExA(
        HKEY_LOCAL_MACHINE,
        "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0",
        0,
        KEY_READ,
        &hKey
    ) == ERROR_SUCCESS) {

        char cpu[256];
        DWORD size = sizeof(cpu);

        if (RegQueryValueExA(
            hKey,
            "ProcessorNameString",
            nullptr,
            nullptr,
            reinterpret_cast<BYTE*>(cpu),
            &size
        ) == ERROR_SUCCESS) {

            std::cout << "│ CPU : " << cpu << std::endl;
        }

        RegCloseKey(hKey);
    }
}