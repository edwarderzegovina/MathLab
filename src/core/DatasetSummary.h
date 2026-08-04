#ifndef PROIECT02_DATASETSUMMARY_H
#define PROIECT02_DATASETSUMMARY_H

#include <string>

// Everything the console prints about a data set, plus the two statistics
// (median, standard deviation) the "Detailed statistics" menu computes
// separately, so one call answers both menu entries.
//
// min/max/mean/median/stddev are only meaningful when `empty` is false: on an
// empty set getMin()/getMax()/getMedian() throw, so the summary reports
// emptiness instead of inventing numbers.
struct DatasetSummary {
    std::string name;
    char unit = 'N';
    bool empty = true;
    bool sorted = false;
    size_t size = 0;
    double min = 0.0;
    double max = 0.0;
    double mean = 0.0;
    double median = 0.0;
    double stddev = 0.0;

    // Renders the summary exactly as the console prints it, header and
    // trailing newline included.
    std::string toText() const;
};

#endif //PROIECT02_DATASETSUMMARY_H
