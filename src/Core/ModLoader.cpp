#include "ModLoader.h"
#include <windows.h>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

// Подключаем JSON (библиотека скачается сама через CMake)
#include <nlohmann/json.hpp>
using json = nlohmann::json;
namespace fs = std::filesystem;

// Структура мода
struct Mod {
    std::string name;
    std::string dllPath;
};

std::vector<Mod> loadedMods;

// Показываем ошибку (можно убрать в релизе)
void ShowError(const std::string& msg) {
    MessageBoxA(NULL, msg.c_str(), "PvZ Mod Loader", MB_ICONERROR);
}

void LoadMods() {
    // Пауза, чтобы игра успела инициализировать свои окна (не обязательно, но полезно)
    Sleep(500); 

    std::string modsPath = "./mods";
    
    // Если папки нет - создаем и выходим
    if (!fs::exists(modsPath)) {
        try {
            fs::create_directory(modsPath);
        } catch (...) {}
        return;
    }

    // Перебираем папки внутри /mods/
    for (const auto& entry : fs::directory_iterator(modsPath)) {
        if (entry.is_directory()) {
            // Ищем mod.json
            fs::path jsonPath = entry.path() / "mod.json";
            
            if (fs::exists(jsonPath)) {
                try {
                    std::ifstream f(jsonPath);
                    json data = json::parse(f);

                    // Получаем имя DLL из конфига
                    std::string dllName = data.value("dll", "");
                    if (dllName.empty()) continue; // Если DLL не указана, пропускаем

                    // Собираем полный путь к DLL мода
                    fs::path dllFullPath = entry.path() / dllName;
                    std::string strPath = dllFullPath.string();

                    // Загружаем DLL мода
                    HMODULE hMod = LoadLibraryA(strPath.c_str());
                    
                    if (hMod) {
                        // Успех!
                        std::string modName = data.value("name", "Unknown Mod");
                        loadedMods.push_back({modName, strPath});
                    } else {
                        ShowError("Could not load DLL: " + strPath);
                    }

                } catch (const std::exception& e) {
                    ShowError("Error loading mod in " + entry.path().string() + ":\n" + e.what());
                }
            }
        }
    }
}