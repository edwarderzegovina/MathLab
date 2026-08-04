#include "DataEntity.h"
#include "Logger.h"
#include <string>

DataEntity::DataEntity(const std::string& name,const char unit):MathEntity(name),unit(unit) {
    Logger::getInstance().log("Created DataEntity: " + name, "DataEntity");
}

DataEntity::DataEntity(const DataEntity &alt):MathEntity(alt),unit(alt.unit) {
    Logger::getInstance().log("Created by copy DataEntity: " + name, "DataEntity");
}

DataEntity::~DataEntity()noexcept {
    Logger::getInstance().log("Destroyed DataEntity: " + name, "DataEntity");
}
