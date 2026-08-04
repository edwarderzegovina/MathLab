#ifndef PROIECT02_IOBJECT_H
#define PROIECT02_IOBJECT_H

#include <string>
#include <iosfwd>

class IObject {
    int id;
    static int idCounter;
public:
    IObject();
    virtual ~IObject()noexcept =default;
    int getId() const noexcept;
    virtual std::string toString() const=0;
    bool operator==(const IObject& alt) const;
    friend std::ostream& operator<<(std::ostream& out,const IObject& obj);
    IObject(const IObject& alt);
    // Identity is never copied: an object keeps the id it was constructed with.
    // The copy constructor mints a fresh id; assignment leaves id alone.
    IObject& operator=(const IObject&) noexcept;
};


#endif //PROIECT02_IOBJECT_H