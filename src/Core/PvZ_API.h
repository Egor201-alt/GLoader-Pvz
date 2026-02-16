#pragma once

// Структура интерфейса API
struct PvZ_Interface {
    // Системные функции ядра
    void (*Log)(const char* msg);
    
    // Функции игры
    // Используем обычные указатели. Для PvZ этого достаточно.
    void (*AddSun)(void* board, int amount);
    void (*GiveMoney)(void* player, int amount);
};

// Тип функции инициализации для модов
typedef void (*ModInit_t)(PvZ_Interface* api);