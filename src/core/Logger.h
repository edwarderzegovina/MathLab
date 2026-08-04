#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <sstream>
#include <fstream>
#include <set>

class Logger {
    std::ostringstream logBuffer;
    std::set<std::string> uniqueTypes; // O(log(n))

    Logger() {
        logBuffer << "===== New Session =====\n";
    }

public:
    // Meyers Singleton
    static Logger& getInstance() {
        static Logger instance;
        return instance;
    }

    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    void log(const std::string& message, const std::string& type = "General") {
        logBuffer << "[LOG] " << message << "\n";
        uniqueTypes.insert(type);
    }

    // Rendered, not printed: the log is data, and which stream (if any) it
    // ends up on is the front-end's decision.
    std::string uniqueTypesText() const {
        std::ostringstream os;
        os << "Object types processed in this session: ";
        for (const auto& t : uniqueTypes) os << t << " ";
        os << "\n";
        return os.str();
    }

    std::string logText() const {
        return logBuffer.str();
    }

    ~Logger() {
        std::ofstream file("logs.txt");
        if (file.is_open()) {
            file << logBuffer.str();
            file << "===== Session Ended =====\n\n";
            file.close();
        }
    }
};

#endif
