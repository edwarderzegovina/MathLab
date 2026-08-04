#include "Matrix.h"
#include <sstream>
#include <ostream>
#include "MathLabException.h"
#include "Logger.h"
int Matrix::objectCount = 0;

void Matrix::validateDimensions(const int rows_, const int columns_) {
    if (rows_ <= 0 || columns_ <= 0)
        throw DimensionMismatchException("dimensions must be strictly positive");
    if (rows_ > MAX_SIZE || columns_ > MAX_SIZE)
        throw DimensionMismatchException("Maximum accepted size is 10x10");
}

Matrix Matrix::toEchelonForm(const bool augmented) const {
    auto temp = data;

    // Tolerance scaled by the matrix's maximum magnitude, used only for
    // comparisons -- never written back into the data.
    float maxAbs = 0.0f;
    for (const auto& row : data)
        for (const float v : row) maxAbs = std::max(maxAbs, std::fabs(v));
    const float eps = 1e-6f * std::max(1.0f, maxAbs);

    // For the augmented matrix of a LinearSystem we don't look for a pivot on
    // the last column (the free-term column); otherwise we pivot on all columns.
    const size_t pivotColumns = augmented
        ? ((columns > 0) ? static_cast<size_t>(columns - 1) : 0)
        : static_cast<size_t>(columns);
    size_t pivotRow = 0;
    const size_t rowCount = static_cast<size_t>(rows);
    const size_t colCount = static_cast<size_t>(columns);
    for (size_t col = 0; col < pivotColumns && pivotRow < rowCount; ++col) {
        size_t bestRow = pivotRow;
        float bestAbs = fabs(temp[bestRow][col]);
        for (size_t r = pivotRow + 1; r < rowCount; ++r) {
            const float currentAbs = fabs(temp[r][col]);
            if (currentAbs > bestAbs) {
                bestAbs = currentAbs;
                bestRow = r;
            }
        }

        if (bestAbs < eps)
            continue;

        if (bestRow != pivotRow) {
            for (size_t j = 0; j < colCount; ++j)
                swap(temp[pivotRow][j], temp[bestRow][j]);
        }

        const float pivot = temp[pivotRow][col];

        for (size_t r = pivotRow + 1; r < rowCount; ++r) {
            const float factor = temp[r][col] / pivot;
            for (size_t j = col; j < colCount; ++j)
                temp[r][j] -= factor * temp[pivotRow][j];
        }

        ++pivotRow;
    }

    return Matrix(temp);
}

//throw error if rows don't match
Matrix Matrix::concat(const Matrix& matrix) const {
    if (rows != matrix.rows)
        throw DimensionMismatchException("concatenation requires the same number of rows");
    if (columns + matrix.columns > MAX_SIZE)
        throw DimensionMismatchException("concatenation result exceeds 10 columns");
    const int nrOfColumns = columns + matrix.columns;
    vector<vector<float>> temp(static_cast<size_t>(rows), vector<float>(static_cast<size_t>(nrOfColumns), 0.0f));
    for (int i=0;i<rows;i++) {
        for (int j=0;j<columns;j++)
            temp[i][j] = data[i][j];
        for (int j=columns;j<nrOfColumns;j++)
            temp[i][j] = matrix.data[i][j-columns];
    }
    return Matrix(temp);
}
Matrix Matrix::toPower(const int power) const {
    if (rows != columns)
        throw DimensionMismatchException("raising to a power requires a square matrix");
    if (power < 0)
        throw CalculationException("negative power", static_cast<unsigned>(getId()));
    if (power == 0) return Matrix(rows);          // identity
    Matrix result(*this);
    for (int i = 1; i < power; i++) result = result * (*this);
    return result;
}

//on same sizes only
Matrix Matrix::operator+(const Matrix& alt) const{
    //throw exception if sizes don't match
    if (rows!=alt.rows || columns != alt.columns)
        throw DimensionMismatchException("Cannot add matrices of different dimensions");
    vector<vector<float>> temp(static_cast<size_t>(rows), vector<float>(static_cast<size_t>(columns), 0.0f));
    for (int i=0;i<rows;i++) {
        for (int j=0;j<columns;j++)
            temp[i][j] = data[i][j]+alt.data[i][j];
    }
    return Matrix(temp);
}
Matrix Matrix::operator-(const Matrix& alt) const {
    return *this+-alt;
}
Matrix Matrix::operator*(const Matrix& alt) const {
    if (columns != alt.rows)
        throw DimensionMismatchException("Cannot multiply matrices "+ to_string(getId())+ " " + to_string(alt.getId()));
    vector<vector<float>> temp(static_cast<size_t>(rows), vector<float>(static_cast<size_t>(alt.columns), 0.0f));
    for (int i=0;i<rows;i++)
        for (int j=0;j<alt.columns;j++)
            for (int k=0;k<columns;k++)
                temp[i][j]+=data[i][k]*alt.data[k][j];
    return Matrix(temp);
}

//index starts from 0.
float Matrix::operator[](const size_t index) const {
    //throw exeption if index >= rows*columns
    if (index >= static_cast<size_t>(rows)*static_cast<size_t>(columns))
        throw IndexException("You selected an element that doesn't exist in matrix " + to_string(getId()));
    return data[index/columns][index%columns];
}

bool Matrix::operator!=(const Matrix& alt) const {
    return !(*this==alt);
}
bool Matrix::operator==(const Matrix& alt) const {
    return rows * columns == alt.rows * alt.columns;
}
bool Matrix::operator<(const Matrix& alt) const {
    return rows*columns<alt.rows*alt.columns;
}
bool Matrix::operator>(const Matrix& alt) const {
    return alt<*this;
}
bool Matrix::operator<=(const Matrix& alt) const {
    return *this<alt || *this==alt;
}
bool Matrix::operator>=(const Matrix& alt) const {
    return alt<=*this;
}

//try/catch for a logic error
float Matrix::determinant() const {
    if (rows!=columns)
        throw CalculationException("Cannot compute the determinant of a non-square matrix",getId());
    vector<vector<double>> temp(static_cast<size_t>(rows), vector<double>(static_cast<size_t>(rows), 0.0));
    double maxAbs = 0.0;
    for (int i=0;i<rows;i++) {
        for (int j=0;j<rows;j++) {
            temp[i][j] = data[i][j];
            maxAbs = std::max(maxAbs, std::fabs(temp[i][j]));
        }
    }
    // Tolerance scaled by the maximum magnitude, keeping the absolute
    // threshold of 1e-9 for matrices of small or normal magnitude.
    const double EPS = 1e-9 * std::max(1.0, maxAbs);

    double det = 1.0;
    int sign = 1;

    for (int i=0;i<rows;i++) {
        int pivot = i;
        double best = fabs(temp[i][i]);
        for (int r=i+1;r<rows;r++) {
            const double current = fabs(temp[r][i]);
            if (current > best) {
                best = current;
                pivot = r;
            }
        }

        if (best < EPS) {
            return 0.0f;
        }

        if (pivot != i) {
            swap(temp[pivot],temp[i]);
            sign *= -1;
        }

        const double pivotValue = temp[i][i];
        det *= pivotValue;

        for (int r=i+1;r<rows;r++) {
            const double factor = temp[r][i] / pivotValue;
            temp[r][i] = 0.0;
            for (int c=i+1;c<rows;c++)
                temp[r][c] -= factor * temp[i][c];
        }
    }
    return static_cast<float>(det * sign);
}

Matrix Matrix::operator-() const {
    vector<vector<float>> temp(static_cast<size_t>(rows), vector<float>(static_cast<size_t>(columns), 0.0f));
    for (int i=0;i<rows;i++) {
        for (int j=0;j<columns;j++)
            temp[i][j] = -data[i][j];
    }
    return Matrix(temp);
}

Matrix Matrix::operator+(const float scalar) const {
    vector<vector<float>> temp(static_cast<size_t>(rows), vector<float>(static_cast<size_t>(columns), 0.0f));
    for (int i=0;i<rows;i++) {
        for (int j=0;j<columns;j++)
            temp[i][j] = data[i][j] + scalar;
    }
    return Matrix(temp);
}
Matrix Matrix::operator-(const float scalar) const {
    return *this+(-scalar);
}
Matrix operator+(const float scalar,const Matrix& matrix) {
    return matrix+scalar;
}
Matrix operator-(const float scalar,const Matrix& matrix) {
    return scalar+(-matrix);
}

Matrix Matrix::operator*(const float scalar) const {
    vector<vector<float>> temp(static_cast<size_t>(rows), vector<float>(static_cast<size_t>(columns), 0.0f));
    for (int i=0;i<rows;i++) {
        for (int j=0;j<columns;j++)
            temp[i][j] = data[i][j]*scalar;
    }
    return Matrix(temp);
}
Matrix operator*(const float scalar,const Matrix& matrix) {
    return matrix*scalar;
}

Matrix Matrix::operator++(int) {
    Matrix coppy(*this);
    ++*this;
    return coppy;
}
Matrix& Matrix::operator++() {
    for (size_t i=0;i<static_cast<size_t>(rows);i++)
        for (size_t j=0;j<static_cast<size_t>(columns);j++)
            data[i][j]++;
    return *this;
}
Matrix& Matrix::operator--() {
    for (size_t i=0;i<static_cast<size_t>(rows);i++)
        for (size_t j=0;j<static_cast<size_t>(columns);j++)
            data[i][j]--;
    return *this;
}
Matrix Matrix::operator--(int) {
    Matrix coppy(*this);
    --*this;
    return coppy;
}

// Every extraction is checked and throws on the first unparseable element, so
// a matrix that fails to read never silently ends up part-zero -- matching
// LinearSystem, Polynomial and Dataset, which all throw on unparseable input.
//
// The values land in a local first and are committed in one step, so a matrix
// that fails to read keeps whatever it held before rather than becoming half
// old and half new.
//
// No prompt is printed here: an extraction operator that writes to the
// terminal cannot be reused by a GUI or read from a file/string source.
// ConsoleIO::readMatrix prints "Enter: C numbers per row, R rows." before
// calling this.
istream& operator>>(istream& in, Matrix& matrix) {
    const auto rowCount = static_cast<size_t>(matrix.rows);
    const auto colCount = static_cast<size_t>(matrix.columns);
    vector<vector<float>> read(rowCount, vector<float>(colCount, 0.0f));
    for (size_t i=0;i<rowCount;i++)
        for (size_t j=0;j<colCount;j++)
            if (!(in>>read[i][j]))
                throw ParseException("Matrix elements must be numbers; "
                                     "I need " + to_string(matrix.rows) + "x" +
                                     to_string(matrix.columns) + " values");
    matrix.data = std::move(read);
    return in;
}
ostream& operator<<(ostream& out,const Matrix& matrix) {
    for (size_t i=0;i<static_cast<size_t>(matrix.rows);i++) {
        out<<'(';
        for (size_t j=0;j+1<static_cast<size_t>(matrix.columns);j++) {
            out<<matrix.data[i][j]<<',';
        }
        out<<matrix.data[i][matrix.columns-1]<<')'<<endl;
    }
    return out;
}
Matrix::Matrix(const float* data_1D,const int rows_,const int columns_):AlgebraEntity("Matrix"+to_string(objectCount)),rows(rows_),columns(columns_) {
    validateDimensions(rows_, columns_);
    if (data_1D == nullptr)
        throw DimensionMismatchException("null pointer passed to constructor");
    Logger::getInstance().log("Created Matrix: " + name, "Matrix");
    data.assign(static_cast<size_t>(rows_), vector<float>(static_cast<size_t>(columns_), 0.0f));
    for (int i=0;i<rows_;i++) {
        for (int j=0;j<columns_;j++)
            data[i][j] = data_1D[i*columns_ + j];
    }
    objectCount++;
}
Matrix::Matrix(float** data_2D,const int rows_,const int columns_):AlgebraEntity("Matrix"+to_string(objectCount)),rows(rows_),columns(columns_) {
    validateDimensions(rows_, columns_);
    if (data_2D == nullptr)
        throw DimensionMismatchException("null pointer passed to constructor");
    for (int i=0;i<rows_;i++) {
        if (data_2D[i] == nullptr)
            throw DimensionMismatchException("null pointer passed to constructor");
    }
    Logger::getInstance().log("Created Matrix: " + name, "Matrix");
    data.assign(static_cast<size_t>(rows_), vector<float>(static_cast<size_t>(columns_), 0.0f));
    for (int i=0;i<rows_;i++) {
        for (int j=0;j<columns_;j++)
            data[i][j]=data_2D[i][j];
    }
    objectCount++;
}

Matrix::Matrix(const vector<vector<float>>& data_2D):AlgebraEntity("Matrix"+to_string(objectCount)),rows(static_cast<int>(data_2D.size())),columns(data_2D.empty()?0:static_cast<int>(data_2D[0].size())) {
    validateDimensions(rows, columns);
    for (const auto& row : data_2D) {
        if (static_cast<int>(row.size()) != columns)
            throw DimensionMismatchException("Rows of different sizes in matrix");
    }
    data = data_2D;
    Logger::getInstance().log("Created Matrix: " + name, "Matrix");
    objectCount++;
}
Matrix::Matrix(const int size):AlgebraEntity("Matrix"+to_string(objectCount)),rows(size),columns(size) {
    validateDimensions(size, size);
    Logger::getInstance().log("Created Matrix: " + name, "Matrix");
    data.assign(static_cast<size_t>(size), vector<float>(static_cast<size_t>(size), 0.0f));
    for (int i=0;i<size;i++) {
        data[i][i] = 1;
    }
    objectCount++;
}

Matrix::Matrix(const int rows_,const int columns_):AlgebraEntity("Matrix"+to_string(objectCount)),rows(rows_),columns(columns_) {
    validateDimensions(rows_, columns_);
    Logger::getInstance().log("Created Matrix: " + name, "Matrix");
    data.assign(static_cast<size_t>(rows_), vector<float>(static_cast<size_t>(columns_), 0.0f));
    objectCount++;
}
Matrix::Matrix():AlgebraEntity("Matrix"+to_string(objectCount)),rows(3),columns(3) {
    Logger::getInstance().log("Created Matrix: " + name, "Matrix");
    data.assign(3, vector<float>(3, 0.0f));
    objectCount++;
}

Matrix::~Matrix()noexcept {
    Logger::getInstance().log("Destroyed Matrix: " + name, "Matrix");
    objectCount--;
}
Matrix::Matrix(const Matrix &alt):AlgebraEntity(alt),rows(alt.rows),columns(alt.columns) {
    Logger::getInstance().log("Created by copy Matrix: " + name, "Matrix");
    data = alt.data;
    objectCount++;
}

Matrix& Matrix::operator=(const Matrix& alt) {
    if (this == &alt) return *this;
    AlgebraEntity::operator=(alt);   // keeps this object's id
    rows = alt.rows;
    columns = alt.columns;
    data = alt.data;
    return *this;
}

Matrix::Matrix(Matrix&& alt) noexcept
    : AlgebraEntity(alt), data(std::move(alt.data)),
      rows(alt.rows), columns(alt.columns) {
    objectCount++;
    alt.rows = 0;
    alt.columns = 0;
}

Matrix& Matrix::operator=(Matrix&& alt) noexcept {
    if (this == &alt) return *this;
    // The name travels with the data, exactly as in copy-assignment -- so
    // after `a = std::move(b)`, a's name matches its new shape rather than
    // staying stale. Moved rather than routed through AlgebraEntity::operator=,
    // whose defaulted string COPY could throw bad_alloc out of a noexcept
    // function; the id is untouched either way.
    name = std::move(alt.name);
    data = std::move(alt.data);
    rows = alt.rows;
    columns = alt.columns;
    alt.rows = 0;
    alt.columns = 0;
    return *this;
}

const vector<vector<float>>& Matrix::getData() const {
    return data;
}

int Matrix::getColumns() const {
    return columns;
}

int Matrix::getRows() const {
    return rows;
}

string Matrix::solve() {
    ostringstream os;
    os<<"The determinant of matrix "<<to_string(getId())<< " is: "<<determinant();
    return os.str();
}

MathEntity *Matrix::clone() const {
    return new Matrix(*this);
}

int Matrix::getObjectCount() {return objectCount;}

string Matrix::toString() const {
    ostringstream os;

    os<<"[Matrix ID="<<getId()<<"]\n";
    os<<"Name = "<<name<<endl;
    os<<"Dimensions " <<rows<<" x "<<columns<<endl;
    os<<*this;

    return os.str();
}
