#ifndef PROIECT02_IMATHFACTORY_H
#define PROIECT02_IMATHFACTORY_H
#include "MathEntity.h"
#include "Polynomial.h"
#include "Matrix.h"
#include "LinearSystem.h"
#include "DataEntity.h"

#include "Dataset.h"
#include "ConsoleIO.h"
#include <iostream>

// The factories read entities FROM THE TERMINAL, so they live with the
// front-end: readEntity() is a console operation, not a core one. Everything
// they build is an ordinary core object.

class IMathFactory {
public:
    virtual MathEntity* readEntity () const = 0;
    virtual MathEntity* createIdentity () const = 0;
    virtual ~IMathFactory() noexcept = default;
};

class MatrixFactory: public IMathFactory {
public:
    MathEntity* readEntity() const override {
        Matrix mat = ConsoleIO::readMatrix();
        return new Matrix(mat);
    }
    MathEntity* createIdentity()const override {
        return new Matrix(3);
    }
    ~MatrixFactory() noexcept override = default;
};

class PolynomialFactory: public IMathFactory {
public:
    MathEntity* readEntity() const override {
        Polynomial p;
        std::cin>>p;
        return new Polynomial(p);
    }
    MathEntity* createIdentity() const override {
        return new Polynomial();
    }
    ~PolynomialFactory() noexcept override = default;
};

class DataEntityFactory: public IMathFactory {
public:
    MathEntity* readEntity() const override {
        Dataset d;
        std::cin>>d;
        return new Dataset(d);
    }
    MathEntity* createIdentity() const override {
        return new Dataset();
    }
    ~DataEntityFactory() noexcept override = default;
};

class LinearSystemFactory: public IMathFactory {
public:
    MathEntity* readEntity() const override {
        LinearSystem s = ConsoleIO::readSystem();
        return new LinearSystem(s);
    }
    MathEntity* createIdentity() const override {
        return new LinearSystem(3);
    }
    ~LinearSystemFactory() noexcept override = default;
};


#endif //PROIECT02_IMATHFACTORY_H
