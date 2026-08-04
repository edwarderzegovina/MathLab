#include "MathEntity.h"
#include "Logger.h"
#include <sstream>

MathEntity::MathEntity(const std::string& name):name(name) {
    Logger::getInstance().log("Created object: " + name + " [ID=" + std::to_string(getId()) + "]", "MathEntity");
}

MathEntity::MathEntity(const MathEntity &alt) : IObject(alt),name(alt.name) {
    Logger::getInstance().log("Created by copy: " + name + " [ID=" + std::to_string(getId()) + "]", "MathEntity");
}

MathEntity::~MathEntity() noexcept{
    Logger::getInstance().log("Destroyed object: " + name + " [ID=" + std::to_string(getId()) + "]", "MathEntity");
}

void MathEntity::setName(const std::string& name) {
    this->name = name;
}

const std::string& MathEntity::getName() const {
    return name;
}

