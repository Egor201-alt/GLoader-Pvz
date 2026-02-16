cmake_minimum_required(VERSION 3.15)
project(PvZ_Modding_System)

# --- ГЛАВНОЕ ИСПРАВЛЕНИЕ: ВКЛЮЧАЕМ C++17 ---
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# ------------------------------------------

# Скачиваем библиотеку JSON
include(FetchContent)
FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.11.2/json.tar.xz)
FetchContent_MakeAvailable(json)

# Собираем Ядро (Core)
add_library(PvZ_Core SHARED 
    src/Core/dllmain.cpp 
    src/Core/ModLoader.cpp
)
# Подключаем JSON к ядру
target_link_libraries(PvZ_Core PRIVATE nlohmann_json::nlohmann_json)
# Убираем префикс "lib" (чтобы было PvZ_Core.dll, а не libPvZ_Core.dll)
set_target_properties(PvZ_Core PROPERTIES PREFIX "")

# Собираем Оболочку (Proxy)
add_library(version SHARED src/Proxy/version.cpp)
set_target_properties(version PROPERTIES PREFIX "")