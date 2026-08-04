#include "ConsoleApp.h"
#include "Logger.h"

int main() {
    Logger::getInstance().log("=== Session started ===");

    ConsoleApp::getInstance().startApplication();

    Logger::getInstance().log("=== Session ended ===");
    return 0;
}