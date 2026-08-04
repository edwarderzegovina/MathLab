#include "Workspace.h"

#include <cctype>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <limits>
#include <memory>
#include <sstream>

#include "Logger.h"

namespace fs = std::filesystem;
using namespace std;

Workspace& Workspace::instance() {
    static Workspace theWorkspace;
    return theWorkspace;
}

// Deliberately silent: the log is printed to the user by menu entry 10, so a
// line logged here would appear in that listing the first time anything
// touched the workspace -- a visible change for a refactor that must not have
// any.
//
// It does force the Logger into existence, though, without writing to it.
// Both are function-local statics, so they are destroyed in reverse order of
// construction; the workspace destructor and every entity destructor log, so
// the Logger must be constructed FIRST or it is already gone by then. Touching
// it here is what guarantees that, whatever order a caller happens to use.
Workspace::Workspace() {
    (void)Logger::getInstance();
}

Workspace::~Workspace() noexcept {
    Logger::getInstance().log("Workspace starting object cleanup...", "System");
    for (auto* obj : objects)
        delete obj;
    objects.clear();
}

void Workspace::add(MathEntity* entity) {
    if (entity == nullptr) return;
    objects.push_back(entity);
}

void Workspace::removeEntity(MathEntity* entity) {
    if (entity == nullptr) return;
    objects.remove(entity);
    delete entity;
}

void Workspace::remove(const int id) {
    removeEntity(byId(id));
}

MathEntity* Workspace::byId(const int id) const {
    for (const auto& obj : objects)
        if (obj->getId() == id)
            return obj;
    throw IndexNotFoundException("use command 1. to see the index of available objects", id);
}

vector<MathEntity*> Workspace::all() const {
    return {objects.begin(), objects.end()};
}

size_t Workspace::size() const noexcept {
    return objects.size();
}

void Workspace::clear() {
    for (auto* obj : objects)
        delete obj;
    objects.clear();
}

void Workspace::sortByName() {
    // Compares getName() directly rather than toString(): toString() begins
    // with "[<ClassLabel> ID=<id>]\n" before the name line, and ids are
    // globally unique, so comparing toString() would effectively sort by
    // class label then id, never reaching the name.
    if (objects.size() <= 1) return;
    objects.sort([](const MathEntity* first, const MathEntity* second) {
        return first->getName() < second->getName();
    });
}

Matrix* Workspace::matrixById(const int id) const {
    return getElementById(ofType<Matrix>(), id);
}

Polynomial* Workspace::polynomialById(const int id) const {
    return getElementById(ofType<Polynomial>(), id);
}

LinearSystem* Workspace::systemById(const int id) const {
    return getElementById(ofType<LinearSystem>(), id);
}

Dataset* Workspace::datasetById(const int id) const {
    return getElementById(ofType<Dataset>(), id);
}

// ---------------------------------------------------------------------------
// Persistence
// ---------------------------------------------------------------------------

void Workspace::loadPolynomials(const string& filePath){
    ifstream file(filePath);
    if(!file.is_open()){
        throw FileException("Could not read from " + filePath);
    }

    // Parse into locals first; only replace the existing polynomials once the
    // whole file has parsed successfully.
    vector<unique_ptr<Polynomial>> loaded;
    while(true){
        auto p = make_unique<Polynomial>();
        if(!(file>>*p))
            break;
        loaded.push_back(std::move(p));
    }
    file.close();

    for (auto* obj : ofType<Polynomial>()) removeEntity(obj);
    for (auto& p : loaded) add(p.release());
}

void Workspace::loadDatasets(const string& filePath){
    ifstream file(filePath);
    if(!file.is_open()){
        throw FileException("Could not read from " + filePath);
    }

    // Parse into locals first; only replace the existing data sets once the
    // whole file has parsed successfully.
    vector<unique_ptr<Dataset>> loaded;
    char choice;
    while(file>>choice){
        char unit;
        if (!(file>>unit))
            throw ParseException("Missing unit of measure for Dataset");
        int n;
        if (!(file>>n) || n<0)
            throw ParseException("Invalid number of elements for Dataset");
        vector<double> temp(static_cast<size_t>(n),0.0);
        for(int i=0;i<n;i++)
            if (!(file>>temp[i]))
                throw ParseException("Insufficient data for Dataset");
        loaded.push_back(make_unique<Dataset>(temp,unit,tolower(choice)=='y'));
    }
    file.close();

    for (auto* obj : ofType<Dataset>()) removeEntity(obj);
    for (auto& d : loaded) add(d.release());
}

void Workspace::loadSystems(const string& filePath){
    ifstream file(filePath);
    if(!file.is_open())
        throw FileException("Could not read from " + filePath);

    // Parse into locals first; only replace the existing systems once the
    // whole file has parsed successfully.
    vector<unique_ptr<LinearSystem>> loaded;
    string name;
    while (getline(file, name)) {
        if (name.find_first_not_of(" \t\r") == string::npos) continue;
        int eqs = 0, vars = 0;
        if (!(file >> eqs >> vars))
            throw ParseException("Missing dimensions for system '" + name + "'");
        file.ignore(numeric_limits<streamsize>::max(), '\n');
        auto sis = make_unique<LinearSystem>(vars, eqs, name);   // unique_ptr: freed automatically if the next line throws
        file >> *sis;
        loaded.push_back(std::move(sis));
    }
    file.close();

    for (auto* obj : ofType<LinearSystem>()) removeEntity(obj);
    for (auto& s : loaded) add(s.release());
}

void Workspace::loadMatrices(const string& filePath){
    ifstream file(filePath);
    if(!file.is_open())
    {
        throw FileException("Could not read from " + filePath);
    }

    // Parse into locals first; only replace the existing matrices once the
    // whole file has parsed successfully.
    vector<unique_ptr<Matrix>> loaded;
    int n;
    while(file>>n){
        int m;
        if (!(file>>m))
            throw ParseException("Missing number of columns for matrix");
        vector<vector<float>> temp(static_cast<size_t>(n), vector<float>(static_cast<size_t>(m), 0.0f));
        for(int i=0;i<n;i++){
            for(int j=0;j<m;j++)
                if (!(file>>temp[i][j]))
                    throw ParseException("Insufficient data for matrix");
        }
        loaded.push_back(make_unique<Matrix>(temp));
    }
    // `while(file>>n)` cannot tell a legitimately empty/exhausted stream apart
    // from unparseable leading/trailing content using its own falsiness alone:
    // both leave the stream in a failed state. Genuine EOF sets failbit AND
    // eofbit; garbage that failed extraction leaves eofbit clear because the
    // offending characters are still sitting in the stream. Without this
    // check, garbage silently looks like "no more matrices" and the commit
    // below replaces the existing matrices with zero of them.
    if (file.fail() && !file.eof())
        throw ParseException("Invalid content in " + filePath +
                             " (expected a number of rows)");
    file.close();

    for (auto* obj : ofType<Matrix>()) removeEntity(obj);
    for (auto& mat : loaded) add(mat.release());
}

// Atomicity here is deliberately PER TYPE, not per directory: each load*
// call parses into locals and commits only on success, so a corrupt file
// leaves its own type untouched — but if polynomials.txt is the broken one, the
// matrices, systems and data sets ahead of it in this list have already been
// replaced and stay replaced. Loading is a user-initiated action reported by a
// thrown exception, and a partial load of well-formed files is more useful than
// discarding three good files because the fourth is damaged. Rolling the whole
// directory back would mean snapshotting and restoring every object of every
// type, ids included, which is a larger change than the guarantee is worth.
void Workspace::loadFromDirectory(const string& directoryName){
    loadMatrices(directoryName+"/matrices.txt");
    loadSystems(directoryName+"/systems.txt");
    loadDatasets(directoryName+"/datasets.txt");
    loadPolynomials(directoryName+"/polynomials.txt");
}

// Every stream here writes at max_digits10, the shortest precision from which
// the exact value can be recovered -- otherwise the stream's default of 6
// significant digits would round-trip a Dataset value like
// 0.123456789012345 into 0.123457, silently changing the user's data on
// save/reload. Matrix, LinearSystem and Polynomial store float and Dataset
// stores double, so the two limits differ and are taken from the type rather
// than hard-coded.
void Workspace::saveToDirectory(const string& directoryName) const {
    constexpr int FLOAT_DIGITS  = numeric_limits<float>::max_digits10;
    constexpr int DOUBLE_DIGITS = numeric_limits<double>::max_digits10;
    if (!fs::exists(directoryName))
        fs::create_directories(directoryName);
    ofstream matrixFile(directoryName+"/matrices.txt");
    if (!matrixFile.is_open()) {
        throw FileException("Could not write to file "+ directoryName+"/matrices.txt");
    }
    matrixFile << setprecision(FLOAT_DIGITS);
    const auto matrices = ofType<Matrix>();
    for (const auto& obj: matrices) {
        const int r = obj->getRows();
        const int c = obj->getColumns();
        matrixFile<<r<<"\n"<<c<<"\n";
        const auto& data = obj->getData();
        for (int row=0;row<r;row++) {
            for (int col=0;col<c;col++) {
                matrixFile<<data[row][col];
                if (col+1<c)
                    matrixFile<<' ';
            }
            matrixFile<<"\n";
        }
    }
    matrixFile.close();
    ofstream systemsFile(directoryName+"/systems.txt");
    if (!systemsFile.is_open()) {
        throw FileException("Could not write to file " + directoryName+"/systems.txt");
    }
    systemsFile << setprecision(FLOAT_DIGITS);
    const auto systems = ofType<LinearSystem>();
    for (const auto& obj:systems) {
        const LinearSystem* s = obj;
        const int varsCount = s->getNrOfVariables();
        const int eqsCount = s->getNrOfEquations();
        systemsFile<<s->getName()<<"\n";
        systemsFile<<eqsCount<<' '<<varsCount<<"\n";
        const Matrix m = s->getMatrix();
        const auto& data = m.getData();
        for (int eq=0;eq<eqsCount;eq++) {
            for (int j=0;j<varsCount;j++) {
                const float coef = data[eq][j];
                if (j>0 && coef>=0)
                    systemsFile<<'+';
                systemsFile<<coef<<'*'<<LinearSystem::symbols[j];
            }
            systemsFile<<'='<<data[eq][varsCount]<<"\n";
        }
    }
    systemsFile.close();

    ofstream dataFile(directoryName+"/datasets.txt");
    if (!dataFile.is_open()) {
        throw FileException("Could not write to file " + directoryName+"/datasets.txt");
    }
    dataFile << setprecision(DOUBLE_DIGITS);

    const auto datasets = ofType<Dataset>();

    for (const auto& obj:datasets) {
        const int n = obj->getSize();
        dataFile<<(obj->getIsSorted()?'y':'n')<<' '<<obj->getUnit()<<' '<<n<<"\n";
        for (int j=0;j<n;j++) {
            dataFile<<(*obj)[j];
            if (j+1<n)
                dataFile<<' ';
        }
        dataFile<<"\n";
    }
    dataFile.close();

    ofstream polyFile(directoryName+"/polynomials.txt");
    if (!polyFile.is_open()) {
        throw FileException("Could not write to file " + directoryName+"/polynomials.txt");
    }
    polyFile << setprecision(FLOAT_DIGITS);

    auto polynomials = ofType<Polynomial>();

    for (const auto& obj: polynomials) {
        obj->printForSave(polyFile);
        polyFile<<"\n";
    }
    polyFile.close();
}

// ---------------------------------------------------------------------------
// Matrices
// ---------------------------------------------------------------------------

// Every operation below returns the result of a single expression rather than
// naming a local and returning it. That is not a style preference: every
// IObject construction consumes an id the user can see in the listings, so the
// NUMBER of objects an operation builds is observable behaviour, and C++17
// guarantees the result is materialised straight into the caller's variable
// only for the prvalue form.
Matrix Workspace::matrixBinaryOp(const int lhsId, const int rhsId, const char op) const {
    const auto matrices = ofType<Matrix>();
    auto [first,second] = getTwoElementsById(matrices,lhsId,rhsId);
    return (op=='+') ? (*first + *second)
        : (op=='-') ? (*first - *second)
            : (op=='*') ? (*first * *second)
                : first->concat(*second);
}

Matrix Workspace::matrixScalarOp(const int id, const float scalar, const char op) const {
    auto* current = matrixById(id);
    return (op=='+') ? (*current + scalar)
        : (op=='-') ? (*current - scalar)
            : (*current * scalar);
}

Matrix Workspace::matrixNegate(const int id) const {
    return -(*matrixById(id));
}

Matrix Workspace::matrixPower(const int id, const int power) const {
    return matrixById(id)->toPower(power);
}

Matrix Workspace::matrixEchelon(const int id) const {
    return matrixById(id)->toEchelonForm();
}

float Workspace::matrixDeterminant(const int id) const {
    return matrixById(id)->determinant();
}

float Workspace::matrixElement(const int id, const size_t index) const {
    return (*matrixById(id))[index];
}

int Workspace::matrixElementCount(const int id) const {
    const Matrix* current = matrixById(id);
    return current->getRows() * current->getColumns();
}

void Workspace::matrixSetElement(const int id, const size_t index, const float value) {
    Matrix* current = matrixById(id);
    const int rows = current->getRows();
    const int cols = current->getColumns();
    if (rows <= 0 || cols <= 0 ||
        index >= static_cast<size_t>(rows) * static_cast<size_t>(cols))
        throw IndexException("You selected an element that doesn't exist in matrix " + std::to_string(id));

    auto data = current->getData();   // copy: every OTHER cell keeps its current value
    data[index / static_cast<size_t>(cols)][index % static_cast<size_t>(cols)] = value;

    // Feed the whole grid back through the existing extraction operator so the
    // commit is atomic and the id/name are never touched -- operator>> only
    // ever assigns matrix.data.
    std::ostringstream os;
    os << std::setprecision(std::numeric_limits<float>::max_digits10);
    for (const auto& row : data)
        for (const float v : row)
            os << v << ' ';
    std::istringstream is(os.str());
    is >> *current;
}

ComparisonResult Workspace::matrixCompare(const int lhsId, const int rhsId) const {
    const auto matrices = ofType<Matrix>();
    auto [first,second] = getTwoElementsById(matrices,lhsId,rhsId);
    ComparisonResult result;
    result.equal          = (*first==*second);
    result.notEqual       = (*first!=*second);
    result.less           = (*first < *second);
    result.lessOrEqual    = (*first<=*second);
    result.greater        = (*first > *second);
    result.greaterOrEqual = (*first>=*second);
    return result;
}

Matrix Workspace::matrixStep(const int id, const char op, const bool prefix) {
    Matrix* current = matrixById(id);
    if (op=='+')
        return prefix ? ++(*current) : (*current)++;
    return prefix ? --(*current) : (*current)--;
}

// ---------------------------------------------------------------------------
// Polynomials
// ---------------------------------------------------------------------------

Polynomial Workspace::polynomialBinaryOp(const int lhsId, const int rhsId, const char op) const {
    const auto polynomials = ofType<Polynomial>();
    auto [first,second] = getTwoElementsById(polynomials,lhsId,rhsId);
    // Default-construct then assign, deliberately: this builds one more
    // Polynomial than `return *first + *second;` would, and every
    // construction moves the global id counter shown in the listings.
    Polynomial result;
    if (op=='+')
        result = *first + *second;
    else if (op=='-')
        result = *first - *second;
    else
        result = *first * *second;
    return result;
}

Polynomial Workspace::polynomialNegate(const int id) const {
    return -(*polynomialById(id));
}

Polynomial Workspace::polynomialDerivative(const int id) const {
    return polynomialById(id)->derivative();
}

Polynomial Workspace::polynomialIntegral(const int id) const {
    return polynomialById(id)->integral();
}

float Workspace::polynomialAt(const int id, const float x) const {
    return polynomialById(id)->calculateValueInPoint(x);
}

// The argument order is calculateDefinedIntegral(a = upper bound, b = lower
// bound); it is deliberately not "fixed" here.
float Workspace::polynomialDefiniteIntegral(const int id, const float upper, const float lower) const {
    return polynomialById(id)->calculateDefinedIntegral(upper, lower);
}

float Workspace::polynomialCoefficient(const int id, const size_t index) const {
    return (*polynomialById(id))[index];
}

size_t Workspace::polynomialMaxDegree(const int id) const {
    return polynomialById(id)->getMaxDegree();
}

void Workspace::polynomialSetCoefficient(const int id, const size_t index, const float value) {
    Polynomial* current = polynomialById(id);
    if (index > current->getMaxDegree())
        throw IndexException("There is no term at power " + std::to_string(index));

    auto coefficients = current->getCoefficients();   // copy
    coefficients[index] = value;

    const string savedName = current->getName();
    *current = Polynomial(coefficients.data(), current->getMaxDegree());
    current->setName(savedName);
}

std::string Workspace::polynomialSaveFormat(const int id) const {
    ostringstream os;
    polynomialById(id)->printForSave(os);
    return os.str();
}

ComparisonResult Workspace::polynomialCompare(const int lhsId, const int rhsId) const {
    const auto polynomials = ofType<Polynomial>();
    auto [first,second] = getTwoElementsById(polynomials,lhsId,rhsId);
    ComparisonResult result;
    result.equal          = (*first==*second);
    result.notEqual       = (*first!=*second);
    result.less           = (*first < *second);
    result.lessOrEqual    = (*first<=*second);
    result.greater        = (*first > *second);
    result.greaterOrEqual = (*first>=*second);
    return result;
}

Polynomial Workspace::polynomialStep(const int id, const char op, const bool prefix) {
    Polynomial* current = polynomialById(id);
    if (op=='+')
        return prefix ? ++(*current) : (*current)++;
    return prefix ? --(*current) : (*current)--;
}

// ---------------------------------------------------------------------------
// Linear systems
// ---------------------------------------------------------------------------

SolutionReport Workspace::solveSystem(const int id) const {
    return systemById(id)->solveSystem();
}

LinearSystem Workspace::systemBinaryOp(const int lhsId, const int rhsId, const char op) const {
    const auto systems = ofType<LinearSystem>();
    auto [first,second] = getTwoElementsById(systems,lhsId,rhsId);
    return (op=='+') ? (*first + *second) : (*first - *second);
}

float Workspace::systemFreeTerm(const int id, const size_t equationIndex) const {
    return (*systemById(id))[equationIndex];
}

// LinearSystem only defines == and <, so the other four are derived here
// exactly as the menu derived them.
ComparisonResult Workspace::systemCompare(const int lhsId, const int rhsId) const {
    const auto systems = ofType<LinearSystem>();
    auto [first,second] = getTwoElementsById(systems,lhsId,rhsId);
    const bool eq = (*first == *second);
    const bool lt = (*first < *second);
    const bool gt = (*second < *first);
    ComparisonResult result;
    result.equal          = eq;
    result.notEqual       = !eq;
    result.less           = lt;
    result.lessOrEqual    = lt || eq;
    result.greater        = gt;
    result.greaterOrEqual = gt || eq;
    return result;
}

LinearSystem Workspace::systemStep(const int id, const char op, const bool prefix) {
    LinearSystem* current = systemById(id);
    if (op=='+')
        return prefix ? ++(*current) : (*current)++;
    return prefix ? --(*current) : (*current)--;
}

// ---------------------------------------------------------------------------
// Data sets
// ---------------------------------------------------------------------------

DatasetSummary Workspace::summariseDataset(const int id) const {
    return datasetById(id)->summarise();
}

Dataset Workspace::datasetBinaryOp(const int lhsId, const int rhsId, const char op) const {
    const auto datasets = ofType<Dataset>();
    auto [first,second] = getTwoElementsById(datasets,lhsId,rhsId);
    return (op=='+') ? (*first + *second)
        : (op=='-') ? (*first - *second)
            : (*first * *second);
}

void Workspace::datasetAddValue(const int id, const double value) {
    datasetById(id)->addValue(value);
}

void Workspace::datasetSort(const int id) {
    datasetById(id)->sortData();
}

void Workspace::datasetRemoveOutliers(const int id) {
    datasetById(id)->deleteOutliners();
}

bool Workspace::datasetContains(const int id, const double value) const {
    return datasetById(id)->inData(value);
}

double Workspace::datasetElement(const int id, const size_t index) const {
    return (*datasetById(id))[index];
}

void Workspace::datasetExportToCSV(const int id, const string& fileName) const {
    datasetById(id)->exportToCSV(fileName);
}

Dataset Workspace::datasetImportFromCSV(const string& fileName) const {
    Dataset localDataset;
    localDataset.importFromCSV(fileName);
    return localDataset;
}

Dataset Workspace::datasetStep(const int id, const char op, const bool prefix) {
    Dataset* current = datasetById(id);
    if (op=='+')
        return prefix ? ++(*current) : (*current)++;
    return prefix ? --(*current) : (*current)--;
}

ComparisonResult Workspace::datasetCompare(const int lhsId, const int rhsId) const {
    const auto datasets = ofType<Dataset>();
    auto [first,second] = getTwoElementsById(datasets,lhsId,rhsId);
    ComparisonResult result;
    result.equal          = (*first==*second);
    result.notEqual       = (*first!=*second);
    result.less           = (*first < *second);
    result.lessOrEqual    = (*first<=*second);
    result.greater        = (*first > *second);
    result.greaterOrEqual = (*first>=*second);
    return result;
}

// ---------------------------------------------------------------------------
// Polymorphic entries
// ---------------------------------------------------------------------------

MathEntity* Workspace::cloneEntity(const int id) {
    auto* cloned = byId(id)->clone();
    add(cloned);
    return cloned;
}

MathEntity* Workspace::cloneAlgebraEntity(const int id) {
    auto* cloned = getElementById(ofType<AlgebraEntity>(), id)->clone();
    add(cloned);
    return cloned;
}

MathEntity* Workspace::cloneDataEntity(const int id) {
    auto* cloned = getElementById(ofType<DataEntity>(), id)->clone();
    add(cloned);
    return cloned;
}

std::string Workspace::solveEntity(const int id) {
    return getElementById(ofType<AlgebraEntity>(), id)->solve();
}

std::string Workspace::summariseEntity(const int id) const {
    return getElementById(ofType<DataEntity>(), id)->summaryText();
}
