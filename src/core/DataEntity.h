#ifndef PROIECT02_DATAENTITY_H
#define PROIECT02_DATAENTITY_H
#include "MathEntity.h"
#include <string>

class DataEntity:public MathEntity {
protected:
    char unit; // what do the numbers represent ex(N-number, %-fractions, etc...)
public:
    explicit DataEntity(const std::string& name = "DataEntity",const char unit='N');
    DataEntity(const DataEntity& alt);
    DataEntity& operator=(const DataEntity& alt)=default;
    // Returns the report as text instead of printing it (see
    // AlgebraEntity::solve for the reasoning). The "--- Data report: ... ---"
    // banner is rendered by DatasetSummary::toText(), the single authority
    // for the whole report.
    virtual std::string summaryText() const = 0;
    ~DataEntity() noexcept override;
};


#endif //PROIECT02_DATAENTITY_H