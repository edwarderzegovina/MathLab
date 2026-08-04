#include "IObject.h"
#include <ostream>

int IObject::idCounter = 1000;

IObject::IObject():id(++IObject::idCounter) {}

int IObject::getId() const noexcept {return id;}

bool IObject::operator==(const IObject &alt) const {return id==alt.id;}

std::ostream& operator<<(std::ostream &out, const IObject &obj) {
    out<<obj.toString();
    return out;
}

IObject::IObject(const IObject&):id(++idCounter) {}

IObject& IObject::operator=(const IObject&) noexcept { return *this; }
