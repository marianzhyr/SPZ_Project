#include <iostream>
#include <filesystem>
#include <fstream>
#include <vector>
#include <string>
#include <ctime>
#include <stdexcept>
#include <unordered_set>

namespace fs = std::filesystem;

class BackupManager {
public:
    BackupManager(const std::string& backupDirectory) : backupDirectory(backupDirectory) {
        if (!fs::exists(backupDirectory)) {
            fs::create_directories(backupDirectory);
        }
    }

    void addDirectory(const std::string& directory) {
        if (fs::exists(directory) && fs::is_directory(directory)) {
            directories.push_back(directory);
        }
        else {
            logError("Directory does not exist: " + directory);
        }
    }

    void addExclusion(const std::string& path) {
        if (fs::exists(path)) {
            exclusions.insert(fs::absolute(path));
        }
        else {
            logError("Exclusion path does not exist: " + path);
        }
    }

    void performBackup() {
        for (const auto& directory : directories) {
            try {
                copyDirectory(directory, backupDirectory);
            }
            catch (const std::exception& e) {
                logError(e.what());
            }
        }
        std::cout << "Backup completed." << std::endl;
    }

private:
    std::vector<std::string> directories;
    std::unordered_set<fs::path> exclusions;
    std::string backupDirectory;

    void copyDirectory(const std::string& source, const std::string& destination) {
        for (const auto& entry : fs::recursive_directory_iterator(source)) {
            const auto& path = entry.path();
            auto relativePathStr = path.lexically_relative(source).string();
            auto destPath = fs::path(destination) / relativePathStr;

            if (isExcluded(path)) {
                continue;
            }

            if (fs::is_directory(path)) {
                fs::create_directories(destPath);
            }
            else {
                fs::copy(path, destPath, fs::copy_options::overwrite_existing);
            }
        }
    }

    bool isExcluded(const fs::path& path) const {
        for (const auto& exclusion : exclusions) {
            if (path.string().find(exclusion.string()) == 0) {
                return true;
            }
        }
        return false;
    }

    void logError(const std::string& message) {
        std::ofstream logFile("backup_log.txt", std::ios_base::app);
        std::time_t now = std::time(nullptr);
        char timeBuffer[26]; // Buffer for formatted time string
        ctime_s(timeBuffer, sizeof(timeBuffer), &now);
        logFile << timeBuffer << ": " << message << std::endl;
    }
};

void printMenu() {
    std::cout << "\nBackup Manager Menu:\n";
    std::cout << "1. Set Backup Directory\n";
    std::cout << "2. Add Directory to Backup\n";
    std::cout << "3. Add Exclusion Path\n";
    std::cout << "4. Perform Backup\n";
    std::cout << "5. Exit\n";
    std::cout << "Enter your choice: ";
}

int main() {
    std::string backupDirectory;
    BackupManager* backupManager = nullptr;

    while (true) {
        printMenu();
        int choice;
        std::cin >> choice;

        switch (choice) {
        case 1:
            std::cout << "Enter backup directory: ";
            std::cin >> backupDirectory;
            backupManager = new BackupManager(backupDirectory);
            break;

        case 2:
            if (!backupManager) {
                std::cout << "Set backup directory first.\n";
                break;
            }
            {
                std::string directory;
                std::cout << "Enter directory to backup: ";
                std::cin >> directory;
                backupManager->addDirectory(directory);
            }
            break;

        case 3:
            if (!backupManager) {
                std::cout << "Set backup directory first.\n";
                break;
            }
            {
                std::string exclusion;
                std::cout << "Enter exclusion path: ";
                std::cin >> exclusion;
                backupManager->addExclusion(exclusion);
            }
            break;

        case 4:
            if (!backupManager) {
                std::cout << "Set backup directory first.\n";
                break;
            }
            backupManager->performBackup();
            break;

        case 5:
            delete backupManager;
            return 0;

        default:
            std::cout << "Invalid choice, please try again.\n";
            break;
        }
    }

    return 0;
}
