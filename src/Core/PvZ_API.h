#pragma once
#include <windows.h>

// Структура со всеми функциями игры, которые мы нашли
struct PvZ_Interface {
    // Пример функций игры (адреса заполним при старте)
    void (__userpurge* AddSun)(void* board, int amount); 
    void* (__cdecl* SpawnZombie)(void* board, int type);
    
    // Наши удобные обертки
    void (*Log)(const char* text);
};

// Тип функции, которую должен иметь каждый мод
typedef void (*ModInit_t)(PvZ_Interface* api);