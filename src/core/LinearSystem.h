#ifndef LINEARSYSTEM_HPP
#define LINEARSYSTEM_HPP

#include <istream>
#include <ostream>
#include <cmath>
#include <string>
#include <stdexcept>
#include "Matrix.h"
#include "AlgebraEntity.h"
#include "SolutionReport.h"

using namespace std;

class LinearSystem:public AlgebraEntity {
    int nrOfVariables,nrOfEquations;
    Matrix matrix; // this will practically be A|b, already the augmented matrix
    // The stored matrix is the AUGMENTED matrix [A|b]: equations x (variables+1).
    // Matrix::MAX_SIZE caps both matrix dimensions at 10, so the two LinearSystem
    // limits are genuinely different: variables+1 <= 10 => variables <= 9,
    // while equations <= 10 directly. Do not collapse these into one constant.
    static const int MAX_VARIABLES;
    static const int MAX_EQUATIONS;

    // Validates the pair and returns `variables`, so it can be used directly in
    // a mem-init list that must not allocate before the shape is known good.
    static int checkedVariables(int variables, int equations);
public:
    // Public for the same reason as Matrix::validateDimensions: the console
    // reader checks the shape before constructing anything, because a
    // constructor that throws from its mem-init list has already consumed an
    // id the user can see.
    static void validateShape(int variables, int equations);
    // Column j of the augmented matrix holds the coefficient of variable
    // symbols[j]. Public so callers that print or reconstruct a system's
    // equations (e.g. ConsoleApp::saveToDirectory) share this single table
    // instead of keeping their own copy that could drift out of sync.
    static const string symbols;

    MathEntity* clone() const override;
    string solve() override;   // the report of solveSystem(), rendered
    Matrix getMatrix() const;
    int getNrOfVariables() const;
    int getNrOfEquations() const;
    LinearSystem();
    LinearSystem(const Matrix& matrix,int nrOfVariables,int nrOfEquations,const string& name="N/A");
    explicit LinearSystem(int size,const string& name="N/A"); // creates a square system populated with 0
    LinearSystem(int nrOfVariables,int nrOfEquations,const string& name="N/A"); //creates a system with nrOfVariables/nrOfEquations populated with 0
    LinearSystem(const LinearSystem& alt);
    ~LinearSystem()noexcept override;
    LinearSystem& operator=(const LinearSystem& alt);
    friend istream& operator>>(istream& in,LinearSystem& alt);
    friend ostream& operator<<(ostream& out,const LinearSystem& alt);

    //increment operators that add 1 to all terms in system
    LinearSystem& operator++();
    LinearSystem operator++(int);
    LinearSystem& operator--();
    LinearSystem operator--(int);

    //based on the determinant of the augmented matrix.
    bool operator==(const LinearSystem& alt) const;
    bool operator<(const LinearSystem& alt)const;


    LinearSystem operator+(const LinearSystem& alt) const;
    LinearSystem operator-(const LinearSystem& alt) const;
    float operator[](size_t index) const;

    // Solves and RETURNS the answer. Throws CalculationException for an
    // inconsistent system, exactly as solveAndPrint() did.
    SolutionReport solveSystem() const;

    string toString() const override;
};

#endif
