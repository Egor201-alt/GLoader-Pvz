#include "ModLoader.h"
#include "Scanner.h"
#include "PvZ_API.h"
#include <windows.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <ctime>
#include <vector>
#include <iomanip>
#include <map>

// JSON library
#include <nlohmann/json.hpp>
using json = nlohmann::json;
namespace fs = std::filesystem;

// Глобальные переменные
std::ofstream logFile;
PvZ_Interface g_PvZ_API; // Наше созданное API для модов
std::map<std::string, uintptr_t> g_AddressDB; // База найденных адресов

struct Mod {
    std::string name;
    std::string version;
};
std::vector<Mod> loadedMods;

// --- СИСТЕМА ЛОГИРОВАНИЯ ---

void Log(const std::string& msg) {
    // Получаем текущее время
    std::time_t now = std::time(nullptr);
    std::tm localTime;
    localtime_s(&localTime, &now);

    // Форматируем время [HH:MM:SS]
    char timeBuffer[32];
    std::strftime(timeBuffer, sizeof(timeBuffer), "[%H:%M:%S] ", &localTime);

    // 1. Вывод в консоль
    std::cout << timeBuffer << msg << std::endl;

    // 2. Вывод в файл (если открыт)
    if (logFile.is_open()) {
        logFile << timeBuffer << msg << std::endl;
        logFile.flush(); 
    }
}

void InitLogging() {
    if (!fs::exists("./logs")) {
        fs::create_directory("./logs");
    }

    logFile.open("./logs/latest.log", std::ios::out | std::ios::trunc);

    AllocConsole();
    FILE* fDummy;
    freopen_s(&fDummy, "CONOUT$", "w", stdout);
    freopen_s(&fDummy, "CONOUT$", "w", stderr);

    std::cout << "==========================================" << std::endl;
    std::cout << "      GLoader Core Initialized            " << std::endl;
    std::cout << "==========================================" << std::endl;
    Log("Logger initialized successfully.");
}

// --- УТИЛИТЫ ДЛЯ ПРАВКИ ПАМЯТИ (Для автономности модов) ---

void API_WriteMemory(uintptr_t address, void* value, size_t size) {
    if (!address) return;
    DWORD oldProtect;
    VirtualProtect((LPVOID)address, size, PAGE_EXECUTE_READWRITE, &oldProtect);
    memcpy((LPVOID)address, value, size);
    VirtualProtect((LPVOID)address, size, oldProtect, &oldProtect);
}

void API_ReadMemory(uintptr_t address, void* buffer, size_t size) {
    if (!address || !buffer) return;
    memcpy(buffer, (LPVOID)address, size);
}

uintptr_t API_GetAddress(const char* name) {
    if (g_AddressDB.count(name)) {
        return g_AddressDB[name];
    }
    return 0;
}

// --- ИНИЦИАЛИЗАЦИЯ API (Поиск функций игры) ---

void InitAPI() {
    Log("Initializing API and Scanning Signatures...");

    // 1. Настраиваем системную часть API
    g_PvZ_API.Log = [](const char* m) {
        Log(std::string("[Mod] ") + m);
    };
    g_PvZ_API.WriteMemory = API_WriteMemory;
    g_PvZ_API.ReadMemory = API_ReadMemory;
    g_PvZ_API.GetAddress = API_GetAddress;

    // 2. Загружаем внешние сигнатуры из файла signatures.json (Автономность!)
    if (fs::exists("signatures.json")) {
        Log("Loading signatures from signatures.json...");
        try {
            std::ifstream f("signatures.json");
            json data = json::parse(f);

            for (auto& [name, pattern] : data.items()) {
                std::string pStr = pattern.get<std::string>();
                uintptr_t addr = Scanner::FindPattern(pStr.c_str());
                if (addr) {
                    g_AddressDB[name] = addr;
                    Log("  -> Signature [" + name + "] found at 0x" + std::to_string(addr));
                } else {
                    Log("  -> Signature [" + name + "] NOT FOUND!");
                }
            }
        } catch (const std::exception& e) {
            Log("  -> Critical error parsing signatures.json: " + std::string(e.what()));
        }
    } else {
        Log("signatures.json not found. Using built-in fallback signatures...");
        
        // Встроенные сигнатуры на случай отсутствия файла
        uintptr_t addrAddSun = Scanner::FindPattern("55 8B EC 81 EC ? ? ? ? 53 56 57 8B F1 8B 46 04");
        if (addrAddSun) g_AddressDB["AddSun"] = addrAddSun;

        uintptr_t addrGiveMoney = Scanner::FindPattern("55 8B EC 53 8B 5D 08 56 8B F1 85 DB 7E ? 8B 86 ? ? ? ?");
        if (addrGiveMoney) g_AddressDB["GiveMoney"] = addrGiveMoney;
    }

    // 3. Привязываем найденные адреса к фиксированным функциям API
    g_PvZ_API.AddSun = (void(*)(void*, int))g_AddressDB["AddSun"];
    g_PvZ_API.GiveMoney = (void(*)(void*, int))g_AddressDB["GiveMoney"];

    Log("API setup complete.");
}

// --- ЗАГРУЗЧИК МОДОВ ---

void LoadMods() {
    // Шаг 1: Подготавливаем API перед загрузкой модов
    InitAPI();

    Log("Scanning for mods in ./mods/ directory...");

    std::string modsPath = "./mods";
    if (!fs::exists(modsPath)) {
        Log("Warning: 'mods' directory not found. Creating one.");
        fs::create_directory(modsPath);
        return;
    }

    int successCount = 0;

    for (const auto& entry : fs::directory_iterator(modsPath)) {
        if (entry.is_directory()) {
            std::string modDirName = entry.path().filename().string();
            fs::path jsonPath = entry.path() / "mod.json";

            // Безопасный блок try-catch, чтобы один плохой мод не крашнул всю игру
            try {
                if (!fs::exists(jsonPath)) {
                    continue;
                }

                std::ifstream f(jsonPath);
                json data = json::parse(f);

                std::string modName = data.value("name", "Unknown Mod");
                std::string dllName = data.value("dll", "");

                Log("Found mod: " + modName);

                if (dllName.empty()) {
                    Log("  -> Skipped: No DLL specified in mod.json");
                    continue;
                }

                fs::path dllFullPath = entry.path() / dllName;
                
                if (!fs::exists(dllFullPath)) {
                     Log("  -> Error: DLL file missing at " + dllFullPath.string());
                     continue;
                }

                // Загружаем DLL мода
                HMODULE hMod = LoadLibraryA(dllFullPath.string().c_str());
                
                if (hMod) {
                    // Ищем в моде функцию InitializeMod
                    ModInit_t initFunc = (ModInit_t)GetProcAddress(hMod, "InitializeMod");
                    
                    if (initFunc) {
                        // Передаем указатель на наше API в мод
                        initFunc(&g_PvZ_API);
                        Log("  -> Loaded and initialized with API!");
                    } else {
                        Log("  -> Loaded, but no 'InitializeMod' function found (Generic DLL).");
                    }

                    loadedMods.push_back({modName, data.value("version", "1.0")});
                    successCount++;
                } else {
                    DWORD err = GetLastError();
                    Log("  -> CRITICAL: Failed to load DLL. Error code: " + std::to_string(err));
                }

            } catch (const json::parse_error& e) {
                Log("  -> JSON Error in " + modDirName + ": " + e.what());
            } catch (const std::exception& e) {
                Log("  -> Unexpected Error in " + modDirName + ": " + e.what());
            } catch (...) {
                Log("  -> Unknown crash happened while loading " + modDirName);
            }
        }
    }

    Log("------------------------------------------");
    Log("Mod loading finished.");
    Log("Total active mods: " + std::to_string(successCount));
    Log("------------------------------------------");
}