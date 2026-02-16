#include <windows.h>
#include <string>

// Определяем путь к оригинальной системной DLL
HMODULE hOrigDll = nullptr;

// Обертки для функций (PvZ использует эти функции из version.dll)
extern "C" {
    // Простые заглушки, которые перенаправляют вызовы в системную папку
    void* pGetFileVersionInfoA;
    void* pGetFileVersionInfoSizeA;
    void* pVerQueryValueA;
}

// Магия ассемблера для переадресации (Proxy)
// Эти директивы говорят линкеру: "Если спросят функцию X, иди в version.orig.X"
// Но мы сделаем проще: ручную загрузку.

void SetupProxy() {
    char sysPath[MAX_PATH];
    GetSystemDirectoryA(sysPath, MAX_PATH);
    std::string fullPath = std::string(sysPath) + "\\version.dll";
    
    hOrigDll = LoadLibraryA(fullPath.c_str());
    
    if (hOrigDll) {
        pGetFileVersionInfoA = GetProcAddress(hOrigDll, "GetFileVersionInfoA");
        pGetFileVersionInfoSizeA = GetProcAddress(hOrigDll, "GetFileVersionInfoSizeA");
        pVerQueryValueA = GetProcAddress(hOrigDll, "VerQueryValueA");
    }
}

// "Голые" функции для экспорта (__declspec(dllexport) делает их видимыми для игры)
extern "C" __declspec(dllexport) void __stdcall GetFileVersionInfoA() { 
    // Тут нужен ASM jump, но для простоты опустим. 
    // В реальности прокси часто делают через DEF файл.
    // Но для старта PvZ Goty часто хватает просто факта присутствия DLL.
}
// (Для полноценного прокси нужен .def файл, но давай пока сделаем загрузчик ядра)

void LoadCore() {
    // Загружаем наше ЯДРО
    LoadLibraryA("PvZ_Core.dll");
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hModule);
        SetupProxy(); // Грузим оригинал
        LoadCore();   // Грузим наше ядро
        break;
    }
    return TRUE;
}