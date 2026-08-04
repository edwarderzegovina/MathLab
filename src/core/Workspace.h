#ifndef PROIECT02_WORKSPACE_H
#define PROIECT02_WORKSPACE_H

#include <algorithm>
#include <list>
#include <string>
#include <vector>

#include "Dataset.h"
#include "DatasetSummary.h"
#include "LinearSystem.h"
#include "MathEntity.h"
#include "MathLabException.h"
#include "Matrix.h"
#include "Polynomial.h"
#include "SolutionReport.h"

// All six relational results for a pair of entities, computed once here so
// every front-end renders the same comparison instead of recomputing it.
struct ComparisonResult {
    bool equal = false;
    bool notEqual = false;
    bool less = false;
    bool lessOrEqual = false;
    bool greater = false;
    bool greaterOrEqual = false;
};

// Owns every object in the session and performs every operation on them.
//
// Deliberately UI-free: no prompting, no printing, not even a terminal stream
// header. Each operation takes already-validated arguments and either returns
// a value or throws -- so the console can drive it with a terminal, a GUI can
// drive it with widgets, and a test can drive it with stdin closed.
class Workspace {
    std::list<MathEntity*> objects;   // list for O(1) insert/delete

    Workspace();
    Workspace(const Workspace&) = delete;
    Workspace& operator=(const Workspace&) = delete;

public:
    static Workspace& instance();
    ~Workspace() noexcept;

    // --- the collection -------------------------------------------------
    void add(MathEntity* entity);          // takes ownership
    void remove(int id);                   // throws IndexNotFoundException
    void removeEntity(MathEntity* entity); // no-op for nullptr
    MathEntity* byId(int id) const;        // throws IndexNotFoundException
    std::vector<MathEntity*> all() const;
    size_t size() const noexcept;
    void clear();
    void sortByName();

    //inspired by C# list.ofType<T>();
    //vector for faster traversal
    template <typename T>
    std::vector<T*> ofType() const noexcept {
        std::vector<T*> filteredList;
        for (const auto& x : objects) {
            auto* ptr = dynamic_cast<T*>(x);
            if (ptr != nullptr)
                filteredList.push_back(ptr);
        }
        return filteredList;
    }

    template <typename T>
    static std::pair<T*,T*> getTwoElementsById(const std::vector<T*>& collection, int id1, int id2) {
        T* firstItem = nullptr, *secondItem = nullptr;
        auto it1 = std::find_if(collection.begin(),collection.end(),
            [&](const auto& obj) {
               return id1 == obj->getId();
            });
        if (it1 == collection.end())
            throw IndexNotFoundException("use command 1. to see the index of available objects",id1);
        auto it2 = std::find_if(collection.begin(),collection.end(),
            [&](const auto& obj) {
               return id2 == obj->getId();
            });
        if (it2 == collection.end())
            throw IndexNotFoundException("use command 1. to see the index of available objects",id2);
        firstItem = *it1;
        secondItem = *it2;
        return std::make_pair(firstItem,secondItem);
    }

    template <typename T>
    static T* getElementById(const std::vector<T*>& collection, int id) {
        auto it = std::find_if(collection.begin(),collection.end(),
            [&](const auto& obj) {
                return id == obj->getId();
            });
        if (it == collection.end())
            throw IndexNotFoundException("use command 1. to see the index of available objects",id);
        T* ptr = *it;
        return ptr;
    }

    // Typed lookup: an id that exists but names another type is "not found".
    Matrix*       matrixById(int id) const;
    Polynomial*   polynomialById(int id) const;
    LinearSystem* systemById(int id) const;
    Dataset*      datasetById(int id) const;

    // --- persistence ----------------------------------------------------
    void loadFromDirectory(const std::string& directoryName);
    void saveToDirectory(const std::string& directoryName) const;
    void loadMatrices(const std::string& filePath);
    void loadSystems(const std::string& filePath);
    void loadDatasets(const std::string& filePath);
    void loadPolynomials(const std::string& filePath);

    // --- matrices -------------------------------------------------------
    // op is '+', '-', '*' or 'c' (concat).
    Matrix matrixBinaryOp(int lhsId, int rhsId, char op) const;
    Matrix matrixScalarOp(int id, float scalar, char op) const;   // '+', '-', '*'
    Matrix matrixNegate(int id) const;
    Matrix matrixPower(int id, int power) const;
    Matrix matrixEchelon(int id) const;
    float  matrixDeterminant(int id) const;
    float  matrixElement(int id, size_t index) const;
    int    matrixElementCount(int id) const;
    // Commits a single cell for the GUI's editable grid. Goes through
    // Matrix's existing extraction operator (operator>>) rather than
    // constructing a replacement Matrix, so the id and name are untouched --
    // it reads back every element (the one just edited plus everything else,
    // unchanged) and reassigns them all in one commit, exactly like a user
    // retyping the whole matrix would.
    void   matrixSetElement(int id, size_t index, float value);
    ComparisonResult matrixCompare(int lhsId, int rhsId) const;
    // op is '+' or '-'; prefix returns the new state, postfix the previous one.
    Matrix matrixStep(int id, char op, bool prefix);

    // --- polynomials ----------------------------------------------------
    Polynomial polynomialBinaryOp(int lhsId, int rhsId, char op) const;  // '+', '-', '*'
    Polynomial polynomialNegate(int id) const;
    Polynomial polynomialDerivative(int id) const;
    Polynomial polynomialIntegral(int id) const;
    float  polynomialAt(int id, float x) const;
    float  polynomialDefiniteIntegral(int id, float upper, float lower) const;
    float  polynomialCoefficient(int id, size_t index) const;
    size_t polynomialMaxDegree(int id) const;
    // Commits a single coefficient for the GUI's editable list. Rebuilds the
    // polynomial through the existing (coefficients, maxDegree) constructor
    // and assignment operator -- the id survives (IObject::operator= never
    // touches it) but the name does not, so it is saved and restored around
    // the assignment.
    void   polynomialSetCoefficient(int id, size_t index, float value);
    std::string polynomialSaveFormat(int id) const;
    ComparisonResult polynomialCompare(int lhsId, int rhsId) const;
    Polynomial polynomialStep(int id, char op, bool prefix);

    // --- linear systems -------------------------------------------------
    SolutionReport solveSystem(int id) const;
    LinearSystem systemBinaryOp(int lhsId, int rhsId, char op) const;    // '+', '-'
    float systemFreeTerm(int id, size_t equationIndex) const;
    ComparisonResult systemCompare(int lhsId, int rhsId) const;
    LinearSystem systemStep(int id, char op, bool prefix);

    // --- data sets ------------------------------------------------------
    DatasetSummary summariseDataset(int id) const;
    Dataset datasetBinaryOp(int lhsId, int rhsId, char op) const;        // '+', '-', '*'
    void    datasetAddValue(int id, double value);
    void    datasetSort(int id);
    void    datasetRemoveOutliers(int id);
    bool    datasetContains(int id, double value) const;
    double  datasetElement(int id, size_t index) const;
    void    datasetExportToCSV(int id, const std::string& fileName) const;
    Dataset datasetImportFromCSV(const std::string& fileName) const;
    Dataset datasetStep(int id, char op, bool prefix);
    ComparisonResult datasetCompare(int lhsId, int rhsId) const;

    // --- polymorphic entries (the Generic/Algebra/Data interfaces) -------
    MathEntity* cloneEntity(int id);          // clones AND adopts the clone
    MathEntity* cloneAlgebraEntity(int id);
    MathEntity* cloneDataEntity(int id);
    std::string solveEntity(int id);          // any AlgebraEntity
    std::string summariseEntity(int id) const;// any DataEntity
};

#endif //PROIECT02_WORKSPACE_H
