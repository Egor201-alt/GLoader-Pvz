#include "ModLoader.h"
#include <windows.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>
// Мы будем использовать простую логику без тяжелых JSON либ для начала,
// или подключим nlohmann/json через CMake (см. ниже).
#include <nlohmann/json.hpp>

using json = nlohmann::json;
namespace fs = std::filesystem;

struct Mod {
    std::string name;
    std::string version;
    std::string entryDll;
    std::vector<std::string> dependencies;
};

std::vector<Mod> loadedMods;

void ShowError(const std::string& msg) {
    MessageBoxA(NULL, msg.c_str(), "PvZ Mod Loader Error", MB_ICONERROR);
}

void LoadMods() {
    std::string modsPath = "./mods";
    if (!fs::exists(modsPath)) {
        fs::create_directory(modsPath);
        return; // Модов нет
    }

    // 1. Сканируем папки
    for (const auto& entry : fs::directory_iterator(modsPath)) {
        if (entry.is_directory()) {
            std::string modJsonPath = entry.path().string() + "/mod.json";
            if (fs::exists(modJsonPath)) {
                try {
                    std::ifstream f(modJsonPath);
                    json data = json::parse(f);

                    Mod newMod;
                    newMod.name = data["name"];
                    newMod.entryDll = entry.path().string() + "/" + std::string(data["dll"]);
                    
                    if (data.contains("dependencies")) {
                        for (auto& dep : data["dependencies"]) {
                            newMod.dependencies.push_back(dep);
                        }
                    }
                    
                    // Тут надо бы проверить зависимости...
                    // Для простоты грузим сразу:
                    HMODULE hMod = LoadLibraryA(newMod.entryDll.c_str());
                    if (!hMod) {
                        ShowError("Failed to load DLL for mod: " + newMod.name);
                    } else {
                        loadedMods.push_back(newMod);
                    }

                } catch (const std::exception& e) {
                    ShowError("Error parsing mod.json in " + entry.path().string() + "\n" + e.what());
                }
            }
        }
    }
}