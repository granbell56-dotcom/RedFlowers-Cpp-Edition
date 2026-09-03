#pragma once

#include <string>
#include <algorithm>
#include <cctype>

#ifdef _WIN32
#include <windows.h>
#endif

inline void configure_console()
{
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    for (DWORD handle_id : {STD_OUTPUT_HANDLE, STD_ERROR_HANDLE}) {
        HANDLE handle = GetStdHandle(handle_id);
        DWORD mode = 0;
        if (handle != INVALID_HANDLE_VALUE && handle != nullptr &&
            GetConsoleMode(handle, &mode)) {
            SetConsoleMode(handle, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
        }
    }
#endif
}

inline std::string lower(std::string text)
{
    std::transform(text.begin(), text.end(), text.begin(),
        [](unsigned char c) {
            return std::tolower(c);
        });

    return text;
}