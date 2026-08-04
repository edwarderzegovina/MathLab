#ifndef DATASET_HPP
#define DATASET_HPP

#include <istream>
#include <ostream>
#include <fstream>
#include <string>
#include <cmath>
#include <algorithm>
#include <vector>
#include "DataEntity.h"
#include "DatasetSummary.h"

using namespace std;

class Dataset:public DataEntity {
    vector<double> data;
    bool isSorted;
    static int objectCount;
    int binarySearch(int left,int right,double value) const;
public:
    MathEntity* clone() const override;
    int getSize() const;
    char getUnit() const;
    bool getIsSorted() const;
    void exportToCSV(const string& fileName) const;
    void importFromCSV(const string& fileName);
    DatasetSummary summarise() const;   // the statistics, structured
    string summaryText() const override;
    double getMin()const;
    double getMax()const;
    void deleteOutliners(); // numbers further than 3 times the standard deviation.
    double standardDeviation() const;
    void addValue(double value);
    double getMedian() const;
    void sortData();
    double averageValue()const;
    bool inData(double toFind) const; // to be changed to binary search on sorted data
    Dataset();
    Dataset(const double* data_,int size,char unit='N',bool isSorted=false);
    Dataset(const vector<double>& data_,char unit='N',bool isSorted=false);
    explicit Dataset(int size,char unit='N',bool isSorted = false);
    Dataset(const Dataset& alt);
    ~Dataset() noexcept override;
    Dataset& operator=(const Dataset& alt);
    friend istream& operator>>(istream& in,Dataset& dataset);
    friend ostream& operator<<(ostream& out,const Dataset& dataset);
    double operator[](size_t index) const;
    //increment operators for every bit of data in the array
    Dataset& operator++();
    Dataset operator++(int);
    Dataset& operator--();
    Dataset operator--(int);
    //define the following math operators
    // + --> union
    // - --> difference
    // * --> intersection
    Dataset operator+(const Dataset& alt) const;
    Dataset operator-(const Dataset& alt) const;
    Dataset operator*(const Dataset& alt) const;
    //we "sort" by averageValue
    bool operator==(const Dataset& alt) const;
    bool operator!=(const Dataset& alt) const;
    bool operator<(const Dataset& alt) const;
    bool operator<=(const Dataset& alt) const;
    bool operator>(const Dataset& alt) const;
    bool operator>=(const Dataset& alt) const;

    string toString() const override;
};

#endif
