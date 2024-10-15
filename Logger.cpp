#include "Logger.h"
#include <fstream>
#include <ctime>
#include <iostream>

const std::string LOG_FILE_NAME = "logfile.txt";

std::string getCurrentTime() {
    std::time_t now = std::time(nullptr);
    char buf[80];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", std::localtime(&now));
    return std::string(buf);
}

void Logger::logMessage(const std::string& message) {
    std::ofstream log_file;
    log_file.open(LOG_FILE_NAME, std::ios::app);
    if (log_file.is_open()) {
        log_file << getCurrentTime() << " - " << message << std::endl;
        log_file.close();
    }
    else {
        std::cerr << "Erreur : impossible d'ouvrir le fichier de log !" << std::endl;
    }
}
