#include <windows.h>
void LoadMods(); // Forward declaration

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
        CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)LoadMods, nullptr, 0, nullptr);
    }
    return TRUE;
}