#ifndef PROIECT02_MATHCACHE_H
#define PROIECT02_MATHCACHE_H
#include "IObject.h"
#include <string>
#include <sstream>
#include <vector>
using namespace std;

template <class T, int N>
class MathCache : public IObject {
    std::vector<T> storage;
public:
    MathCache() { storage.reserve(N); }
    ~MathCache() noexcept override = default;

    string toString() const override {
        ostringstream os;
        os << "Cache with id " << getId() << " has the following elements: ";
        for (const auto& e : storage) os << e;
        return os.str();
    }

    //we don't want to copy MathCache
    MathCache(const MathCache&) = delete;
    MathCache& operator=(const MathCache&) = delete;

    void addELement(const T& element) {
        if ((int)storage.size() >= N) throw overflow_error("Cache is full");
        storage.push_back(element);   // no slot consumed if this throws
    }

    int getCurrentCount() const noexcept { return (int)storage.size(); }

    template <class U>
    void castAndAdd(const U& value) {
        if ((int)storage.size() >= N) throw overflow_error("Cache is full");
        storage.push_back(static_cast<T>(value));
    }
};

//specialisation for string, strictly keeps logs (displayed messages)
template<>
class MathCache<string,10> {
    std::string textStorage[10];
    int count = 0;
public:
    void addELement(const std::string& msg) {
        if (count < 10) textStorage[count++] = msg;
    }
    string cacheLogsText() const {
        ostringstream os;
        os << "--- Last "<<count<< " text messages in Cache ---\n";
        for (int i = 0; i < count; i++) os << textStorage[i] << "\n";
        return os.str();
    }
};


#endif //PROIECT02_MATHCACHE_H