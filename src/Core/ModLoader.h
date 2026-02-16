#pragma once
#include <string>

// Инициализация консоли и системы логов
void InitLogging();

// Главная функция для записи сообщений
void Log(const std::string& msg);

// Загрузка модов
void LoadMods();