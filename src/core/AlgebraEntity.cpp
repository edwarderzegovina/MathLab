#include "AlgebraEntity.h"
#include "Logger.h"
#include <sstream>

AlgebraEntity::AlgebraEntity(const std::string& name):MathEntity(name) {
    Logger::getInstance().log("Created AlgebraEntity: " + name, "AlgebraEntity");
}

AlgebraEntity::AlgebraEntity(const AlgebraEntity &alt):MathEntity(alt) {
    Logger::getInstance().log("Created by copy AlgebraEntity: " + name, "AlgebraEntity");
}

AlgebraEntity::~AlgebraEntity()noexcept {
    Logger::getInstance().log("Destroyed AlgebraEntity: " + name, "AlgebraEntity");
}
