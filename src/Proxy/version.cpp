#include <windows.h>

// --- ПЕРЕАДРЕСАЦИЯ ДЛЯ WINMM.DLL ---
// Мы говорим игре: "Функции лежат в системной C:\Windows\System32\winmm.dll"

#pragma comment(linker, "/export:timeBeginPeriod=C:\\Windows\\System32\\winmm.timeBeginPeriod")
#pragma comment(linker, "/export:timeEndPeriod=C:\\Windows\\System32\\winmm.timeEndPeriod")
#pragma comment(linker, "/export:timeGetTime=C:\\Windows\\System32\\winmm.timeGetTime")
#pragma comment(linker, "/export:PlaySoundA=C:\\Windows\\System32\\winmm.PlaySoundA")
#pragma comment(linker, "/export:waveOutOpen=C:\\Windows\\System32\\winmm.waveOutOpen")

// Функция загрузки ядра
void LoadCore() {
    // Пытаемся загрузить ядро
    HMODULE hCore = LoadLibraryA("PvZ_Core.dll");
    
    if (hCore) {
        MessageBoxA(NULL, "Core Loaded Successfully!", "PvZ Mod Loader", MB_OK);
    } else {
        // Если ядра нет или оно сломано - покажем ошибку с кодом
        DWORD err = GetLastError();
        char errBuff[256];
        wsprintfA(errBuff, "Failed to load PvZ_Core.dll\nError Code: %lu", err);
        MessageBoxA(NULL, errBuff, "Mod Loader Error", MB_ICONERROR);
    }
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hModule);
        
        // Сразу показываем окно, что мы живы
        // MessageBoxA(NULL, "Winmm Proxy Attached!", "Debug", MB_OK);
        
        LoadCore();
    }
    return TRUE;
}