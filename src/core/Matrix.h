#ifndef MATRIX_HPP
#define MATRIX_HPP

#include <istream>
#include <ostream>
#include <cmath>
#include <algorithm>
#include <vector>
#include "AlgebraEntity.h"
#include <string>

using namespace std;

class Matrix:public AlgebraEntity {
    vector<vector<float>> data;
    int rows,columns;
    static constexpr int MAX_SIZE = 10;
    static int objectCount;

public:
    // Public so a front-end can reject a shape BEFORE constructing anything:
    // the (rows, columns) constructor validates from its mem-init list, i.e.
    // after the base AlgebraEntity has already been built and has consumed an
    // id. Since ids are visible to the user, callers should check the shape
    // first and this needs to stay reachable from outside.
    static void validateDimensions(int rows_, int columns_);
    static int getObjectCount();
    MathEntity *clone() const override;
    string solve() override;
    const vector<vector<float>>& getData() const;
    int getRows() const;
    int getColumns() const;
    Matrix(const float* data_1D,int rows_,int columns_); //Creates class from unidimensional array
    Matrix(float** data_2D,int rows_,int columns_);      //Creates class from bidimensional array
    Matrix(const vector<vector<float>>& data_2D);        //Creates class from STL container
    explicit Matrix(int size);                           //Creates I matrix (a[i][i]= 1, i=1,size)
    Matrix();
    Matrix(int rows_,int columns_);
    Matrix(const Matrix& alt);                           //Coppy constructor
    Matrix(Matrix&& alt) noexcept;                        //Move constructor
    ~Matrix() noexcept override;                                           //Destructor
    Matrix& operator=(const Matrix& alt);                //assign operator
    Matrix& operator=(Matrix&& alt) noexcept;             //move assign operator
    friend ostream& operator<<(ostream& out,const Matrix& matrix);
    friend istream& operator>>(istream& in, Matrix& matrix);
    Matrix& operator++();   //increments by 1 and returns the changed matrix
    Matrix operator++(int); //increments by 1 and returns the previous state
    Matrix operator--(int); //decrements by 1 and returns the previous state
    Matrix& operator--(); //decrements by 1 and returns the changed matrix
    Matrix operator-() const;   // -Matrix
    friend Matrix operator+(float scalar,const Matrix& matrix); //scalar + Matrix
    friend Matrix operator-(float scalar,const Matrix& matrix); //scalar - Matrix
    Matrix operator+(float scalar)const;    //Matrix + scalar
    Matrix operator-(float scalar)const;    //Matrix - scalar
    Matrix operator*(float scalar) const;   //multiplication by scalar
    friend Matrix operator*(float scalar,const Matrix& matrix); //left multiplication by scalar
    Matrix operator+(const Matrix& alt) const;    //Addition between matrices
    Matrix operator-(const Matrix& alt) const;    //Subtraction between matrices
    Matrix operator*(const Matrix& alt) const;    //Multiplication between matrices
    float determinant()const;   //determinant (zeroing below the main diagonal via multiplications and subtractions to avoid divisions); determinant is the product of the main diagonal / multiplication coefficient
    //comparison operators --> number of elements of a matrix
    bool operator==(const Matrix& alt) const;
    bool operator!=(const Matrix& alt) const;
    bool operator<(const Matrix& alt) const;
    bool operator>(const Matrix& alt) const;
    bool operator<=(const Matrix& alt) const;
    bool operator>=(const Matrix& alt) const;
    float operator[](size_t index) const; //returns the index-th element
    Matrix toPower(int power)const;       //returns the matrix to the power->power;
    Matrix concat(const Matrix& matrix) const;  //concatenates 2 Matrices of same nr of rows
    Matrix toEchelonForm(bool augmented = false) const;   //calculates the echelon form of a matrix; augmented=true skips the last (free-term) column
    string toString() const override;
};

#endif
