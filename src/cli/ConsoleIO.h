#ifndef MATHLAB_CONSOLEIO_H
#define MATHLAB_CONSOLEIO_H

#include <iostream>
#include <string>
#include "Matrix.h"
#include "LinearSystem.h"
#include "Polynomial.h"
#include "Dataset.h"
#include "MathLabException.h"
#include "SolutionReport.h"
#include "DatasetSummary.h"

// Everything that talks to the terminal on behalf of the core types. The
// entity classes read from an istream and render to a string; the terminal
// prompts (e.g. operator>>'s "Enter: N numbers per row") live here instead,
// because a GUI has no use for them and core must not name cin/cout.
namespace ConsoleIO {

    // Prompts and reads a whole matrix: both dimension prompts first, then
    // the dimensions, then the element prompt, then the elements. The shape
    // is validated BEFORE any Matrix is constructed, so a rejected shape
    // consumes no object id.
    Matrix readMatrix();

    // Prompts and reads a whole system, name (which may contain spaces)
    // included.
    LinearSystem readSystem();

    // The element prompt on its own, for the menu path that has already asked
    // for the dimensions itself.
    void promptMatrixElements(const Matrix& matrix);

    // Thin printers: the text itself is built by the core renderers, so the
    // console and any other front-end cannot drift apart.
    void printSolution(std::ostream& out, const SolutionReport& report);
    void printSummary(std::ostream& out, const DatasetSummary& summary);

    // The message for a caught MathLab exception. Recovering cin (clearing it
    // and discarding the offending line) after an InputException is console
    // business, not core's, so it happens here rather than in what().
    const char* describe(const MathLabException& error);
}

#endif
