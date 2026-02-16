#pragma once
#include <windows.h>
#include <vector>
#include <string>

namespace Scanner {
    // Ищет последовательность байт в памяти игры
    uintptr_t FindPattern(const char* pattern, const char* mask);
    
    // Автоматический поиск по текстовой сигнатуре (например, "55 8B EC 6A FF")
    uintptr_t Scan(std::string pattern);
}