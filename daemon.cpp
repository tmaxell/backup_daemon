#include <iostream>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <syslog.h>
#include <thread>
#include <csignal>
#include <iomanip>

void handleSignal(int signalNumber) {
    syslog(LOG_NOTICE, "BackupService stopped");
    closelog();
    exit(signalNumber);
}

void loadConfiguration(std::string& sourceDir, std::string& backupDir, unsigned& backupFrequency) {
    std::ifstream configFile("settings.yaml");

    if (!configFile.is_open()) {
        syslog(LOG_ERR, "ERROR: Failed to open settings.yaml");
    }

    std::string line;
    while (getline(configFile, line)) {
        std::istringstream iss(line);
        std::string key, value;

        getline(iss, key, ':');
        getline(iss, value);
        value = value.substr(1);

        if (key == "sourceDir") sourceDir = value;
        if (key == "backupDir") backupDir = value;
        if (key == "frequency") {
            if (value == "minutely") backupFrequency = 1;
            else if (value == "hourly") backupFrequency = 60;
            else if (value == "daily") backupFrequency = 1440;
            else if (value == "weekly") backupFrequency = 10080;
            else if (value == "monthly") backupFrequency = 43200;
            else {
                syslog(LOG_ERR, "ERROR: Unsupported frequency format");
                handleSignal(1);
            }
        }
    }
}

std::string getCurrentTimestamp() {
    auto currentTime = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(currentTime);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time), "%H:%M:%S %d-%m-%Y");
    return ss.str();
}

std::string extractDirectoryName(const std::string& path) {
    std::filesystem::path filePath(path);
    return filePath.filename();
}

void performBackup(const std::string& sourceDir, const std::string& backupDir, unsigned &backupFrequency) {
    while (true) {
        std::string backupFolder = backupDir + "/" + extractDirectoryName(sourceDir) + "_backup " + getCurrentTimestamp();

        if (!std::filesystem::create_directory(backupFolder)) {
            syslog(LOG_ERR, "ERROR: Failed to create backup directory");
            exit(1);
        }

        std::filesystem::copy(sourceDir, backupFolder, std::filesystem::copy_options::recursive);

        syslog(LOG_INFO, "Backup created successfully: %s", backupFolder.c_str());

        std::this_thread::sleep_for(std::chrono::minutes(backupFrequency));
    }
}

int main() {
    signal(SIGTERM, handleSignal);

    openlog("BackupService", LOG_PID, LOG_DAEMON);
    syslog(LOG_NOTICE, "BackupService started");

    std::string sourceDir, backupDir;
    unsigned backupFrequency;

    loadConfiguration(sourceDir, backupDir, backupFrequency);

    performBackup(sourceDir, backupDir, backupFrequency);

    return 0;
}
