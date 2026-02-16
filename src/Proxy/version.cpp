#include <windows.h>

// --- МАГИЯ ПЕРЕНАПРАВЛЕНИЯ (PROXY) ---
// Эти строчки говорят компилятору: "Не делай функции сам. Просто скажи игре, 
// что они лежат в C:\Windows\System32\version.dll"
#pragma comment(linker, "/export:GetFileVersionInfoA=C:\\Windows\\System32\\version.GetFileVersionInfoA")
#pragma comment(linker, "/export:GetFileVersionInfoSizeA=C:\\Windows\\System32\\version.GetFileVersionInfoSizeA")
#pragma comment(linker, "/export:VerQueryValueA=C:\\Windows\\System32\\version.VerQueryValueA")

// Функция загрузки нашего ядра
void LoadCore() {
    // Пытаемся загрузить наше ядро
    HMODULE hCore = LoadLibraryA("PvZ_Core.dll");
    if (!hCore) {
        // Если ядра нет - молча ничего не делаем, игра просто запустится как обычно (ванильная)
        // Либо можно вывести MessageBox для отладки, если хочешь.
    }
}

// Точка входа в DLL
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
        // ТЕСТОВОЕ ОКНО
        MessageBoxA(NULL, "PvZ Mod Loader: Injected!", "It Works!", MB_OK);

        DisableThreadLibraryCalls(hModule);
        LoadCore();
    }
    return TRUE;
}