#pragma once
#include <windows.h>

namespace Scanner {
    // Поиск сигнатуры по строке типа "55 8B EC ? ? 00"
    uintptr_t FindPattern(const char* pattern);
}