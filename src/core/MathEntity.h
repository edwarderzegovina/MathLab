
#ifndef PROIECT02_MATHENTITY_H
#define PROIECT02_MATHENTITY_H
#include "IObject.h"
#include <string>

class MathEntity:public IObject {
protected:
    std::string name; // protected: visible in derived classes but hidden globally
public:
    explicit MathEntity(const std::string& name="N/A");
    MathEntity(const MathEntity& alt);
    MathEntity& operator=(const MathEntity& alt)=default;
    void setName(const std::string& name);
    const std::string& getName() const;

    virtual MathEntity* clone() const = 0;    //makes a clone of this object good for polymorfic behavior
                                              // MathEntitys[i].clone() --> matrix/system/...
    virtual ~MathEntity() noexcept;
};


#endif //PROIECT02_MATHENTITY_H