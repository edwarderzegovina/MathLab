#include "LinearSystem.h"
#include <sstream>
#include <vector>
#include "MathLabException.h"
#include "Logger.h"

bool LinearSystem::operator==(const LinearSystem& alt) const {
    if (nrOfVariables != alt.nrOfVariables) return false;
    if (nrOfEquations != alt.nrOfEquations) return false;
    return matrix.getData() == alt.matrix.getData();
}

bool LinearSystem::operator<(const LinearSystem& alt) const {
    // Total order that always exists: size first, then lexicographic on the
    // augmented data (determinant() would require a square matrix, which the
    // augmented matrix generally isn't).
    if (nrOfVariables * nrOfEquations != alt.nrOfVariables * alt.nrOfEquations)
        return nrOfVariables * nrOfEquations < alt.nrOfVariables * alt.nrOfEquations;
    if (nrOfEquations != alt.nrOfEquations) return nrOfEquations < alt.nrOfEquations;
    return matrix.getData() < alt.matrix.getData();
}

//the number in the free-term column of row index
float LinearSystem::operator[](const size_t index) const {
    const auto eqCount = static_cast<size_t>(nrOfEquations);
    if (index >= eqCount)
        throw IndexException("[!] You must enter a number smaller than " + to_string(nrOfEquations));
    return matrix.getData()[index][static_cast<size_t>(nrOfVariables)];
}

LinearSystem LinearSystem::operator+(const LinearSystem& alt) const {
    if (nrOfVariables!=alt.nrOfVariables || nrOfEquations!=alt.nrOfEquations)
        throw DimensionMismatchException("The dimensions of the two systems do not match");
    return {matrix+alt.matrix,nrOfVariables,nrOfEquations,name};
}

LinearSystem LinearSystem::operator-(const LinearSystem &alt) const {
    if (nrOfVariables!=alt.nrOfVariables || nrOfEquations!=alt.nrOfEquations)
        throw DimensionMismatchException("The dimensions of the two systems do not match");
    return {matrix-alt.matrix,nrOfVariables,nrOfEquations,name};
}

LinearSystem& LinearSystem::operator++() {
    ++matrix;
    return *this;
}
LinearSystem& LinearSystem::operator--() {
    --matrix;
    return *this;
}
LinearSystem LinearSystem::operator++(int) {
    LinearSystem coppy(*this);
    ++*this;
    return coppy;
}

LinearSystem LinearSystem::operator--(int) {
    LinearSystem coppy(*this);
    --*this;
    return coppy;
}

#include <sstream>
#include <string>
#include <algorithm>
#include <cctype>

// stof stops at the first character it cannot use and reports how far it got in
// its second argument. Without checking that, "1.2.3" would silently parse as
// 1.2 and "5abc" as 5. Nothing short of a fully consumed token is a valid
// literal here.
static float parseFullFloat(const string& token, const string& message) {
    size_t consumed = 0;
    float value = 0.0f;
    try { value = stof(token, &consumed); }
    catch (const std::exception&) { throw ParseException(message); }
    if (consumed != token.size())
        throw ParseException(message);
    return value;
}

istream& operator>>(istream& in, LinearSystem& alt) {
    Matrix built(alt.nrOfEquations, alt.nrOfVariables + 1);
    auto data = built.getData();

    for (int k = 0; k < alt.nrOfEquations; k++) {
        string line;
        // Skip blank lines, then take the whole equation including spaces.
        do { if (!getline(in, line)) throw ParseException("Not enough equations"); }
        while (line.find_first_not_of(" \t\r") == string::npos);

        line.erase(remove_if(line.begin(), line.end(),
                             [](unsigned char c){ return isspace(c); }), line.end());

        const size_t eq = line.find('=');
        if (eq == string::npos)
            throw ParseException("You did not enter '=' in equation: '" + line + "'");

        const string lhs = line.substr(0, eq);
        const string rhs = line.substr(eq + 1);
        data[(size_t)k][(size_t)alt.nrOfVariables] =
            parseFullFloat(rhs, "Invalid free term: '" + rhs + "'");

        size_t i = 0;
        while (i < lhs.size()) {
            float sign = 1.0f;
            if (lhs[i] == '+') { i++; }
            else if (lhs[i] == '-') { sign = -1.0f; i++; }

            const size_t numStart = i;
            while (i < lhs.size() &&
                   (isdigit((unsigned char)lhs[i]) || lhs[i] == '.')) i++;
            float coefficient = 1.0f;
            if (i > numStart) {
                // Optional scientific-notation exponent suffix (e/E, optional
                // sign, one or more digits). operator<< formats coefficients
                // with ostream<<float, which switches to scientific notation
                // outside [1e-5, 1e6), so a saved "1e+06*x=1" must read back.
                // Only consumed when digits actually follow the marker/sign: a
                // bare "2e" or "2e+" is left in place so the variable lookup
                // below rejects it, exactly as Polynomial::operator>> does.
                if (i < lhs.size() && (lhs[i] == 'e' || lhs[i] == 'E')) {
                    size_t expLookahead = i + 1;
                    if (expLookahead < lhs.size() &&
                        (lhs[expLookahead] == '+' || lhs[expLookahead] == '-'))
                        expLookahead++;
                    if (expLookahead < lhs.size() &&
                        isdigit((unsigned char)lhs[expLookahead])) {
                        i = expLookahead;
                        while (i < lhs.size() && isdigit((unsigned char)lhs[i])) i++;
                    }
                }
                coefficient = parseFullFloat(lhs.substr(numStart, i - numStart),
                                             "Invalid coefficient in '" + lhs + "'");
            }
            if (i < lhs.size() && lhs[i] == '*') i++;

            if (i >= lhs.size())
                throw ParseException("Missing variable in '" + lhs + "'");

            // Place by NAME, not by position.
            const char symbol = lhs[i++];
            const size_t col = LinearSystem::symbols.find(symbol);
            if (col == string::npos || (int)col >= alt.nrOfVariables)
                throw ParseException(string("Variable '") + symbol +
                                     "' does not belong to the system");
            data[(size_t)k][col] += sign * coefficient;
        }
    }
    alt.matrix = Matrix(data);
    return in;
}

ostream& operator<<(ostream& out,const LinearSystem& alt) {
    const auto& tempData = alt.matrix.getData();
    const auto eqCount = static_cast<size_t>(alt.nrOfEquations);
    const auto varCount = static_cast<size_t>(alt.nrOfVariables);
    for (size_t i=0;i<eqCount;i++) {
        for (size_t j=0;j<varCount;j++) {
            const float coef = tempData[i][j];
            out << (j == 0 ? "" : (coef < 0 ? "" : "+")) << coef << "*"
                << LinearSystem::symbols[j];
        }
        out<<'='<<tempData[i][varCount]<<endl;
    }
    return out;
}

// Gauss elimination via Matrix::toEchelonForm, then a Rouche-Capelli rank
// comparison to classify the system, then back-substitution (with
// free-variable parametrisation in the Infinite case).
SolutionReport LinearSystem::solveSystem() const {
    // The tolerance must scale with the system's magnitude: a fixed absolute
    // threshold misclassifies systems whose coefficients are all very small
    // (falsely reads as rank-deficient) or very large (swallows real
    // coefficients as noise). We equilibrate each row before elimination,
    // not just the comparisons -- multiplying a whole equation (row,
    // including the free term) by a nonzero constant leaves its solution set
    // unchanged, and it lets toEchelonForm's own internal tolerance (tuned
    // for magnitude around 1) work correctly too.
    //
    // Equilibration is done PER ROW rather than with one global factor: a
    // single factor can only re-center one magnitude, so a system whose rows
    // have very different scales would have some rows normalized and others
    // crushed below EPS. Row equilibration puts every row's maximum in
    // [0.5,1), and so the whole matrix's maximum too, matching the EPS used
    // below.
    //
    // The factor is a power of two (frexp/ldexp), so the scaling introduces no
    // rounding error. A row that is entirely zero is left untouched (there is
    // no factor and nothing to normalize).
    vector<vector<float>> scaled = matrix.getData();
    for (auto& row : scaled) {
        float rowMax = 0.0f;
        for (const float v : row) rowMax = max(rowMax, fabs(v));
        if (rowMax == 0.0f)
            continue;                      // row is identically zero: nothing to normalize
        int exponent = 0;
        frexp(rowMax, &exponent);          // rowMax = m * 2^exponent, m in [0.5,1)
        // ldexp(1.0f, k) gives +inf for k >= 128 (a subnormal row, with
        // rowMax < 2^-128, calls for exactly that) and drops into subnormals
        // below k = -126. We apply the shift in steps that stay normal
        // numbers, so each multiplication is still exact.
        for (int shift = -exponent; shift != 0; ) {
            const int step = shift > 126 ? 126 : (shift < -126 ? -126 : shift);
            const float factor = ldexp(1.0f, step);
            for (float& v : row) v *= factor;
            shift -= step;
        }
    }

    const float EPS = 1e-6f;               // absolute on equilibrated data = relative on each row
    //Get the echelon form matrix
    const Matrix echelon = Matrix(scaled).toEchelonForm(true);
    const auto& data = echelon.getData();
    int rankA = 0;
    int rankAb = 0;
    //compute the rank of matrix A and of the augmented matrix
    for (int i = 0; i < nrOfEquations; i++) {
        bool hasCoeff = false;
        for (int j = 0; j < nrOfVariables; j++) {
            if (fabs(data[i][j]) > EPS) {
                hasCoeff = true;
                break;
            }
        }
        const bool hasCoeffb = hasCoeff || fabs(data[i][nrOfVariables]) > EPS;
        if (hasCoeff)
            rankA++;
        if (hasCoeffb)
            rankAb++;
    }
    // Rouche-Capelli classification.
    if (rankA < rankAb) {
        throw CalculationException("Inconsistent system: cannot be solved",getId());
    }
    SolutionReport report;
    report.variableNames.reserve(static_cast<size_t>(nrOfVariables));
    for (int j = 0; j < nrOfVariables; j++)
        report.variableNames.emplace_back(1, LinearSystem::symbols[static_cast<size_t>(j)]);

    if (rankA < nrOfVariables) {
        report.kind = SolutionKind::Infinite;
        report.message = "Consistent system: infinitely many solutions.";
        //infinitely many solutions
        // columns with pivots are basic (leading) variables, those without are free (secondary)
        auto pivotColByRow = vector<int>(nrOfEquations, -1); //pivotColByRow[i] = pivot column of row i
        auto isPivotColumn = vector<bool>(nrOfVariables, false);
        for (int i = 0; i < nrOfEquations; i++) {
            for (int j = 0; j < nrOfVariables; j++) {
                if (fabs(data[i][j]) > EPS) {
                    pivotColByRow[i] = j;
                    isPivotColumn[j] = true;
                    break;
                }
            }
        }
        vector<int> freeColumns;
        freeColumns.reserve(nrOfVariables);
        for (int j = 0; j < nrOfVariables; j++)
            if (!isPivotColumn[j])
                freeColumns.push_back(j);
        const int paramCount = static_cast<int>(freeColumns.size());  //how many parameters we have (secondary variables)
        // Each variable has the form
        // x_j = constPart[j] + sum(paramCoeff[j][k] * t(k+1)).
        auto constPart = vector<float>(nrOfVariables, 0.0f);
        auto paramCoeff = vector<vector<float>>(nrOfVariables, vector<float>(paramCount, 0.0f));
        // secondary variables become parameters
        for (int k = 0; k < paramCount; k++) {
            const int col = freeColumns[k];
            paramCoeff[col][k] = 1.0f;
        }
        // back-substitution method for the pivot variables
        for (int i = nrOfEquations - 1; i >= 0; i--) {
            const int pivot = pivotColByRow[i];
            if (pivot == -1)
                continue;
            float rhsConst = data[i][nrOfVariables]; //the constant part at the end, also computed from the other pivots
            auto rhsParam = vector<float>(paramCount, 0.0f);//coefficients of the secondary variables
            for (int j = pivot + 1; j < nrOfVariables; j++) {
                if (fabs(data[i][j]) <= EPS)
                    continue;
                rhsConst -= data[i][j] * constPart[j];
                for (int k = 0; k < paramCount; k++)
                    rhsParam[k] -= data[i][j] * paramCoeff[j][k];
            }
            //divide the constant part and coefficients by the pivot coefficient so we have value for one.
            constPart[pivot] = rhsConst / data[i][pivot];
            for (int k = 0; k < paramCount; k++)
                paramCoeff[pivot][k] = rhsParam[k] / data[i][pivot];
        }
        // the secondary variables
        for (int k = 0; k < paramCount; k++) {
            ostringstream line;
            line << "t" << (k + 1) << " = " << LinearSystem::symbols[freeColumns[k]] << " (free parameter)";
            report.freeParameters.push_back(line.str());
        }
        // the parametric solutions
        //pretty printer with spaces.
        for (int j = 0; j < nrOfVariables; j++) {
            ostringstream expression;
            bool printedSomething = false;
            if (fabs(constPart[j]) > EPS) {
                expression << constPart[j];
                printedSomething = true;
            }
            for (int k = 0; k < paramCount; k++) {
                const float c = paramCoeff[j][k];
                if (fabs(c) <= EPS)
                    continue;
                if (!printedSomething) {
                    if (c < 0)
                        expression << '-';
                } else {
                    expression << (c >= 0 ? " + " : " - ");
                }
                const float absC = fabs(c);
                if (fabs(absC - 1.0f) > EPS)
                    expression << absC << '*';
                expression << 't' << (k + 1);
                printedSomething = true;
            }
            if (!printedSomething)
                expression << 0;
            report.parametrised.push_back(expression.str());
        }
        return report;
    }

    // Unique solution, same principle as above.
    auto solution = vector<float>(nrOfVariables, 0.0f);
    for (int i = nrOfEquations - 1; i >= 0; i--) {
        int pivotCol = -1;
        for (int j = 0; j < nrOfVariables; j++) {
            if (fabs(data[i][j]) > EPS) {
                pivotCol = j;
                break;
            }
        }

        // All-zero coefficient row (already handled by rank logic).
        if (pivotCol == -1)
            continue;

        float sumKnown = 0.0f;
        for (int j = pivotCol + 1; j < nrOfVariables; j++)
            sumKnown += data[i][j] * solution[j];

        solution[pivotCol] = (data[i][nrOfVariables] - sumKnown) / data[i][pivotCol];
    }

    report.kind = SolutionKind::Unique;
    report.message = "Consistent system: unique solution.";
    report.values = solution;
    return report;
}

// Assignment reshapes rather than throwing, matching Matrix::operator=: this
// lets LinearSystem be stored and reassigned in std::sort or vector::erase
// over a container that mixes shapes.
//
// The class invariant — matrix is nrOfEquations x (nrOfVariables+1) — carries
// across intact: `alt` already satisfies it, and Matrix::operator= copies rows,
// columns and data together, so the three members always move as one.
LinearSystem &LinearSystem::operator=(const LinearSystem &alt) {
    if (this==&alt)return *this;
    AlgebraEntity::operator=(alt);   // keeps this object's id
    nrOfVariables = alt.nrOfVariables;
    nrOfEquations = alt.nrOfEquations;
    matrix = alt.matrix;
    return *this;
}

LinearSystem::LinearSystem(const LinearSystem &alt):AlgebraEntity(alt),nrOfVariables(alt.nrOfVariables),nrOfEquations(alt.nrOfEquations),matrix(alt.matrix) {
    Logger::getInstance().log("Created by copy LinearSystem: " + name, "LinearSystem");
}

void LinearSystem::validateShape(const int variables, const int equations) {
    if (variables <= 0 || equations <= 0)
        throw DimensionMismatchException("the system requires at least one variable and one equation");
    if (variables > MAX_VARIABLES)
        throw DimensionMismatchException("The number of variables is too large, maximum " +
                                         to_string(MAX_VARIABLES));
    if (equations > MAX_EQUATIONS)
        throw DimensionMismatchException("The number of equations is too large, maximum " +
                                         to_string(MAX_EQUATIONS));
}

int LinearSystem::checkedVariables(const int variables, const int equations) {
    validateShape(variables, equations);
    return variables;
}

LinearSystem::LinearSystem(const int nrOfVariables_,const int nrOfEquations_,const string& name)
    : AlgebraEntity(name),
      nrOfVariables(checkedVariables(nrOfVariables_, nrOfEquations_)),
      nrOfEquations(nrOfEquations_),
      matrix(nrOfEquations_, nrOfVariables_ + 1) {}

LinearSystem::LinearSystem(const int size,const string& name)
    : AlgebraEntity(name),
      nrOfVariables(checkedVariables(size, size)),
      nrOfEquations(size),
      matrix(size, size + 1) {}

LinearSystem::~LinearSystem() noexcept {
    Logger::getInstance().log("Destroyed LinearSystem: " + name, "LinearSystem");
}

//throws an error if the system's size doesn't match the matrix's size
LinearSystem::LinearSystem(const Matrix &matrix_,const int nrOfVariables_,const int nrOfEquations_,const string& name)
    : AlgebraEntity(name),
      nrOfVariables(checkedVariables(nrOfVariables_, nrOfEquations_)),
      nrOfEquations(nrOfEquations_),
      matrix(matrix_) {
    if (matrix_.getRows() != nrOfEquations_ ||
        matrix_.getColumns() != nrOfVariables_ + 1)
        throw DimensionMismatchException(
            "The associated matrix is incompatible with the system");
}

//the system 0=0;
LinearSystem::LinearSystem()
    : AlgebraEntity("N/A"), nrOfVariables(1), nrOfEquations(1), matrix(1, 2) {}

Matrix LinearSystem::getMatrix() const {
    return matrix;
}

int LinearSystem::getNrOfVariables() const {
    return nrOfVariables;
}

int LinearSystem::getNrOfEquations() const {
    return nrOfEquations;
}

MathEntity *LinearSystem::clone() const {
    return new LinearSystem(*this);
}

string LinearSystem::solve() {
    return solveSystem().toText();
}


string LinearSystem::toString() const {
    ostringstream os;

    os<<"[System ID="<<getId()<<"]\n";
    os<<"Name = "<<name<<endl;
    os<<nrOfVariables<<" unknowns and " <<nrOfEquations<<" equations\n";
    os<<*this;

    return os.str();
}

// Augmented matrix is equations x (variables+1); Matrix::MAX_SIZE is 10.
const int LinearSystem::MAX_VARIABLES = 9;  // variables+1 <= 10
const int LinearSystem::MAX_EQUATIONS = 10; // equations <= 10
const string LinearSystem::symbols = "xyztwuvnm";

