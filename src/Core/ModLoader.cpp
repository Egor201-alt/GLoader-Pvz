#include "ModLoader.h"
#include <windows.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <ctime>
#include <vector>
#include <iomanip>

// JSON library
#include <nlohmann/json.hpp>
using json = nlohmann::json;
namespace fs = std::filesystem;

// Глобальные переменные
std::ofstream logFile;
struct Mod {
    std::string name;
    std::string version;
};
std::vector<Mod> loadedMods;

// --- СИСТЕМА ЛОГИРОВАНИЯ ---

void InitLogging() {
    // 1. Создаем папку logs, если нет
    if (!fs::exists("./logs")) {
        fs::create_directory("./logs");
    }

    // 2. Открываем (или создаем) файл logs/latest.log
    // ios::trunc означает, что при каждом запуске игры лог очищается
    logFile.open("./logs/latest.log", std::ios::out | std::ios::trunc);

    // 3. Создаем консольное окно (черное)
    AllocConsole();
    
    // 4. Перенаправляем вывод std::cout в это окно
    FILE* fDummy;
    freopen_s(&fDummy, "CONOUT$", "w", stdout);
    freopen_s(&fDummy, "CONOUT$", "w", stderr);

    // Приветствие
    std::cout << "==========================================" << std::endl;
    std::cout << "      GLoader Core Initialized     " << std::endl;
    std::cout << "==========================================" << std::endl;
    Log("Logger initialized successfully.");
}

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
        logFile.flush(); // Сразу сохраняем на диск
    }
}

// --- ЗАГРУЗЧИК МОДОВ ---

void LoadMods() {
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
                    // Log("Skipping folder " + modDirName + " (no mod.json found).");
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

                // Полный путь к DLL
                fs::path dllFullPath = entry.path() / dllName;
                
                if (!fs::exists(dllFullPath)) {
                     Log("  -> Error: DLL file missing at " + dllFullPath.string());
                     continue;
                }

                // Попытка загрузки DLL
                HMODULE hMod = LoadLibraryA(dllFullPath.string().c_str());
                
                if (hMod) {
                    Log("  -> Loaded successfully!");
                    loadedMods.push_back({modName, data.value("version", "1.0")});
                    successCount++;
                } else {
                    // Получаем код ошибки Windows
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

    Log("Mod loading finished.");
    Log("Total active mods: " + std::to_string(successCount));
}