#pragma once
#include <windows.h>
#include <string>

struct PvZ_Interface {
    // 1. Логирование
    void (*Log)(const char* msg);

    // 2. Memory API (Дает мододелам полную свободу)
    void (*WriteMemory)(uintptr_t address, void* value, size_t size);
    void (*ReadMemory)(uintptr_t address, void* buffer, size_t size);
    uintptr_t (*GetAddress)(const char* name); // Поиск адреса по имени из базы

    // 3. Базовые функции (будут заполнены ядром)
    void (*AddSun)(void* board, int amount);
    void (*GiveMoney)(void* player, int amount);
};

typedef void (*ModInit_t)(PvZ_Interface* api);