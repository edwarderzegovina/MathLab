#ifndef PROIECT02_SOLUTIONREPORT_H
#define PROIECT02_SOLUTIONREPORT_H

#include <string>
#include <vector>

// What the solver concluded about a linear system.
//
// NOTE on Inconsistent: LinearSystem::solveSystem() throws
// CalculationException for an inconsistent system rather than returning a
// report carrying this kind -- the console relies on that (it prints the
// exception through its own handler). The enumerator still exists so a
// front-end that would rather render than catch has a value to render, and so
// the classification is nameable outside the solver.
enum class SolutionKind { Unique, Infinite, Inconsistent };

// The whole answer to "solve this system", with nothing printed.
//
// `values` is filled for Unique; `parametrised` and `freeParameters` are
// filled for Infinite. `parametrised[j]` is the already-rendered right-hand
// side for variable j ("3 - 2*t1"), because the rendering rules (which terms
// are suppressed as zero, when the coefficient 1 is left implicit) belong with
// the numbers that produced them, not duplicated in each front-end.
struct SolutionReport {
    SolutionKind kind = SolutionKind::Unique;
    std::vector<std::string> variableNames;   // one per variable, in column order
    std::vector<float> values;                // Unique: value of each variable
    std::vector<std::string> parametrised;    // Infinite: expression per variable
    std::vector<std::string> freeParameters;  // Infinite: "t1 = y (free parameter)"
    std::string message;                      // human-readable summary line

    // Renders the report exactly as the console prints it, trailing newline
    // included. One authority for the text: AlgebraEntity::solve() returns
    // this, the CLI printer writes it.
    std::string toText() const;
};

#endif //PROIECT02_SOLUTIONREPORT_H
