#ifndef CONSOLEAPP_HPP
#define CONSOLEAPP_HPP

#include <iostream>
#include <string>
#include <filesystem>
#include <cctype>
#include <limits>
#include "MathCache.h"
#include "MathLabException.h"
#include "Matrix.h"
#include "LinearSystem.h"
#include "Dataset.h"
#include "Polynomial.h"
#include "Workspace.h"
#include "IMathFactory.h"

using namespace std;
namespace fs = filesystem;

// The terminal front-end: menus, prompts and printing. Every object and every
// operation lives in Workspace, which knows nothing about a terminal; each
// menu entry here reads its input, calls one Workspace method, prints the
// result, and lets the surrounding handler display any exception.
class ConsoleApp {
    MathCache<Matrix,10> mathCache;
    Workspace& workspace;

    void promptSaveMatrixResult(const Matrix& result);
    void promptSavePolynomialResult(const Polynomial& result);
    void promptSaveSystemResult(const LinearSystem& result);
    void printComparisons(const string& lhsLabel, const string& rhsLabel,
                          const ComparisonResult& comparison) const;
    ConsoleApp();
    ConsoleApp& operator=(const ConsoleApp& alt) = delete;
    ConsoleApp(const ConsoleApp& alt) = delete;
public:
    void printDatasets()const;
    void printMatrices() const;
    void printPolynomials() const;
    void printSystems() const;
    ~ConsoleApp()noexcept;
    void startApplication();
    void genericMenu();
    void algebraMenu();
    void dataEntityMenu();
    void matrixMenu();
    void polynomialMenu();
    void systemMenu();
    void datasetMenu();

    void buildEntity(const IMathFactory* factory,int buildType);
    void factoryMenu();

    static ConsoleApp& getInstance();
};

#endif
