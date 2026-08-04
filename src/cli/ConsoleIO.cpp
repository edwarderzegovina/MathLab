#include "ConsoleIO.h"
#include <limits>

using namespace std;

namespace ConsoleIO {

void promptMatrixElements(const Matrix& matrix) {
    cout<<"Enter: "<<matrix.getColumns()<<" numbers per row, "<<matrix.getRows()<<" rows."<<endl;
}

//Since I have two constant values, I'll read and create a matrix to copy into the current one
Matrix readMatrix() {
    int n,m; // n rows , m columns
    cout<<"Enter the number of rows: "<<endl;
    cout<<"Enter the number of columns"<<endl;
    if (!(cin >> n >> m))
        throw InputException("Dimensions must be integers");
    Matrix::validateDimensions(n, m);
    Matrix temp(n,m);
    promptMatrixElements(temp);
    cin>>temp;
    return temp;
}

LinearSystem readSystem() {
    int n=0,m=0;
    string buffer;
    cout<<"Enter your system's name: ";
    cin.ignore(1000,'\n');
    getline(cin,buffer);
    cout<<"Enter the number of equations and the number of variables: ";
    if (!(cin >> n >> m))
        throw InputException("The number of equations and variables must be integers");
    LinearSystem::validateShape(m, n);   // m variables, n equations
    LinearSystem returned(m,n,buffer);
    cout<<"Enter the equations, each on its own line, in the form coef*(var)+....=number; order matters, if you write x+z+y and then x+y+z it will still count as x+z+y"<<endl;
    cin>>returned;
    return returned;
}

void printSolution(ostream& out, const SolutionReport& report) {
    out << report.toText();
}

void printSummary(ostream& out, const DatasetSummary& summary) {
    out << summary.toText();
}

const char* describe(const MathLabException& error) {
    if (dynamic_cast<const InputException*>(&error) != nullptr) {
        cin.clear();
        cin.ignore(1000,'\n');
    }
    return error.what();
}

}
