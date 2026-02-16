#include <windows.h>
#include "ModLoader.h"

// Этот поток запускается отдельно, чтобы не тормозить запуск самой игры
DWORD WINAPI LoaderThread(LPVOID lpParam) {
    // Инициализируем консоль и логи
    InitLogging();
    
    // Ждем секунду, чтобы окно игры успело появиться (опционально)
    Sleep(1000);
    
    // Запускаем поиск модов
    LoadMods();
    
    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
        // Запускаем наш код в отдельном потоке
        CreateThread(nullptr, 0, LoaderThread, nullptr, 0, nullptr);
    }
    return TRUE;
}