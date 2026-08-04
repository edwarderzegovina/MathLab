#ifndef POLYNOMIAL_HPP
#define POLYNOMIAL_HPP

#include <istream>
#include <ostream>
#include <cmath>
#include <string>
#include <stdexcept>
#include <algorithm>
#include <vector>
#include "AlgebraEntity.h"

using namespace std;

class Polynomial:public AlgebraEntity{
    std::vector<float> coefficients;
    size_t maxDegree;
    int derivativeOrder;
    int integralOrder;
    void trimPolynom();
    Polynomial(const float* coefficients_, size_t maxDegree_, int derivativeOrder_,int integralOrder_=0);
    static int objectCount;
public:
    MathEntity* clone() const override;
    string solve() override;
    static size_t getObjectCount();
    size_t getMaxDegree() const;   //done
    const std::vector<float>& getCoefficients() const; //done
    Polynomial(); //done
    Polynomial(const float* coefficients_, size_t maxDegree_); //done
    explicit Polynomial(const float* coefficients_); //the array must end in INFINITY for this constructor to make sense
    explicit Polynomial (size_t maxDegree_); //done
    ~Polynomial() noexcept final; //done
    Polynomial(const Polynomial& alt); //done
    Polynomial& operator=(const Polynomial& alt); //done
    friend ostream& operator<<(ostream& os, const Polynomial& polynomial);  //done
    friend istream& operator>>(istream& is, Polynomial& polynomial);        //done
    Polynomial operator++(int); //done
    Polynomial& operator++();    //done
    Polynomial operator--(int);    //done
    Polynomial& operator--();    //done
    Polynomial operator-() const; //done
    Polynomial operator+(const Polynomial& alt) const;  //done
    Polynomial operator-(const Polynomial& alt) const; //done
    Polynomial operator*(const Polynomial& alt) const; //done
    Polynomial derivative() const; //done
    Polynomial integral() const; //done
    float calculateValueInPoint(float x) const; // done
    float calculateDefinedIntegral(float a,float b) const; //a = upperBound , b = lower bound
    float calculate(float value) const;
    void printForSave(ostream& os) const;
    float operator[](size_t index) const; //returns the index-th coefficient
    bool operator==(const Polynomial& alt) const;
    bool operator!=(const Polynomial& alt) const;
    bool operator<(const Polynomial& alt) const;
    bool operator>(const Polynomial& alt) const;
    bool operator<=(const Polynomial& alt) const;
    bool operator>=(const Polynomial& alt) const;
    string toString() const override;
};

#endif
