#include "Polynomial.h"
#include <sstream>
#include "MathLabException.h"
#include "Logger.h"

int Polynomial::objectCount =0;

//throw logic_error for invalid index
float Polynomial::operator[](const size_t index) const {
    if (index>maxDegree)
        throw IndexException("There is no term at power " + to_string(index));
    return coefficients[index];
}

float Polynomial::calculate(const float value) const {
    return calculateValueInPoint(value);
}

bool Polynomial::operator>=(const Polynomial& alt) const {
    return (*this==alt)||(*this>alt);
}
bool Polynomial::operator>(const Polynomial& alt)const {
    return alt<*this;
}
bool Polynomial::operator<=(const Polynomial& alt) const {
    return (*this==alt)||(*this<alt);
}
bool Polynomial::operator<(const Polynomial& alt) const {
    const size_t hi = std::max(maxDegree, alt.maxDegree);
    for (size_t k = hi + 1; k-- > 0; ) {
        const float mine  = (k < coefficients.size())     ? coefficients[k]     : 0.0f;
        const float other = (k < alt.coefficients.size()) ? alt.coefficients[k] : 0.0f;
        if (mine < other) return true;
        if (mine > other) return false;
    }
    return false;
}
bool Polynomial::operator!=(const Polynomial& alt) const {
    return !(*this==alt);
}
bool Polynomial::operator==(const Polynomial& alt) const {
    if (maxDegree != alt.maxDegree)
        return false;
    for (size_t i=0;i<=maxDegree;i++)
        if (coefficients[i] != alt.coefficients[i])
            return false;
    return true;
}

float Polynomial::calculateDefinedIntegral(const float a,const float b) const {
    const Polynomial integral_ = this->integral();
    return integral_.calculateValueInPoint(a)-integral_.calculateValueInPoint(b);
}

float Polynomial::calculateValueInPoint(const float x) const{
    float sum = 0;
    if (coefficients.empty())
        throw EmptySetException("The polynomial is empty");
    for(size_t i=0;i<=maxDegree;i++)
        sum = sum+coefficients[i]*powf(x,static_cast<float>(i));
    return sum;
}

Polynomial Polynomial::integral() const{
    if(maxDegree == 0 && coefficients[0]==0){
        std::vector<float> temp(1,0.0f);
        Polynomial returned(temp.data(),0,derivativeOrder,integralOrder+1);
        return returned;
    }
    std::vector<float> temp(maxDegree + 2, 0.0f);
    temp[0]=0;
    for(size_t i=1;i<=maxDegree+1;i++)
        temp[i]=coefficients[i-1]/static_cast<float>(i);
    Polynomial returned(temp.data(),maxDegree+1,derivativeOrder,integralOrder+1);
    return returned;
}

Polynomial Polynomial::derivative() const{
    if(maxDegree == 0){
        std::vector<float> temp(1,0.0f);
        Polynomial returned(temp.data(),0,derivativeOrder+1,integralOrder-1);
        return returned;
    }
    std::vector<float> temp(maxDegree,0.0f);
    for(size_t i = 0;i<maxDegree;i++)
        temp[i]= coefficients[i+1]*(static_cast<float>(i+1));
    Polynomial returned(temp.data(),maxDegree-1,derivativeOrder+1,integralOrder-1);
    return returned;
}

Polynomial Polynomial::operator*(const Polynomial& alt) const{
    const size_t size = maxDegree+ alt.maxDegree;
    std::vector<float> temp(size+1,0.0f);
    for(size_t i=0;i<=maxDegree;i++)
        for(size_t j=0;j<=alt.maxDegree;j++)
            temp[i+j] += (coefficients[i]*alt.coefficients[j]);
    Polynomial returned(temp.data(),size);
    returned.trimPolynom();
    return returned;
}

Polynomial Polynomial::operator-(const Polynomial& alt) const{
    return *this+(-alt);
}

Polynomial Polynomial::operator-() const{
    std::vector<float> temp(maxDegree+1,0.0f);
    for(size_t i =0;i<=maxDegree;i++)
        temp[i]=-coefficients[i];
    Polynomial returned(temp.data(),maxDegree);
    return returned;
}

Polynomial Polynomial::operator+(const Polynomial& alt) const{
    if (this->coefficients.empty()) return alt;
    if (alt.coefficients.empty())return *this;
    size_t i = 0;
    const auto size = static_cast<size_t>(max(maxDegree,alt.maxDegree));
    std::vector<float> temp(size+1,0.0f);
    while(i<=maxDegree && i<=alt.maxDegree){
        temp[i] = coefficients[i] + alt.coefficients[i];
        i++;
    }
    while(i<=maxDegree){
        temp[i] = coefficients[i];
        i++;
    }
    while(i<=alt.maxDegree){
        temp[i] = alt.coefficients[i];
        i++;
    }
    Polynomial returned(temp.data(),size);
    returned.trimPolynom();
    return returned;
}

Polynomial Polynomial::operator--(int) {
    Polynomial coppy(*this);
    coefficients[0]--;
    return coppy;
}

Polynomial& Polynomial::operator--(){
    coefficients[0]--;
    return *this;
}

Polynomial Polynomial::operator++(int){
    Polynomial coppy(*this);
    coefficients[0]++;
    return coppy;
}

Polynomial& Polynomial::operator++(){
    coefficients[0]++;
    return *this;
}

istream& operator>>(istream& is, Polynomial& polynomial) {
    string input;
    if (!(is >> input)) return is;                       // no fabricated P(X)=1 on failed read

    static constexpr int MAX_POWER = 255;
    vector<float> coefTable(MAX_POWER + 1, 0.0f);
    int highest = 0;

    size_t i = 0;
    const size_t n = input.size();
    while (i < n) {
        const size_t start = i;

        float sign = 1.0f;
        if (input[i] == '+') { i++; }
        else if (input[i] == '-') { sign = -1.0f; i++; }

        // Coefficient: parsed by hand so that 'x'/'X' can never be swallowed
        // as a hex-float prefix.
        float coefficient = 1.0f;
        bool hasCoefficient = false;
        size_t numStart = i;
        while (i < n && (isdigit(static_cast<unsigned char>(input[i])) || input[i] == '.'))
            i++;
        if (i > numStart) {
            // Optional scientific-notation exponent suffix (e/E, optional
            // sign, one or more digits) so printForSave's ostream-formatted
            // large/small coefficients (e.g. "1e+30") round-trip. Only
            // consumed when digits actually follow the marker/sign — a bare
            // "2e" or "2e+" must NOT be silently swallowed, so it is left for
            // the next iteration to reject.
            if (i < n && (input[i] == 'e' || input[i] == 'E')) {
                size_t expLookahead = i + 1;
                if (expLookahead < n && (input[expLookahead] == '+' || input[expLookahead] == '-'))
                    expLookahead++;
                if (expLookahead < n && isdigit(static_cast<unsigned char>(input[expLookahead]))) {
                    i = expLookahead;
                    while (i < n && isdigit(static_cast<unsigned char>(input[i]))) i++;
                }
            }
            // stof reports how far it got; without checking that, "1.2.3*x^2"
            // would silently yield 1.2 instead of being rejected.
            {
                const string literal = input.substr(numStart, i - numStart);
                size_t consumed = 0;
                try { coefficient = stof(literal, &consumed); }
                catch (const std::exception&) {
                    throw ParseException("Invalid coefficient in '" + input + "'");
                }
                if (consumed != literal.size())
                    throw ParseException("Invalid coefficient in '" + input + "'");
            }
            hasCoefficient = true;
        }

        int power = 0;
        if (i < n && input[i] == '*') i++;

        if (i < n && (input[i] == 'x' || input[i] == 'X')) {
            // '*' between a coefficient and the variable is optional: the
            // grammar is unambiguous (digits followed by x is always
            // coefficient-then-variable), and "3x^2" is exactly how a real
            // user writes it. The save format is unaffected: printForSave
            // always emits the explicit '*'.
            i++;
            power = 1;
            if (i < n && input[i] == '^') {
                i++;
                const size_t powStart = i;
                if (i < n && (input[i] == '+' || input[i] == '-')) i++;
                while (i < n && isdigit(static_cast<unsigned char>(input[i]))) i++;
                if (i == powStart)
                    throw ParseException("Missing exponent in '" + input + "'");
                try { power = stoi(input.substr(powStart, i - powStart)); }
                catch (const std::exception&) {
                    // Overflows int (e.g. "1x^99999999999") instead of the
                    // MAX_POWER check below ever running; must surface as the
                    // same ParseException as every other error path, not leak
                    // std::out_of_range with a raw library message.
                    throw ParseException("Invalid or too large exponent in '" + input + "'");
                }
            }
        } else if (!hasCoefficient) {
            // Nothing consumable here: neither a coefficient nor a variable.
            throw ParseException("Cannot parse '" + input + "'");
        }

        if (power < 0 || power > MAX_POWER)            // reject before it can index coefTable out of bounds
            throw ParseException("Exponent out of range 0.." +
                                 to_string(MAX_POWER) + " in '" + input + "'");

        coefTable[static_cast<size_t>(power)] += sign * coefficient;
        if (power > highest) highest = power;

        if (i == start)                                 // belt and braces: guarantee forward progress
            throw ParseException("Cannot parse '" + input + "'");
    }

    polynomial.coefficients.assign(coefTable.begin(), coefTable.begin() + highest + 1);
    polynomial.maxDegree = static_cast<size_t>(highest);
    polynomial.trimPolynom();                           // drop cancelled leading terms
    return is;
}

//Overwrite operator<<
ostream& operator<<(ostream& os, const Polynomial& polynomial){
    if(polynomial.maxDegree==0 && polynomial.coefficients[0]==0){
        for(int i=0;i<polynomial.integralOrder;i++)
            os<<"S";
        os<<"P";
        for(int i=0;i<polynomial.derivativeOrder;i++)
            os<<'`';
        os<<"(X)=0"<<endl;
        return os;
    }
    for(int i=0;i<polynomial.integralOrder;i++)
        os<<"S";
    os<<"P";
    for(int i=0;i<polynomial.derivativeOrder;i++)
        os<<'`';
    os<<"(X)=";
    if(polynomial.coefficients[polynomial.maxDegree]==1 && polynomial.maxDegree!=0)
        os<<"X^"<<polynomial.maxDegree;
    else if(polynomial.coefficients[polynomial.maxDegree]==-1 && polynomial.maxDegree!=0)
        os<<"-X^"<<polynomial.maxDegree;
    else if(polynomial.maxDegree!=0)
        os<<polynomial.coefficients[polynomial.maxDegree]<<"X^"<<polynomial.maxDegree;
    for(size_t i = polynomial.maxDegree; i-- > 1; ){
        if(polynomial.coefficients[i]!=0){
            if(polynomial.coefficients[i]>0)
                os<<'+';
            if(polynomial.coefficients[i]==-1)
                os<<'-';
            else if(polynomial.coefficients[i]!=1)
                os<<polynomial.coefficients[i];
            os<<"X";
            if(i!=1)
                os<<'^'<<i;
        }
    }
    if(polynomial.coefficients[0]>0){
        if(polynomial.maxDegree!=0)
            os<<'+';
        os<<polynomial.coefficients[0];
    }
    else if(polynomial.coefficients[0]<0)
        os<<polynomial.coefficients[0];
    os<<endl;
    return os;
}

void Polynomial::printForSave(ostream& os) const {
    if(maxDegree==0 && coefficients[0]==0){
        os<<0;
        return;
    }
    bool first = true;
    for(int i=static_cast<int>(maxDegree);i>=1;i--){
        if(coefficients[i]==0)
            continue;
        if(!first && coefficients[i]>0)
            os<<'+';
        if(i==1)
            os<<coefficients[i]<<"*x";
        else
            os<<coefficients[i]<<"*x^"<<i;
        first = false;
    }
    if(coefficients[0]!=0 || first){
        if(!first && coefficients[0]>0)
            os<<'+';
        os<<coefficients[0];
    }
}

//Destructor for the class, which frees the dynamically allocated memory
Polynomial::~Polynomial()noexcept{
    Logger::getInstance().log("Destroyed Polynomial: " + name, "Polynomial");
    objectCount--;
}

//argument-less constructor
Polynomial::Polynomial():AlgebraEntity("Polynomial"+to_string(objectCount)),maxDegree(0),derivativeOrder(0),integralOrder(0){
    Logger::getInstance().log("Created Polynomial: " + name, "Polynomial");
    coefficients.assign(1,0.0f);
    objectCount++;
}


//constructor with all arguments
Polynomial::Polynomial(const float* coefficients_, const size_t maxDegree_):AlgebraEntity("Polynomial"+to_string(objectCount)),maxDegree(maxDegree_),derivativeOrder(0),integralOrder(0){
    Logger::getInstance().log("Created Polynomial: " + name, "Polynomial");
    coefficients.assign(maxDegree_+1,0.0f);
    for(size_t i=0;i<=maxDegree_;i++)
        coefficients[i] = coefficients_[i];
    objectCount++;
}

//constructor with just length(arr), we initialize the coefficients with 0
// <=> with '\0' for char
Polynomial::Polynomial(const size_t maxDegree_):AlgebraEntity("Polynomial"+to_string(objectCount)),maxDegree(maxDegree_),derivativeOrder(0),integralOrder(0){
    Logger::getInstance().log("Created Polynomial: " + name, "Polynomial");
    coefficients.assign(maxDegree_+1,0.0f);
    objectCount++;
}

//Constructor with just an array; for this to work it must end with INFINITY
// <=> with '\0' for char - from there we take the array's size and do the assignments
Polynomial::Polynomial(const float* coefficients_):AlgebraEntity("Polynomial"+to_string(objectCount)),derivativeOrder(0),integralOrder(0){
    Logger::getInstance().log("Created Polynomial: " + name, "Polynomial");
    if (coefficients_[0] == INFINITY) {
        coefficients.assign(1, 0.0f);
        maxDegree = 0;
        objectCount++;
        return;
    }
    size_t i=0;
    maxDegree = 0;
    while(coefficients_[i++]!=INFINITY)
        maxDegree = i-1;
    coefficients.assign(maxDegree+1,0.0f);
    for(i=0;i<=maxDegree;i++)
        coefficients[i]=coefficients_[i];
    objectCount++;
}


//Copy constructor
Polynomial::Polynomial(const Polynomial& alt):AlgebraEntity(alt){
    Logger::getInstance().log("Created by copy Polynomial: " + name, "Polynomial");
    maxDegree = alt.maxDegree;
    derivativeOrder = alt.derivativeOrder;
    integralOrder = alt.integralOrder;
    coefficients = alt.coefficients;
    objectCount++;
}

//Private constructor used for inner class creation since we don't want the user to set the derivativeDegree
Polynomial::Polynomial(const float* coefficients_, const size_t maxDegree_,const int derivativeOrder_,const int integralOrder_):AlgebraEntity("Polynomial"+to_string(objectCount)),maxDegree(maxDegree_), derivativeOrder(derivativeOrder_),integralOrder(integralOrder_){
    Logger::getInstance().log("Created Polynomial: " + name, "Polynomial");
    coefficients.assign(maxDegree_+1,0.0f);
    for(size_t i = 0; i<=maxDegree_;i++)
        coefficients[i]=coefficients_[i];
    objectCount++;
}

//assignment operator
Polynomial& Polynomial::operator=(const Polynomial& alt){
    if(this == &alt)return *this;
    AlgebraEntity::operator=(alt);
    maxDegree = alt.maxDegree;
    derivativeOrder =alt.derivativeOrder;
    integralOrder = alt.integralOrder;
    coefficients = alt.coefficients;
    return *this;
}

const std::vector<float>& Polynomial::getCoefficients() const{return coefficients;}

size_t Polynomial::getMaxDegree() const{return maxDegree;}

void Polynomial::trimPolynom(){
    const size_t size=maxDegree;
    int newSize = -1;
    for(int j=static_cast<int>(size);j>=0;j--){
        if(coefficients[j]!=0){
            newSize = j;
            break;
        }
    }
    if(newSize == -1){
        coefficients.assign(1,0.0f);
        maxDegree = 0;
        return;
    }
    maxDegree = static_cast<size_t>(newSize);
    coefficients.resize(maxDegree+1);
}

size_t Polynomial::getObjectCount() {return objectCount;}


string Polynomial::toString() const {
    ostringstream os;

    os<<"[Polynomial ID="<<getId()<<"]\n";
    os<<"Name = "<<name<<endl;
    os<<"Maximum degree = "<<maxDegree<<endl;
    os<<*this;

    return os.str();
}

MathEntity* Polynomial::clone() const {
    return new Polynomial(*this);
}

// Renders the solve report as text (roots for degree <= 2, a message otherwise).
string Polynomial::solve() {
    trimPolynom();
    ostringstream os;

    if (maxDegree == 0) {
        if (fabs(coefficients[0]) < 1e-6f)
            os << "P(X)=0 for any X (infinitely many solutions)." << "\n";
        else
            os << "Nonzero constant polynomial: no solutions." << "\n";
        return os.str();
    }

    if (maxDegree == 1) {
        // Form: a*X + b = 0
        const float a = coefficients[1];
        const float b = coefficients[0];

        os << "Degree 1 equation: " << a << "*X + (" << b << ") = 0" << "\n";
        os << "X = " << (-b / a) << "\n";
        return os.str();
    }

    if (maxDegree == 2) {
        // Form: a*X^2 + b*X + c = 0
        const float a = coefficients[2];
        const float b = coefficients[1];
        const float c = coefficients[0];

        os << "Degree 2 equation: " << a << "*X^2 + (" << b << ")*X + (" << c << ") = 0" << "\n";

        const float delta = b * b - 4.0f * a * c;

        if (delta < 0.0f) {
            os << "Delta = " << delta << " (negative): No real roots." << "\n";
            return os.str();
        }

        if (fabs(delta) < 1e-6f) { //delta = 0
            os << "Delta = 0: Double root X1 = X2 = " << (-b / (2.0f * a)) << "\n";
            return os.str();
        }

        // Delta > 0
        const float sqrtDelta = sqrtf(delta);
        os << "Delta = " << delta << ": Distinct real roots:" << "\n";
        os << "X1 = " << ((-b - sqrtDelta) / (2.0f * a)) << "\n";
        os << "X2 = " << ((-b + sqrtDelta) / (2.0f * a)) << "\n";
        return os.str();
    }

    os << "The solve() method currently only supports polynomials up to degree 2." << "\n";
    return os.str();
}
