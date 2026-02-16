cmake_minimum_required(VERSION 3.15)
project(PvZ_Modding_System)

# 1. Принудительно 32-бита (на всякий случай дублируем тут)
set(CMAKE_SIZEOF_VOID_P 4)

# 2. ВКЛЮЧАЕМ C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 3. ГЛАВНОЕ: Статическая линковка (/MT). 
# Это убирает зависимость от VCRUNTIME140.dll
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded")

# Скачиваем JSON
include(FetchContent)
FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.11.2/json.tar.xz)
FetchContent_MakeAvailable(json)

# --- ЯДРО ---
add_library(PvZ_Core SHARED src/Core/dllmain.cpp src/Core/ModLoader.cpp)
target_link_libraries(PvZ_Core PRIVATE nlohmann_json::nlohmann_json)
set_target_properties(PvZ_Core PROPERTIES PREFIX "") 

# --- ОБОЛОЧКА (Теперь WINMM.DLL) ---
add_library(winmm SHARED src/Proxy/version.cpp)
set_target_properties(winmm PROPERTIES PREFIX "")