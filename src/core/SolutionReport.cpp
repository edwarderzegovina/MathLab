#include "SolutionReport.h"
#include <sstream>

std::string SolutionReport::toText() const {
    std::ostringstream os;
    os << message << "\n";
    if (kind == SolutionKind::Infinite) {
        for (const auto& line : freeParameters)
            os << line << "\n";
        for (size_t j = 0; j < parametrised.size(); j++)
            os << variableNames[j] << " = " << parametrised[j] << "\n";
        return os.str();
    }
    for (size_t j = 0; j < values.size(); j++)
        os << variableNames[j] << " = " << values[j] << "\n";
    return os.str();
}
