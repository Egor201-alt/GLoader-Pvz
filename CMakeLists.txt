cmake_minimum_required(VERSION 3.15)
project(PvZ_Modding_System)

# Автоматически скачиваем библиотеку JSON
include(FetchContent)
FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.11.2/json.tar.xz)
FetchContent_MakeAvailable(json)

# --- Собираем Ядро (PvZ_Core.dll) ---
add_library(PvZ_Core SHARED 
    src/Core/dllmain.cpp 
    src/Core/ModLoader.cpp
)
target_link_libraries(PvZ_Core PRIVATE nlohmann_json::nlohmann_json)
# Копируем DLL в папку build (для удобства)
set_target_properties(PvZ_Core PROPERTIES PREFIX "") # Чтобы не было libPvZ_Core.dll

# --- Собираем Оболочку (version.dll) ---
add_library(version SHARED src/Proxy/version.cpp)
set_target_properties(version PROPERTIES PREFIX "") # Важно! Имя должно быть version.dll
# Опционально: нужен .def файл для экспорта, но пока пробуем без него