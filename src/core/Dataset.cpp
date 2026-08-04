#include "Dataset.h"
#include <sstream>
#include <iomanip>
#include <limits>
#include "MathLabException.h"
#include "Logger.h"


int Dataset::objectCount = 0;

int Dataset::getSize() const {
    return static_cast<int>(data.size());
}

char Dataset::getUnit() const {
    return unit;
}

bool Dataset::getIsSorted() const {
    return isSorted;
}

//return 1-Succeeded, return 0-Failed

//Only works for files with data in the form index,(separator)value
void Dataset::importFromCSV(const string& fileName) {
    ifstream file(fileName);
    if (!file.is_open())
        throw FileException("Could not import from " + fileName);

    std::string header;
    if (!std::getline(file, header))
        throw FileException("Empty CSV file: " + fileName);
    const size_t up = header.find("unit=");
    char parsedUnit = unit;
    if (up != std::string::npos && up + 5 < header.size()) parsedUnit = header[up + 5];

    std::vector<double> loaded;
    std::string line;
    int lineNo = 1;
    while (std::getline(file, line)) {
        lineNo++;
        if (line.empty()) continue;
        const size_t comma = line.find(',');
        if (comma == std::string::npos)
            throw ParseException("Line " + std::to_string(lineNo) + " does not contain ','");
        try { loaded.push_back(std::stod(line.substr(comma + 1))); }
        catch (const std::exception&) {
            throw ParseException("Invalid value on line " + std::to_string(lineNo));
        }
    }
    data = loaded;                       // commit only after a clean parse
    unit = parsedUnit;
    isSorted = std::is_sorted(data.begin(), data.end());
}

void Dataset::exportToCSV(const string& fileName) const {
    ofstream file(fileName);
    if (!file.is_open())
        throw FileException("Could not export to " + fileName);
    file << "index,value,unit=" << unit << "\n";
    file << std::setprecision(std::numeric_limits<double>::max_digits10);
    for (size_t i=0;i<data.size();i++)
        file<<i+1<<","<<data[i]<<"\n";
    file.close();
}

// The statistics, with nothing printed. getMin/getMax/getMedian throw on an
// empty set, so on an empty set they are simply not called and `empty` says so
// -- which is what printSummary() did by returning early after "(empty set)".
DatasetSummary Dataset::summarise() const {
    DatasetSummary summary;
    summary.name = name;
    summary.unit = unit;
    summary.sorted = isSorted;
    summary.size = data.size();
    summary.empty = data.empty();
    if (data.empty())
        return summary;
    summary.min = getMin();
    summary.max = getMax();
    summary.mean = averageValue();
    summary.median = getMedian();
    summary.stddev = standardDeviation();
    return summary;
}

string Dataset::summaryText() const {
    return summarise().toText();
}

double Dataset::getMin() const {
    if (data.empty())
        throw EmptySetException("You have no data in the dataset");
    if (isSorted || data.size()==1)
        return data.front();
    double minim = data.front();
    for (size_t i=1;i<data.size();i++)
        minim = minim>data[i]?data[i]:minim;
    return minim;
}

double Dataset::getMax() const {
    if (data.empty())
        throw EmptySetException("You have no data in the dataset");
    if (isSorted || data.size()==1)
        return data.back();
    double maxim = data.front();
    for (size_t i=1;i<data.size();i++)
        maxim = maxim<data[i]?data[i]:maxim;
    return maxim;
}

void Dataset::deleteOutliners() {
    const double offset = standardDeviation()*3;
    const double media = averageValue();
    if (data.empty())
        return;
    const auto originalSize = data.size();
    data.erase(remove_if(data.begin(),data.end(),
        [&](const double value) {
            return fabs(value - media) > offset;
        }),data.end());
    if (data.size() != originalSize)
        isSorted = false;
}

void Dataset::addValue(const double value) {
    isSorted = false;
    data.push_back(value);
}

double Dataset::standardDeviation() const {
    const double media = averageValue();
    double distances=0;
    for (double i : data)
        distances +=(i-media)*(i-media);
    return data.empty() ? 0.0 : sqrt(distances/static_cast<double>(data.size()));
}

double Dataset::getMedian() const {
    if (data.empty()) throw EmptySetException("You have no data in the dataset");
    std::vector<double> tmp(data);
    if (!isSorted) std::sort(tmp.begin(), tmp.end());
    const size_t n = tmp.size();
    return (n % 2 == 1) ? tmp[n / 2]
                        : (tmp[n / 2 - 1] + tmp[n / 2]) / 2.0;
}

int Dataset::binarySearch(int left, int right, const double value) const {
    while (left <= right) {
        const int mid = left + (right-left)/2;
        if (fabs(data[mid] - value) < 1e-6f)
            return mid;
        if (data[mid]<value)
            left = mid+1;
        else
            right = mid-1;
    }
    return -1;
}

void Dataset::sortData() {
    sort(data.begin(),data.end());
    isSorted = true;
}

bool Dataset::operator>=(const Dataset &alt) const {
    return *this==alt || *this>alt;
}

bool Dataset::operator>(const Dataset& alt) const {
    return alt<*this;
}

bool Dataset::operator<(const Dataset& alt) const {
    return averageValue()<alt.averageValue();
}

bool Dataset::operator<=(const Dataset &alt) const {
    return *this<alt || *this==alt;
}

bool Dataset::operator==(const Dataset &alt) const {
    return averageValue() == alt.averageValue();
}
bool Dataset::operator!=(const Dataset& alt) const {
    return !(*this==alt);
}
double Dataset::averageValue() const {
    double suma=0;
    for (double i : data)
        suma+=i;
    return data.empty() ? 0.0 : suma/static_cast<double>(data.size());
}

Dataset Dataset::operator*(const Dataset &alt) const {
    std::vector<double> temp;
    for (const double v : data)
        if (alt.inData(v) && std::find(temp.begin(), temp.end(), v) == temp.end())
            temp.push_back(v);
    return Dataset(temp, unit, false);
}

Dataset Dataset::operator+(const Dataset& alt) const {
    std::vector<double> temp;
    for (const double v : data)
        if (std::find(temp.begin(), temp.end(), v) == temp.end())
            temp.push_back(v);
    for (const double v : alt.data)
        if (std::find(temp.begin(), temp.end(), v) == temp.end())
            temp.push_back(v);
    return Dataset(temp, unit, false);      // never claim sorted
}

Dataset Dataset::operator-(const Dataset& alt) const {
    std::vector<double> temp;
    for (const double v : data)
        if (!alt.inData(v) && std::find(temp.begin(), temp.end(), v) == temp.end())
            temp.push_back(v);
    return Dataset(temp, unit, false);      // no {0.0} fabrication
}

bool Dataset::inData(const double toFind) const{
    static constexpr double TOL = 1e-6;
    if (isSorted) return binarySearch(0, static_cast<int>(data.size()) - 1, toFind) >= 0;
    for (const double v : data) if (fabs(v - toFind) < TOL) return true;
    return false;
}

Dataset Dataset::operator--(int) {
    Dataset coppy(*this);
    --*this;
    return coppy;
}

Dataset& Dataset::operator--() {
    for (double & i : data)
        i--;
    return *this;
}

Dataset Dataset::operator++(int) {
    Dataset coppy(*this);
    ++*this;
    return coppy;
}

Dataset& Dataset::operator++() {
    for (double & i : data)
        i++;
    return *this;
}

double Dataset::operator[](const size_t index) const{
    if (index>= data.size())
        throw IndexException("You only have " + to_string(data.size()) + " elements!");
    return data[index];
}

ostream& operator<<(ostream& out,const Dataset& dataset) {
    out<<"Your data analyzer contains "<<dataset.data.size()<<" data points:\n";
    for (const double i : dataset.data)
        out<<i<<' ';
    out<<endl;
    return out;
}

// Reads into locals and commits in one step, exactly as importFromCSV does,
// so a read that throws partway through (e.g. a truncated record) leaves the
// dataset exactly as it was rather than partially overwritten.
istream& operator>>(istream& in, Dataset& dataset) {
    char choice = 'n';
    char parsedUnit = dataset.unit;
    in >> choice;
    in >> parsedUnit;
    int count = 0;
    if (!(in >> count) || count < 0)
        throw InputException("The number of elements must be a positive integer");
    std::vector<double> loaded(static_cast<size_t>(count), 0.0);
    for (int i = 0; i < count; i++)
        if (!(in >> loaded[static_cast<size_t>(i)]))
            throw ParseException("Insufficient data in stream");
    dataset.data = std::move(loaded);
    dataset.unit = parsedUnit;
    dataset.isSorted = std::is_sorted(dataset.data.begin(), dataset.data.end());
    (void)choice;
    return in;
}

Dataset &Dataset::operator=(const Dataset &alt) {
    if (this == &alt) return *this;
    DataEntity::operator=(alt);
    isSorted = alt.isSorted;
    data = alt.data;
    return *this;
}

Dataset::Dataset(const Dataset &alt):DataEntity(alt),data(alt.data),isSorted(alt.isSorted) {
    objectCount++;
    Logger::getInstance().log("Created by copy Dataset: " + name, "Dataset");
}

//empty dataset.
Dataset::Dataset(const int size,const char unit,const bool isSorted):DataEntity("Dataset"+to_string(objectCount),unit),data(static_cast<size_t>(size),0.0),isSorted(isSorted) {
    this->isSorted = std::is_sorted(data.begin(), data.end());
    Logger::getInstance().log("Created Dataset: " + name, "Dataset");
    objectCount++;
}

Dataset::Dataset(const double *data_,const int size,const char unit,const bool isSorted):DataEntity("Dataset"+to_string(objectCount),unit),data(data_,data_+size),isSorted(isSorted) {
    this->isSorted = std::is_sorted(data.begin(), data.end());
    Logger::getInstance().log("Created Dataset: " + name, "Dataset");
    objectCount++;
}

Dataset::Dataset(const vector<double>& data_,const char unit,const bool isSorted):DataEntity("Dataset"+to_string(objectCount),unit),data(data_),isSorted(isSorted) {
    this->isSorted = std::is_sorted(data.begin(), data.end());
    Logger::getInstance().log("Created Dataset: " + name, "Dataset");
    objectCount++;
}

Dataset::Dataset():DataEntity("Dataset"+to_string(objectCount)),data(),isSorted(false) {
    Logger::getInstance().log("Created Dataset: " + name, "Dataset");
    objectCount++;
}

Dataset::~Dataset()noexcept {
    objectCount--;
    Logger::getInstance().log("Destroyed Dataset: " + name, "Dataset");
}

MathEntity* Dataset::clone() const {
    return new Dataset(*this);
}


string Dataset::toString() const {
    ostringstream os;

    os<<"[Dataset ID="<<getId()<<"]\n";
    os<<"Name = "<<name<<endl;
    os<<data.size()<<" elements, Sorted: "<<(isSorted ? "YES" : "NO") << std::endl;
    os<<*this;

    return os.str();
}
