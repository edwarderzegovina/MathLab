#ifndef PROIECT02_ALGEBRAENTITY_H
#define PROIECT02_ALGEBRAENTITY_H

#include "MathEntity.h"
#include <string>

class AlgebraEntity:public MathEntity {
public:
    explicit AlgebraEntity(const std::string& name = "AlgebraEntity");
    AlgebraEntity(const AlgebraEntity& alt);
    AlgebraEntity& operator=(const AlgebraEntity& alt)= default;
    // Returns the worked answer as text instead of printing it, so a GUI can
    // put it in a label and the console can print it. Deliberately NOT
    // const: Polynomial::solve() trims cancelled leading terms first, which is
    // a mutation the caller is entitled to see.
    virtual std::string solve() = 0;
    ~AlgebraEntity() noexcept override;
};


#endif //PROIECT02_ALGEBRAENTITY_H