#include "DatasetSummary.h"
#include <sstream>

std::string DatasetSummary::toText() const {
    std::ostringstream os;
    os << "\n--- Data report: " << name << " [" << unit << "] ---\n";
    if (empty) {
        os << "(empty set)\n";
        return os.str();
    }
    os << "Number of elements: " << size << "\n";
    os << "Sorted: " << (sorted ? "True" : "False") << "\n";
    os << "Min: " << min << "\n";
    os << "Max: " << max << "\n";
    os << "Mean: " << mean << "\n";
    return os.str();
}
