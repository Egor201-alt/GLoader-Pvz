#include "Scanner.h"
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>

uintptr_t Scanner::FindPattern(const char* pattern) {
    uintptr_t moduleBase = (uintptr_t)GetModuleHandleA(NULL);
    PIMAGE_DOS_HEADER dosHeader = (PIMAGE_DOS_HEADER)moduleBase;
    PIMAGE_NT_HEADERS ntHeaders = (PIMAGE_NT_HEADERS)(moduleBase + dosHeader->e_lfanew);
    
    // Ищем во всем сегменте кода (.text)
    DWORD size = ntHeaders->OptionalHeader.SizeOfImage;
    
    std::vector<int> patternBytes;
    std::stringstream ss(pattern);
    std::string temp;

    while (ss >> temp) {
        if (temp == "?" || temp == "??") patternBytes.push_back(-1);
        else patternBytes.push_back(std::stoi(temp, nullptr, 16));
    }

    uint8_t* scanBytes = (uint8_t*)moduleBase;
    for (size_t i = 0; i < size - patternBytes.size(); i++) {
        bool found = true;
        for (size_t j = 0; j < patternBytes.size(); j++) {
            if (patternBytes[j] != -1 && scanBytes[i + j] != (uint8_t)patternBytes[j]) {
                found = false;
                break;
            }
        }
        if (found) return (uintptr_t)(scanBytes + i);
    }
    return 0;
}