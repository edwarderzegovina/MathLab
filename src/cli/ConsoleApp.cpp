#include "ConsoleApp.h"
#include "InputUtils.h"
#include "ConsoleIO.h"
#include "Logger.h"
#include <string>
#include <iomanip>
#include <limits>
#include "MathLabException.h"

void ConsoleApp::printSystems() const{
    const auto temp = workspace.ofType<LinearSystem>();
    if (temp.empty()) {
        throw EmptySetException("No systems exist.");
    }
    for (const auto& obj: temp)
        cout<<obj->getId()<<".\n"<<*obj;
}

void ConsoleApp::printPolynomials() const{
    const auto temp = workspace.ofType<Polynomial>();
    if (temp.empty()) {
        throw EmptySetException("No polynomials exist.");
    }
    for (const auto& obj:temp)
        cout<<obj->getId()<<".\n"<<*obj;
}

void ConsoleApp::printMatrices() const{
    const auto temp = workspace.ofType<Matrix>();
    if (temp.empty()) {
        throw EmptySetException("No matrices exist");
    }
    for (const auto& obj: temp)
        cout<<obj->getId()<<".\n"<<*obj;
}

void ConsoleApp::printDatasets() const {
    auto temp = workspace.ofType<Dataset>();
    if (temp.empty()) {
        throw EmptySetException("You have no datasets");
    }
    for (const auto& obj:temp)
        cout<<obj->getId()<<".\n"<<*obj;
}

// The six relational answers, in a fixed order.
void ConsoleApp::printComparisons(const string& lhsLabel, const string& rhsLabel,
                                  const ComparisonResult& comparison) const {
    cout<<lhsLabel<<" == "<<rhsLabel<<" : "<<(comparison.equal?"True":"False")<<endl;
    cout<<lhsLabel<<" != "<<rhsLabel<<" : "<<(comparison.notEqual?"True":"False")<<endl;
    cout<<lhsLabel<<" <  "<<rhsLabel<<" : "<<(comparison.less?"True":"False")<<endl;
    cout<<lhsLabel<<" <= "<<rhsLabel<<" : "<<(comparison.lessOrEqual?"True":"False")<<endl;
    cout<<lhsLabel<<" >  "<<rhsLabel<<" : "<<(comparison.greater?"True":"False")<<endl;
    cout<<lhsLabel<<" >= "<<rhsLabel<<" : "<<(comparison.greaterOrEqual?"True":"False")<<endl;
}

void ConsoleApp::promptSaveMatrixResult(const Matrix& result) {
    char saveChoice;
    cout<<"Do you want to save the result as a new matrix? (Y/N): ";
    cin>>saveChoice;
    if (cin.fail())
        throw InputException("Invalid input");
    if (tolower(saveChoice)=='y') {
        workspace.add(new Matrix(result));
        cout<<"[+] Result saved."<<endl;
    }
    cout<<"Do you want to save the result in the cache?";
    cin>>saveChoice;
    if (cin.fail())
        throw InputException("Invalid input");
    if (tolower(saveChoice) == 'y') {
        mathCache.addELement(result);
        Logger::getInstance().log("Matrix saved successfully in MathCache (Template)", "Cache");
    }
}

void ConsoleApp::promptSavePolynomialResult(const Polynomial& result) {
    char saveChoice;
    cout<<"Do you want to save the result as a new polynomial? (Y/N): ";
    cin>>saveChoice;
    if (cin.fail())
        throw InputException("Invalid input");
    if (tolower(saveChoice)=='y') {
        workspace.add(new Polynomial(result));
        cout<<"[+] Result saved."<<endl;
    }
}

void ConsoleApp::promptSaveSystemResult(const LinearSystem& result) {
    char saveChoice;
    cout<<"Do you want to save the result as a new system? (Y/N): ";
    cin>>saveChoice;
    if (cin.fail())
        throw InputException("Invalid input");
    if (tolower(saveChoice)=='y') {
        workspace.add(new LinearSystem(result));
        cout<<"[+] Result saved."<<endl;
    }
}

void ConsoleApp::datasetMenu() {
    while (true) {
        cout<<"===== Dataset Interface ====="<<endl;
        cout<<"1. Show all datasets"<<endl;
        cout<<"2. Add new dataset (manual)"<<endl;
        cout<<"3. Import dataset from CSV"<<endl;
        cout<<"4. Export dataset to CSV"<<endl;
        cout<<"5. Show dataset summary"<<endl;
        cout<<"6. Add a value to a dataset"<<endl;
        cout<<"7. Sort dataset"<<endl;
        cout<<"8. Search for a value in a dataset"<<endl;
        cout<<"9. Remove outliers from a dataset"<<endl;
        cout<<"10. Detailed statistics"<<endl;
        cout<<"11. Operators on two datasets (submenu)"<<endl;
        cout<<"12. Delete a dataset"<<endl;
        cout<<"13. Back"<<endl;
        cout<<"Choose the option: ";

        int command;
        if (!InputUtils::readInt(cin, command)) {
            cout << "\nEOF - shutting down.\n";
            return;
        }
        if (command == -1) {
            cout<<"Invalid input."<<endl;
            continue;
        }
        if (command == 13)
            return;
        auto datasets = workspace.ofType<Dataset>();
        try {
            switch (command) {
                case 1: {
                    printDatasets();
                    break;
                }
                case 2: {
                    auto* ptr = new Dataset();
                    cin>>*ptr;
                    workspace.add(ptr);
                    cout<<"[+] Dataset added."<<endl;
                    break;
                }
                case 3: {
                    string fileName;
                    cout<<"Enter the path to the CSV: ";
                    cin>>fileName;
                    Dataset localDataset = workspace.datasetImportFromCSV(fileName);
                    workspace.add(new Dataset(localDataset));
                    cout<<"[+] Import successful."<<endl;
                    break;
                }
                case 4: {
                    if (datasets.empty()) {
                        throw EmptySetException("Datasets");
                    }
                    int index;
                    string fileName;
                    cout<<"ID: ";
                    cin>>index;
                    if (cin.fail() || index<0) {
                        throw InputException("[-] The ID must be a positive number");
                    }
                    workspace.datasetById(index);   // an unknown id is rejected before the next prompt
                    cout<<"Destination CSV file: ";
                    cin>>fileName;
                    workspace.datasetExportToCSV(index, fileName);
                    cout<<"[+] Export successful";
                    break;
                }
                case 5: {
                    if (datasets.empty())
                        throw EmptySetException("Datasets");
                    int index;
                    cout<<"ID: ";
                    cin>>index;
                    if (cin.fail()) {
                        throw InputException("[-] The ID must be a positive number");
                    }
                    cout<<*workspace.datasetById(index);
                    ConsoleIO::printSummary(cout, workspace.summariseDataset(index));
                    break;
                }
                case 6: {
                    if (datasets.empty()) {
                        throw EmptySetException("Datasets");
                    }
                    int index;
                    double value;
                    cout<<"ID: ";
                    cin>>index;
                    if (cin.fail() || index<0) {
                        throw InputException("[-] The ID must be a positive number");
                    }
                    workspace.datasetById(index);   // an unknown id is rejected before the next prompt
                    cout<<"Value to add: ";
                    cin>>value;
                    if (cin.fail()) {
                        throw InputException("[-] Invalid value");
                    }
                    workspace.datasetAddValue(index, value);
                    cout<<'\n';
                    cout<<"[+] Value added."<<endl;
                    break;
                }
                case 7: {
                    if (datasets.empty()) {
                        throw EmptySetException("Datasets");
                    }
                    int index;
                    cout<<"ID: ";
                    cin>>index;
                    if (cin.fail() || index<0) {
                        throw InputException("[-] The ID must be a positive number");
                    }
                    workspace.datasetSort(index);
                    cout<<"[+] Dataset sorted."<<endl;
                    break;
                }
                case 8: {
                    if (datasets.empty()) {
                        throw EmptySetException("Datasets");
                    }
                    int index;
                    double value;
                    cout<<"ID: ";
                    cin>>index;
                    if (cin.fail() || index<0) {
                        throw InputException("[-] The ID must be a positive number");
                    }
                    workspace.datasetById(index);   // an unknown id is rejected before the next prompt
                    cout<<"Value to search for: ";
                    cin>>value;
                    if (cin.fail()) {
                        throw InputException("[-] Invalid value");
                    }
                    cout<<(workspace.datasetContains(index, value)?"[+] Value found.":"[-] Value not found.")<<endl;
                    break;
                }
                case 9: {
                    if (datasets.empty()) {
                        throw EmptySetException("Datasets");
                    }
                    int index;
                    cout<<"ID: ";
                    cin>>index;
                    if (cin.fail() || index<0) {
                        throw InputException("[-] The ID must be a positive number");
                    }
                    workspace.datasetRemoveOutliers(index);
                    cout<<"[+] Outliers have been processed."<<endl;
                    break;
                }
                case 10: {
                    if (datasets.empty()) {
                        throw EmptySetException("Datasets");
                    }
                    int index;
                    cout<<"ID: ";
                    cin>>index;
                    if (cin.fail() || index<0) {
                        throw InputException("[-] The ID must be a positive number");
                    }
                    const DatasetSummary stats = workspace.summariseDataset(index);
                    // Reports an empty set by printing the "Min: " label and
                    // then throwing, matching the failure shape of the other
                    // per-line reads below.
                    if (stats.empty) {
                        cout<<"Min: ";
                        throw EmptySetException("You have no data in the dataset");
                    }
                    cout<<"Min: "<<stats.min<<endl;
                    cout<<"Max: "<<stats.max<<endl;
                    cout<<"Mean: "<<stats.mean<<endl;
                    cout<<"Median: "<<stats.median<<endl;
                    cout<<"Std dev: "<<stats.stddev<<endl;
                    cout<<"Element[0]: "<<workspace.datasetElement(index, 0)<<endl;
                    break;
                }
                case 11: {
                    while (true) {
                        datasets = workspace.ofType<Dataset>();
                        cout<<"--- Dataset Operators ---"<<endl;
                        cout<<"1. Union (A + B)"<<endl;
                        cout<<"2. Difference (A - B)"<<endl;
                        cout<<"3. Comparisons (==, !=, <, <=, >, >=)"<<endl;
                        cout<<"4. Prefix ++ on A"<<endl;
                        cout<<"5. Postfix ++ on A"<<endl;
                        cout<<"6. Prefix -- on A"<<endl;
                        cout<<"7. Postfix -- on A"<<endl;
                        cout<<"8. Back"<<endl;
                        cout<<"Choose the option: ";

                        // Own try/catch, so a bad-input throw re-prompts this inner
                        // submenu instead of escaping to datasetMenu's outer handler
                        // and kicking the user out of "Dataset Operators" entirely.
                        //
                        // Every read below must tell "end of stream" apart from "bad
                        // input, re-prompt", exactly like InputUtils::readInt does for
                        // the outer menus: a plain `if (cin.fail()) throw ...` cannot
                        // make that distinction, since failbit/eofbit never clear on
                        // their own, and the throw would fire on every iteration
                        // forever at real EOF. So every int read uses
                        // InputUtils::readInt (false only at true EOF -> return out of
                        // datasetMenu, matching the outer-loop idiom); the two char
                        // reads use an equivalent explicit eof()/clear()/ignore() check.
                        try {
                        int op;
                        if (!InputUtils::readInt(cin, op)) {
                            cout << "\nEOF - shutting down.\n";
                            return;
                        }
                        if (op == -1) {
                            throw InputException("[-] Invalid input");
                        }
                        if (op==8)
                            break;

                        if (op>=1 && op<=3) {
                            if (datasets.size() < 2) {
                                throw EmptySetException("Not enough datasets. You need 2, you have "+to_string(datasets.size()));
                            }
                            int firstId,secondId;
                            cout<<"First ID: ";
                            if (!InputUtils::readInt(cin, firstId)) {
                                cout << "\nEOF - shutting down.\n";
                                return;
                            }
                            cout<<"Second ID: ";
                            if (!InputUtils::readInt(cin, secondId)) {
                                cout << "\nEOF - shutting down.\n";
                                return;
                            }

                            if (firstId<0 || secondId<0) {
                                throw InputException("[-] The ID must be a positive number");
                            }
                            if (firstId == secondId)
                                throw MathLabException("The operation on the same objects is pointless");
                            if (op==1) {
                                Dataset reunion = workspace.datasetBinaryOp(firstId,secondId,'+');
                                cout<<"(A + B):\n"<<reunion;
                                char saveChoice;
                                cout<<"Do you want to save the result as a new dataset? (Y/N): ";
                                if (!(cin>>saveChoice)) {
                                    if (cin.eof()) {
                                        cout << "\nEOF - shutting down.\n";
                                        return;
                                    }
                                    cin.clear();
                                    cin.ignore(numeric_limits<streamsize>::max(), '\n');
                                    throw InputException("[-] Invalid input");
                                }
                                if (tolower(saveChoice)=='y') {
                                    workspace.add(new Dataset(reunion));
                                    cout<<"Result saved."<<endl;
                                }
                            }
                            else if (op==2) {
                                Dataset difference = workspace.datasetBinaryOp(firstId,secondId,'-');
                                cout<<"(A - B):\n"<<difference;
                                char saveChoice;
                                cout<<"Do you want to save the result as a new dataset? (Y/N): ";
                                if (!(cin>>saveChoice)) {
                                    if (cin.eof()) {
                                        cout << "\nEOF - shutting down.\n";
                                        return;
                                    }
                                    cin.clear();
                                    cin.ignore(numeric_limits<streamsize>::max(), '\n');
                                    throw InputException("[-] Invalid input");
                                }
                                if (tolower(saveChoice)=='y') {
                                    workspace.add(new Dataset(difference));
                                    cout<<"Result saved."<<endl;
                                }
                            }
                            else {
                                printComparisons("A", "B", workspace.datasetCompare(firstId,secondId));
                            }
                            cout<<"\n\n";
                            continue;
                        }

                        switch (op) {
                            case 4: {
                                if (datasets.empty()) {
                                    throw EmptySetException("Datasets");
                                }
                                int first;
                                cout<<"ID: ";
                                if (!InputUtils::readInt(cin, first)) {
                                    cout << "\nEOF - shutting down.\n";
                                    return;
                                }
                                if (first<0) {
                                    throw InputException("[-] The ID must be a positive number");
                                }
                                Dataset pre = workspace.datasetStep(first,'+',true);
                                cout<<"(++A):\n"<<pre;
                                break;
                            }
                            case 5: {
                                if (datasets.empty()) {
                                    throw EmptySetException("Datasets");
                                }
                                int first;
                                cout<<"ID: ";
                                if (!InputUtils::readInt(cin, first)) {
                                    cout << "\nEOF - shutting down.\n";
                                    return;
                                }
                                if (first<0) {
                                    throw InputException("[-] The ID must be a positive number");
                                }
                                Dataset post = workspace.datasetStep(first,'+',false);
                                cout<<"(A++ - old state):\n"<<post;
                                cout<<"(A current):\n"<<*workspace.datasetById(first);
                                //I don't remember what I wanted to show here anymore :(
                                break;
                            }
                            case 6: {
                                if (datasets.empty()) {
                                    throw EmptySetException("Datasets");
                                }
                                int first;
                                cout<<"ID: ";
                                if (!InputUtils::readInt(cin, first)) {
                                    cout << "\nEOF - shutting down.\n";
                                    return;
                                }
                                if (first<0) {
                                    throw InputException("[-] The ID must be a positive number");
                                }
                                Dataset pre = workspace.datasetStep(first,'-',true);
                                cout<<"(--A):\n"<<pre;
                                break;
                            }
                            case 7: {
                                if (datasets.empty()) {
                                    throw EmptySetException("Datasets");
                                }
                                int first;
                                cout<<"ID: ";
                                if (!InputUtils::readInt(cin, first)) {
                                    cout << "\nEOF - shutting down.\n";
                                    return;
                                }
                                if (first<0) {
                                    throw InputException("[-] The ID must be a positive number");
                                }
                                Dataset post = workspace.datasetStep(first,'-',false);
                                cout<<"(A-- - old state):\n"<<post;
                                cout<<"(A current):\n"<<*workspace.datasetById(first);
                                //not sure here either what I wanted to show :)
                                break;
                            }
                            default:
                                cout<<"Invalid option."<<endl;
                        }
                        } catch (const MathLabException& e) {
                            cout<<endl<<ConsoleIO::describe(e)<<endl;
                        } catch (const exception& e) {
                            cout<<"\n[Unknown exception] "<<e.what()<<endl;
                        } catch (...) {
                            cout<<"\n[FATAL ERROR]\n";
                        }
                        cout<<"\n\n";
                    }
                    break;
                }
                case 12: {
                    if (datasets.empty()) {
                        throw EmptySetException("Datasets");
                    }
                    int index;
                    cout<<"ID: ";
                    cin>>index;
                    if (cin.fail() || index<0) {
                        throw InputException("[-] The ID must be a positive number");
                    }
                    workspace.removeEntity(workspace.datasetById(index));
                    cout<<"Dataset deleted.";
                    break;
                }
                default:
                    cout<<"Invalid option."<<endl;
            }
        }catch (const MathLabException& e) {
            cout<<endl<<ConsoleIO::describe(e)<<endl;
        }catch (const exception& e) {
            cout<<"\n[Unknown exception] "<<e.what()<<endl;
        }catch (...) {
            cout<<"\n[FATAL ERROR]\n";
        }
        cout<<"\n\n";
    }
}

void ConsoleApp::matrixMenu() {
    while (true) {
        cout<<"===== Matrix Interface ====="<<endl;
        cout<<"1. Show all matrices"<<endl;
        cout<<"2. Add new matrix (manual)"<<endl;
        cout<<"3. Show a matrix"<<endl;
        cout<<"4. Operations between two matrices (+,-,*,concat)"<<endl;
        cout<<"5. Operations with a scalar (+,-,*)"<<endl;
        cout<<"6. Unary minus (-M)"<<endl;
        cout<<"7. Power (M^p)"<<endl;
        cout<<"8. Echelon form"<<endl;
        cout<<"9. Determinant"<<endl;
        cout<<"10. Comparisons between two matrices"<<endl;
        cout<<"11. Increment/Decrement"<<endl;
        cout<<"12. Element by linear index"<<endl;
        cout<<"13. Delete matrix"<<endl;
        cout<<"14. Show the matrix cache"<<endl;
        cout<<"15. Back"<<endl;
        cout<<"Choose the option: ";

        int command;
        if (!InputUtils::readInt(cin, command)) {
            cout << "\nEOF - shutting down.\n";
            return;
        }
        if (command == -1) {
            cout<<"Invalid input."<<endl;
            continue;
        }

        if (command==15)
            return;
        auto matrices = workspace.ofType<Matrix>();

        try {
            switch (command) {
                case 1: {
                    printMatrices();
                    break;
                }
                case 2: {
                    int n,m;
                    cout<<"Number of rows: ";
                    cin>>n;
                    cout<<"Number of columns: ";
                    cin>>m;
                    if (cin.fail() || n<=0 || m<=0) {
                        throw InputException("[-] Invalid dimensions");
                    }
                    Matrix mat(n,m);
                    ConsoleIO::promptMatrixElements(mat);
                    cin>>mat;
                    if (cin.fail()) {
                        throw InputException("[-] Invalid input");
                    }
                    workspace.add(new Matrix(mat));
                    cout<<"[+] Matrix added."<<endl;
                    break;
                }
                case 3: {
                    if (matrices.empty()) {
                        throw EmptySetException("Matrices");
                    }
                    int index;
                    cout<<"ID: ";
                    cin>>index;
                    if (cin.fail() || index<0) {
                        throw InputException("[-] The ID must be a positive number");
                    }
                    cout<<*workspace.matrixById(index);
                    break;
                }
                case 4: {
                    if (matrices.size()<2) {
                        throw EmptySetException("You need at least 2 matrices. You have " + to_string(matrices.size()));
                    }
                    cout<<"1. A + B"<<endl;
                    cout<<"2. A - B"<<endl;
                    cout<<"3. A * B"<<endl;
                    cout<<"4. concat(A,B)"<<endl;
                    cout<<"Choose the operation: ";
                    int op;
                    cin>>op;
                    if (cin.fail() || op<1 || op>4) {
                        throw InputException("[-] Invalid option");
                    }
                    int firstId,secondId;
                    cout<<"ID: ";
                    cin>>firstId;
                    cout<<"ID: ";
                    cin>>secondId;
                    if (cin.fail() || firstId<0 || secondId<0) {
                        throw InputException("[-] The ID must be a positive number");
                    }
                    const char operation = (op==1) ? '+' : (op==2) ? '-' : (op==3) ? '*' : 'c';
                    Matrix result = workspace.matrixBinaryOp(firstId,secondId,operation);
                    cout<<"[+] Result:\n"<<result;
                    promptSaveMatrixResult(result);
                    break;
                }
                case 5: {
                    if (matrices.empty()) {
                        throw EmptySetException("Matrices");
                    }
                    cout<<"1. M + s"<<endl;
                    cout<<"2. M - s"<<endl;
                    cout<<"3. M * s"<<endl;
                    cout<<"Choose the operation: ";
                    int op;
                    cin>>op;
                    if (cin.fail() || op<1 || op>3) {
                        throw InputException("[-] Invalid option");
                    }
                    int index;
                    float scalar;
                    cout<<"ID: ";
                    cin>>index;
                    if (cin.fail() || index<0) {
                        throw InputException("[-] The ID must be a positive number");
                    }
                    workspace.matrixById(index);   // an unknown id is rejected before the next prompt
                    cout<<"Scalar: ";
                    cin>>scalar;
                    const char operation = (op==1) ? '+' : (op==2) ? '-' : '*';
                    Matrix result = workspace.matrixScalarOp(index,scalar,operation);
                    cout<<"Result:\n"<<result;
                    promptSaveMatrixResult(result);
                    break;
                }
                case 6: {
                    if (matrices.empty()) {
                        throw EmptySetException("Matrices");
                    }
                    int index;
                    cout<<"ID: ";
                    cin>>index;
                    if (cin.fail() || index<0) {
                        throw InputException("[-] The ID must be a positive number");
                    }
                    Matrix result = workspace.matrixNegate(index);
                    cout<<"[+] Result:\n"<<result;
                    promptSaveMatrixResult(result);
                    break;
                }
                case 7: {
                    if (matrices.empty()) {
                        throw EmptySetException("Matrices");
                    }
                    int index,power;
                    cout<<"ID: ";
                    cin>>index;
                    cout<<"Power p (>=1): ";
                    cin>>power;
                    if (cin.fail() || index<0 || power<1) {
                        throw InputException("[-] The ID must be a positive number. (or the power <= 0)");
                    }
                    Matrix result = workspace.matrixPower(index,power);
                    cout<<"[+] Result:\n"<<result;
                    promptSaveMatrixResult(result);
                    break;
                }
                case 8: {
                    if (matrices.empty()) {
                        throw EmptySetException("Matrices");
                    }
                    int index;
                    cout<<"ID: ";
                    cin>>index;
                    if (cin.fail() || index<0) {
                        throw InputException("[-] The ID must be a positive number");
                    }
                    Matrix result = workspace.matrixEchelon(index);
                    cout<<"Echelon form:\n"<<result;
                    promptSaveMatrixResult(result);
                    break;
                }
                case 9: {
                    if (matrices.empty()) {
                        throw EmptySetException("Matrices");
                    }
                    int index;
                    cout<<"ID: ";
                    cin>>index;
                    if (cin.fail() || index<0) {
                        throw InputException("[-] The ID must be a positive number");
                    }
                    cout<<"det(M) = "<<workspace.matrixDeterminant(index)<<endl;
                    break;
                }
                case 10: {
                    if (matrices.size()<2) {
                        throw EmptySetException("You need at least 2 matrices. You have " + to_string(matrices.size()));
                    }
                    int firstId,secondId;
                    cout<<"First ID: ";
                    cin>>firstId;
                    cout<<"Second ID: ";
                    cin>>secondId;
                    if (cin.fail() || firstId<0 ||secondId<0) {
                        throw InputException("[-] The ID must be a positive number");
                    }
                    printComparisons("A", "B", workspace.matrixCompare(firstId,secondId));
                    break;
                }
                case 11: {
                    if (matrices.empty()) {
                        throw EmptySetException("Matrices");
                    }
                    int index;
                    cout<<"ID: ";
                    cin>>index;
                    if (cin.fail() || index<0) {
                        throw InputException("[-] The ID must be a positive number");
                    }
                    cout<<"1. ++M"<<endl;
                    cout<<"2. M++"<<endl;
                    cout<<"3. --M"<<endl;
                    cout<<"4. M--"<<endl;
                    cout<<"Choose the operation: ";
                    int op;
                    cin>>op;
                    if (cin.fail() || op<1 || op>4) {
                        throw InputException("[-] The ID must be a positive number");
                    }
                    if (op==1) {
                        Matrix result = workspace.matrixStep(index,'+',true);
                        cout<<"Result (++M):\n"<<result;
                    }
                    else if (op==2) {
                        Matrix old = workspace.matrixStep(index,'+',false);
                        cout<<"Old state (M++):\n"<<old;
                        cout<<"Current state:\n"<<*workspace.matrixById(index);
                    }
                    else if (op==3) {
                        Matrix result = workspace.matrixStep(index,'-',true);
                        cout<<"Result (--M):\n"<<result;
                    }
                    else {
                        Matrix old = workspace.matrixStep(index,'-',false);
                        cout<<"Old state (M--):\n"<<old;
                        cout<<"Current state:\n"<<*workspace.matrixById(index);
                    }
                    break;
                }
                case 12: {
                    if (matrices.empty()) {
                        throw EmptySetException("Matrices");
                    }
                    int index;
                    size_t elemIndex;
                    cout<<"ID: ";
                    cin>>index;
                    if (cin.fail() || index<0) {
                        throw InputException("[-] The ID must be a positive number");
                    }
                    const int total = workspace.matrixElementCount(index);
                    cout<<"Element index (0-"<<total-1<<"): ";
                    cin>>elemIndex;
                    if (cin.fail()) {
                        throw InputException("[-] The index must be a number between 0 and " + to_string(total-1));
                    }
                    cout<<"Element = "<<workspace.matrixElement(index, elemIndex)<<endl;

                    break;
                }
                case 13: {
                    if (matrices.empty()) {
                        throw EmptySetException("Matrices");
                    }
                    int index;
                    cout<<"ID: ";
                    cin>>index;
                    if (cin.fail() || index<0) {
                        throw InputException("[-] The ID must be a positive number");
                    }
                    workspace.removeEntity(workspace.matrixById(index));
                    cout<<"[+] Matrix deleted."<<endl;
                    break;
                }
                case 14: {
                    cout<<mathCache.toString()<<endl;
                    cout<<"Number of elements in cache: "<<mathCache.getCurrentCount()<<endl;
                    break;
                }
                default:
                    cout<<"[!] Invalid option."<<endl;
            }
        }catch (const MathLabException& e) {
            cout<<endl<<ConsoleIO::describe(e)<<endl;
        }catch (const exception& e) {
            cout<<"\n[Unknown exception] "<<e.what()<<endl;
        }catch (...) {
            cout<<"\n[FATAL ERROR]\n";
        }
        cout<<"\n\n";
    }
}

void ConsoleApp::polynomialMenu() {
    while (true) {
        cout<<"===== Polynomial Interface ====="<<endl;
        cout<<"1. Show all polynomials"<<endl;
        cout<<"2. Add new polynomial (manual)"<<endl;
        cout<<"3. Show a polynomial (+save format)"<<endl;
        cout<<"4. Compute value at a point"<<endl;
        cout<<"5. Compute the definite integral"<<endl;
        cout<<"6. Derivative"<<endl;
        cout<<"7. Integral"<<endl;
        cout<<"8. Unary minus (-P)"<<endl;
        cout<<"9. Operations between two polynomials (+,-,*)"<<endl;
        cout<<"10. Comparisons between two polynomials"<<endl;
        cout<<"11. Increment/Decrement the free term"<<endl;
        cout<<"12. Coefficient by index"<<endl;
        cout<<"13. Delete polynomial"<<endl;
        cout<<"14. Back"<<endl;
        cout<<"Choose the option: ";

        int command;
        if (!InputUtils::readInt(cin, command)) {
            cout << "\nEOF - shutting down.\n";
            return;
        }
        if (command == -1) {
            cout<<"Invalid input."<<endl;
            continue;
        }

        if (command==14)
            return;

        auto polynomials = workspace.ofType<Polynomial>();
        try {
            switch (command) {
                case 1: {
                    printPolynomials();
                    break;
                }
                case 2: {
                    auto* p = new Polynomial();
                    cin>>*p;
                    if (cin.fail()) {
                        delete p;
                        throw InputException("Invalid input");
                    }
                    workspace.add(p);
                    cout<<"[+] Polynomial added."<<endl;
                    break;
                }
                case 3: {
                    if (polynomials.empty()) {
                        throw EmptySetException("Polynomials");
                    }
                    int index;
                    cout<<"ID: ";
                    cin>>index;
                    if (cin.fail() || index<0) {
                        throw InputException("[-] The ID must be a positive number");
                    }
                    cout<<*workspace.polynomialById(index);
                    cout<<"[+] Save format: ";
                    cout<<workspace.polynomialSaveFormat(index);
                    cout<<endl;
                    break;
                }
                case 4: {
                    if (polynomials.empty()) {
                        throw EmptySetException("Polynomials");
                    }
                    int index;
                    float x;
                    cout<<"ID: ";
                    cin>>index;
                    if (cin.fail() || index<0) {
                        throw InputException("[-] The ID must be a positive number");
                    }
                    workspace.polynomialById(index);   // an unknown id is rejected before the next prompt
                    cout<<"x = ";
                    cin>>x;
                    if (cin.fail()) {
                        throw InputException("[-] Invalid value");
                    }
                    cout<<"P("<<x<<") = "<<workspace.polynomialAt(index, x)<<endl;
                    break;
                }
                case 5: {
                    if (polynomials.empty()) {
                        throw EmptySetException("You have no polynomials");
                    }
                    int index;
                    float a,b;
                    cout<<"ID: ";
                    cin>>index;
                    if (cin.fail() || index<0) {
                        throw InputException("[-] The ID must be a positive number");
                    }
                    workspace.polynomialById(index);   // an unknown id is rejected before the next prompt
                    cout<<"a (lower bound) = ";
                    cin>>b;
                    cout<<"b (upper bound) = ";
                    cin>>a;
                    if (cin.fail()) {
                        throw InputException("[-] Invalid values");
                    }
                    cout<<"Definite integral = "<<workspace.polynomialDefiniteIntegral(index,a,b)<<endl;
                    break;
                }
                case 6: {
                    if (polynomials.empty()) {
                        throw EmptySetException("You have no polynomials!");
                    }
                    int index;
                    cout<<"ID: ";
                    cin>>index;
                    if (cin.fail() || index<0) {
                        throw InputException("[-] The ID must be a positive number");
                    }
                    Polynomial result = workspace.polynomialDerivative(index);
                    cout<<"[+] Derivative:\n"<<result;
                    promptSavePolynomialResult(result);
                    break;
                }
                case 7: {
                    if (polynomials.empty()) {
                        throw EmptySetException("No polynomials exist.");
                    }
                    int index;
                    cout<<"ID: ";
                    cin>>index;
                    if (cin.fail() || index<0) {
                        throw InputException("[-] The ID must be a positive number");
                    }
                    Polynomial result = workspace.polynomialIntegral(index);
                    cout<<"[+] Integral:\n"<<result;
                    promptSavePolynomialResult(result);
                    break;
                }
                case 8: {
                    if (polynomials.empty()) {
                        throw EmptySetException("No polynomials exist");
                    }
                    int index;
                    cout<<"ID: ";
                    cin>>index;
                    if (cin.fail() || index<0) {
                        throw InputException("[-] The ID must be a positive number");
                    }
                    Polynomial result = workspace.polynomialNegate(index);
                    cout<<"-P:\n"<<result;
                    promptSavePolynomialResult(result);
                    break;
                }
                case 9: {
                    if (polynomials.size()<2) {
                        throw EmptySetException("You need at least 2 polynomials. You have "+ to_string(polynomials.size()));
                    }
                    cout<<"1. P1 + P2"<<endl;
                    cout<<"2. P1 - P2"<<endl;
                    cout<<"3. P1 * P2"<<endl;
                    cout<<"Choose the operation: ";
                    int op;
                    cin>>op;
                    if (cin.fail() || op<1 || op>3) {
                        throw InputException("[-] Invalid option");
                    }
                    int firstId,secondId;
                    cout<<"First ID: ";
                    cin>>firstId;
                    cout<<"Second ID: ";
                    cin>>secondId;
                    if (cin.fail() || firstId<0 || secondId<0) {
                        throw InputException("[-] The ID must be a positive number");
                    }
                    const char operation = (op==1) ? '+' : (op==2) ? '-' : '*';
                    Polynomial result = workspace.polynomialBinaryOp(firstId,secondId,operation);
                    cout<<"[+] Result:\n"<<result;
                    promptSavePolynomialResult(result);
                    break;
                }
                case 10: {
                    if (polynomials.size()<2) {
                        throw EmptySetException("You need at least 2 polynomials. You have " + to_string(polynomials.size()));
                    }
                    int firstId,secondId;
                    cout<<"First ID: ";
                    cin>>firstId;
                    cout<<"Second ID: ";
                    cin>>secondId;
                    if (cin.fail() || firstId<0 || secondId<0) {
                        throw InputException("[-] The ID must be a positive number");
                    }
                    printComparisons("P1", "P2", workspace.polynomialCompare(firstId,secondId));
                    break;
                }
                case 11: {
                    if (polynomials.empty()) {
                        throw EmptySetException("No polynomials exist!");
                    }
                    int index;
                    cout<<"ID: ";
                    cin>>index;
                    if (cin.fail() || index<0) {
                        throw InputException("[-] The ID must be a positive number");
                    }
                    cout<<"1. ++P"<<endl;
                    cout<<"2. P++"<<endl;
                    cout<<"3. --P"<<endl;
                    cout<<"4. P--"<<endl;
                    cout<<"Choose the operation: ";
                    int op;
                    cin>>op;
                    if (cin.fail() || op<1 || op>4) {
                        throw InputException("[-] Invalid option");
                    }
                    if (op==1) {
                        Polynomial result = workspace.polynomialStep(index,'+',true);
                        cout<<"Result (++P):\n"<<result;
                    }
                    else if (op==2) {
                        Polynomial old = workspace.polynomialStep(index,'+',false);
                        cout<<"Old state (P++):\n"<<old;
                        cout<<"Current state:\n"<<*workspace.polynomialById(index);
                    }
                    else if (op==3) {
                        Polynomial result = workspace.polynomialStep(index,'-',true);
                        cout<<"Result (--P):\n"<<result;
                    }
                    else {
                        Polynomial old = workspace.polynomialStep(index,'-',false);
                        cout<<"Old state (P--):\n"<<old;
                        cout<<"Current state:\n"<<*workspace.polynomialById(index);
                    }
                    break;
                }
                case 12: {
                    if (polynomials.empty()) {
                        throw EmptySetException("No polynomials exist");
                    }
                    int index;
                    size_t coefIndex;
                    cout<<"ID: ";
                    cin>>index;
                    if (cin.fail() || index<0) {
                        throw InputException("[-] The ID must be a positive number");
                    }
                    // Fetched before the prompt is written, so an unknown id
                    // fails with nothing on screen instead of a half-printed prompt.
                    const size_t maxDegree = workspace.polynomialMaxDegree(index);
                    cout<<"Coefficient index (0-"<<maxDegree<<"): ";
                    cin>>coefIndex;
                    if (cin.fail()) {
                        throw InputException("[-] Invalid input");
                    }
                    cout<<"Coef["<<coefIndex<<"] = "<<workspace.polynomialCoefficient(index, coefIndex)<<endl;
                    break;
                }
                case 13: {
                    if (polynomials.empty()) {
                        throw EmptySetException("You have no polynomials");
                    }
                    int index;
                    cout<<"ID: ";
                    cin>>index;
                    if (cin.fail() || index<0 ) {
                        throw InputException("[-] The ID must be a positive number");
                    }
                    workspace.removeEntity(workspace.polynomialById(index));
                    cout<<"[+] Polynomial deleted."<<endl;
                    break;
                }
                default:
                    cout<<"Invalid option."<<endl;
            }
        }catch (const MathLabException& e) {
            cout<<endl<<ConsoleIO::describe(e)<<endl;
        }catch (const exception& e) {
            cout<<"\n[Unknown exception] "<<e.what()<<endl;
        }catch (...) {
            cout<<"\n[FATAL ERROR]\n";
        }
        cout<<"\n\n";
    }
}

void ConsoleApp::systemMenu() {
    while (true) {
        cout<<"===== LinearEquation Interface ====="<<endl;
        cout<<"1. Show all systems"<<endl;
        cout<<"2. Add new system (manual)"<<endl;
        cout<<"3. Show a system"<<endl;
        cout<<"4. Solve a system"<<endl;
        cout<<"5. Operations between two systems (+,-)"<<endl;
        cout<<"6. Comparisons between two systems"<<endl;
        cout<<"7. Increment/Decrement system"<<endl;
        cout<<"8. Free term of an equation"<<endl;
        cout<<"9. Delete system"<<endl;
        cout<<"10. Back"<<endl;
        cout<<"Choose the option: ";

        int command;
        if (!InputUtils::readInt(cin, command)) {
            cout << "\nEOF - shutting down.\n";
            return;
        }
        if (command == -1) {
            cout<<"Invalid input."<<endl;
            continue;
        }

        if (command==10)
            return;

        auto systems = workspace.ofType<LinearSystem>();
        try {
            switch (command) {
                case 1: {
                    printSystems();
                    break;
                }
                case 2: {
                    LinearSystem created = ConsoleIO::readSystem();
                    workspace.add(new LinearSystem(created));
                    cout<<"[+] System added."<<endl;
                    break;
                }
                case 3: {
                    if (systems.empty()) {
                        throw EmptySetException("No systems exist");
                    }
                    int index;
                    cout<<"ID: ";
                    cin>>index;
                    if (cin.fail() || index<0) {
                        throw InputException("[-] The ID must be a positive number");
                    }
                    cout<<*workspace.systemById(index);
                    break;
                }
                case 4: {
                    if (systems.empty()) {
                        throw EmptySetException("No systems exist.");
                    }
                    int index;
                    cout<<"ID: ";
                    cin>>index;
                    if (cin.fail() || index<0) {
                        throw InputException("[-] The ID must be a positive number");
                    }
                    ConsoleIO::printSolution(cout, workspace.solveSystem(index));
                    break;
                }
                case 5: {
                    if (systems.size()<2) {
                        throw EmptySetException("You need at least 2 systems. You have " + to_string(systems.size()));
                    }
                    cout<<"1. A + B"<<endl;
                    cout<<"2. A - B"<<endl;
                    cout<<"Choose the operation: ";
                    int op;
                    cin>>op;
                    if (cin.fail() || op<1 || op>2) {
                        throw InputException("[-] Invalid option");
                    }
                    int firstId,secondId;
                    cout<<"ID: ";
                    cin>>firstId;
                    cout<<"ID: ";
                    cin>>secondId;
                    if (cin.fail() || firstId<0 || secondId<0) {
                        throw InputException("[-] The ID must be a positive number");
                    }
                    LinearSystem result = workspace.systemBinaryOp(firstId,secondId,(op==1)?'+':'-');
                    cout<<"Result:\n"<<result;
                    promptSaveSystemResult(result);
                    break;
                }
                case 6: {
                    if (systems.size()<2) {
                        throw EmptySetException("You need at least two systems. You have " + to_string(systems.size()));
                    }
                    int firstId,secondId;
                    cout<<"ID: ";
                    cin>>firstId;
                    cout<<"ID: ";
                    cin>>secondId;
                    if (cin.fail() || firstId<0 || secondId<0) {
                        throw InputException("[-] The ID must be a positive number");
                    }
                    printComparisons("A", "B", workspace.systemCompare(firstId,secondId));
                    break;
                }
                case 7: {
                    if (systems.empty()) {
                        throw EmptySetException("You have no systems");
                    }
                    int index;
                    cout<<"ID: ";
                    cin>>index;
                    if (cin.fail() || index<0) {
                        throw InputException("[-] The ID must be a positive number");
                    }
                    cout<<"1. ++S"<<endl;
                    cout<<"2. S++"<<endl;
                    cout<<"3. --S"<<endl;
                    cout<<"4. S--"<<endl;
                    cout<<"Choose the operation: ";
                    int op;
                    cin>>op;
                    if (cin.fail() || op<1 || op>4) {
                        throw InputException("[-] Invalid option");
                    }
                    if (op==1) {
                        LinearSystem result = workspace.systemStep(index,'+',true);
                        cout<<"Result (++S):\n"<<result;
                    }
                    else if (op==2) {
                        LinearSystem old = workspace.systemStep(index,'+',false);
                        cout<<"Old state (S++):\n"<<old;
                        cout<<"Current state:\n"<<*workspace.systemById(index);
                    }
                    else if (op==3) {
                        LinearSystem result = workspace.systemStep(index,'-',true);
                        cout<<"Result (--S):\n"<<result;
                    }
                    else {
                        LinearSystem old = workspace.systemStep(index,'-',false);
                        cout<<"Old state (S--):\n"<<old;
                        cout<<"Current state:\n"<<*workspace.systemById(index) ;
                    }
                    break;
                }
                case 8: {
                    if (systems.empty()) {
                        throw EmptySetException("You have no systems");
                    }
                    int index;
                    size_t eqIndex;
                    cout<<"ID: ";
                    cin>>index;
                    if (cin.fail() || index<0) {
                        throw InputException("[-] The ID must be a positive number");
                    }
                    workspace.systemById(index);   // an unknown id is rejected before the next prompt
                    cout<<"Equation index (starting from 0): ";
                    cin>>eqIndex;
                    if (cin.fail()) {
                        throw InputException("[-] Invalid input");
                    }
                    cout<<"Free term = "<<workspace.systemFreeTerm(index, eqIndex)<<endl;
                    break;
                }
                case 9: {
                    if (systems.empty()) {
                        throw EmptySetException("You have no systems");
                    }
                    int index;
                    cout<<"ID: ";
                    cin>>index;
                    if (cin.fail() || index<0) {
                        throw InputException("[-] The ID must be a positive number");
                    }
                    workspace.removeEntity(workspace.systemById(index));
                    cout<<"[+] System deleted."<<endl;
                    break;
                }
                default:
                    cout<<"[!] Invalid option."<<endl;
            }
        }catch (const MathLabException& e) {
            cout<<endl<<ConsoleIO::describe(e)<<endl;
        }catch (const exception& e) {
            cout<<"\n[Unknown exception] "<<e.what()<<endl;
        }catch (...) {
            cout<<"\n[FATAL ERROR]\n";
        }
        cout<<"\n\n";
    }
}


ConsoleApp::~ConsoleApp() noexcept = default;

void ConsoleApp::genericMenu() {
    while (true) {
        cout<<"===== Generic Interface ====="<<endl;
        cout<<"1. Show all objects"<<endl;
        cout<<"2. Clone an object"<<endl;
        cout<<"3. Sort objects by name"<<endl;
        cout<<"4. Back"<<endl;
        cout<<"Choose the option: ";

        int command;
        if (!InputUtils::readInt(cin, command)) {
            cout << "\nEOF - shutting down.\n";
            return;
        }
        if (command == -1) {
            cout<<"Invalid input."<<endl;
            continue;
        }

        if (command == 4)
            return;
        try {
            switch (command) {
                case 1:
                    if (workspace.size() == 0)
                        throw EmptySetException("No objects exist in the application");
                    for (const auto& obj: workspace.all())
                        cout<<*obj;
                    break;
                case 2: {
                    if (workspace.size() == 0) {
                        throw EmptySetException("No objects exist in the application");
                    }
                    int index;
                    cout<<"ID: ";
                    cin>>index;
                    if (cin.fail() || index<0 ) {
                        throw InputException("[-] The ID must be a positive number");
                    }
                    workspace.cloneEntity(index);
                    cout<<"The object was cloned and added to the application."<<endl;
                    break;
                }
                case 3:
                    workspace.sortByName();
                    break;
                default:
                    cout<<"[!] Invalid option."<<endl;
            }
        }
        catch (const MathLabException& e) {
            cout<<endl<<ConsoleIO::describe(e)<<endl;
        }catch (const exception& e) {
            cout<<"\n[Unknown exception] "<<e.what()<<endl;
        }catch (...) {
            cout<<"\n[FATAL ERROR]\n";
        }
        cout<<"\n\n";
    }
}

void ConsoleApp::algebraMenu() {
    while (true) {
        cout<<"===== Algebra Interface ====="<<endl;
        cout<<"1. Show all objects"<<endl;
        cout<<"2. Solve an object"<<endl;
        cout<<"3. Clone an object "<<endl;
        cout<<"4. Back"<<endl;
        cout<<"Choose the option: ";

        int command;
        if (!InputUtils::readInt(cin, command)) {
            cout << "\nEOF - shutting down.\n";
            return;
        }
        if (command == -1) {
            cout<<"Invalid input."<<endl;
            continue;
        }

        if (command == 4)
            return;

        auto algebraObjects = workspace.ofType<AlgebraEntity>();
        try {
            switch (command) {
                case 1:
                    if (algebraObjects.empty())
                        throw EmptySetException("No algebraic objects exist in the application");
                    for (const auto& obj: algebraObjects)
                        cout<<*obj;
                    break;
                case 2: {
                    if (algebraObjects.empty()) {
                        throw EmptySetException("You have no math objects in the application");
                    }
                    int index;
                    cout<<"ID: ";
                    cin>>index;
                    if (cin.fail() || index<0) {
                        throw InputException("[-] The ID must be a positive number");
                    }
                    cout<<workspace.solveEntity(index);
                    break;
                }
                case 3: {
                    if (algebraObjects.empty()) {
                        throw EmptySetException("You have no algebraic objects in the application");
                    }
                    int index;
                    cout<<"ID: ";
                    cin>>index;
                    if (cin.fail() || index<0) {
                        throw InputException("[-] The ID must be a positive number");
                    }
                    workspace.cloneAlgebraEntity(index);
                    cout<<"The algebraic object was cloned and added."<<endl;
                    break;
                }
                default:
                    cout<<"[!] Invalid option."<<endl;
            }
        }
        catch (const MathLabException& e) {
            cout<<endl<<ConsoleIO::describe(e)<<endl;
        }catch (const exception& e) {
            cout<<"\n[Unknown exception] "<<e.what()<<endl;
        }catch (...) {
            cout<<"\n[FATAL ERROR]\n";
        }
        cout<<"\n\n";
    }
}

void ConsoleApp::dataEntityMenu() {
    while (true) {
        cout<<"===== Data Interface ====="<<endl;
        cout<<"1. Show all objects"<<endl;
        cout<<"2. Show summary"<<endl;
        cout<<"3. Clone an object"<<endl;
        cout<<"4. Back"<<endl;
        cout<<"Choose the option: ";

        int command;
        if (!InputUtils::readInt(cin, command)) {
            cout << "\nEOF - shutting down.\n";
            return;
        }
        if (command == -1) {
            cout<<"Invalid input."<<endl;
            continue;
        }

        if (command == 4)
            return;

        auto dataObjects = workspace.ofType<DataEntity>();
        try {
            switch (command) {
                case 1:
                    if (dataObjects.empty())
                        throw EmptySetException("You have no data of type dataEntity in the application");
                    for (const auto& obj: dataObjects)
                        cout<<*obj;
                    break;
                case 2: {
                    if (dataObjects.empty()) {
                        throw EmptySetException("You have no data of type dataEntity in the application");
                    }
                    int index;
                    cout<<"ID: ";
                    cin>>index;
                    if (cin.fail() || index<0) {
                        throw InputException("[-] The ID must be a positive number");
                    }
                    cout<<workspace.summariseEntity(index);
                    break;
                }
                case 3: {
                    if (dataObjects.empty()) {
                        throw EmptySetException("You have no data of type dataEntity in the application");
                    }
                    int index;
                    cout<<"ID: ";
                    cin>>index;
                    if (cin.fail() || index<0) {
                        throw InputException("[-] The ID must be a positive number");
                    }
                    workspace.cloneDataEntity(index);
                    cout<<"[+] The data object was cloned and added."<<endl;
                    break;
                }
                default:
                    cout<<"[!] Invalid option."<<endl;
            }
        }
        catch (const MathLabException& e) {
            cout<<endl<<ConsoleIO::describe(e)<<endl;
        }catch (const exception& e) {
            cout<<"\n[Unknown exception] "<<e.what()<<endl;
        }catch (...) {
            cout<<"\n[FATAL ERROR]\n";
        }
        cout<<"\n\n";
    }
}


void ConsoleApp::startApplication() {

    cout<<"Welcome to MathLab"<<endl;
    cout<<"Do you want to read data from a project (only if it was created by the application): (Y/N) ";
    char choice;
    cin>>choice;
    if (cin.fail()) {
        cin.clear();
        cin.ignore(10000,'\n');
        cout<<"Invalid input."<<endl;
        return;
    }
    if (tolower(choice) == 'y') {
        cout<<"Enter the directory where you saved the data: ";
        string directory;
        cin>>directory;
        if (cin.fail()) {
            cin.clear();
            cin.ignore(10000,'\n');
            cout<<"Invalid input."<<endl;
            return;
        }
        if(fs::exists(directory)){
            try {
                workspace.loadFromDirectory(directory);
            } catch (const exception& e) {
                cout << e.what() << endl;
            }
        }
        else
            cout<<"Could not find the specified directory"<<endl;
    }
    while (true){
        cout<<"-----------------------"<<endl;
        cout<<"1.Generic Interface"<<endl;
        cout<<"2.Algebra Interface"<<endl;
        cout<<"3.Data Interface"<<endl;
        cout<<"4.Go to the Dataset section"<<endl;
        cout<<"5.Go to the Polynomials section" << endl;
        cout<<"6.Go to the Matrices section" <<endl;
        cout<<"7.Go to the Linear Equations section" <<endl;
        cout<<"8.Load from folder"<<endl;
        cout<<"9.Save to folder"<<endl;
        cout<<"10.Show Log"<<endl;
        cout<<"11. Quickly add read/identity entities"<<endl;
        cout<<"12.Exit the application"<<endl;
        cout<<"-----------------------"<<endl;
        cout<<"Enter the number for the desired operation: ";
        int nr;
        if (!InputUtils::readInt(cin, nr)) {
            cout << "\nEOF - shutting down.\n";
            return;
        }
        if (nr == -1) {
            cout<<"Invalid input."<<endl;
            continue;
        }
        try {
            switch (nr) {
                case 1:
                    genericMenu();
                    break;
                case 2:
                    algebraMenu();
                    break;
                case 3:
                    dataEntityMenu();
                    break;
                case 4:
                    datasetMenu();
                    break;
                case 5:
                    polynomialMenu();
                    break;
                case 6:
                    matrixMenu();
                    break;
                case 7:
                    systemMenu();
                    break;
                case 8: {
                    cout<<"Enter the directory name: ";
                    string buffer;
                    cin>>buffer;
                    if (cin.fail()) {
                        cin.clear();
                        cin.ignore(10000,'\n');
                        cout<<"Invalid input."<<endl;
                        break;
                    }
                    if(fs::exists(buffer))
                        workspace.loadFromDirectory(buffer);
                    else
                        cout<<"No directory with that name exists."<<endl;
                    break;
                }
                case 9: {
                    cout<<"Enter the directory name to save to: ";
                    string saveBuffer;
                    cin>>saveBuffer;
                    if (cin.fail()) {
                        cin.clear();
                        cin.ignore(10000,'\n');
                        cout<<"Invalid input."<<endl;
                        break;
                    }
                    workspace.saveToDirectory(saveBuffer);
                    cout<<"[+] Objects saved successfully";
                    break;
                }
                case 10: {
                    cout << Logger::getInstance().uniqueTypesText();
                    cout << "The log has been updated in memory and will be written to 'logs.txt' on exit.\n";
                    cout << Logger::getInstance().logText();
                    break;
                }
                case 11:
                    factoryMenu();
                    break;
                case 12:
                    return;
                    break;
                default:
                    cout<<"You did not enter a valid option"<<endl;
            }
        }
        catch (const MathLabException& e) {
            cout<<endl<<ConsoleIO::describe(e)<<endl;
        }catch (const exception& e) {
            cout<<"\n[Unknown exception] "<<e.what()<<endl;
        }catch (...) {
            cout<<"\n[FATAL ERROR]\n";
        }
        cout<<"\n\n";
    }
}

void ConsoleApp::factoryMenu() {
    while (true) {
        cout<<"--------Create Objects--------"<<endl;
        cout<<"1.Create Matrices"<<endl;
        cout<<"2.Create Polynomials"<<endl;
        cout<<"3.Create Systems"<<endl;
        cout<<"4.Create Datasets"<<endl;
        cout<<"5.Back" <<endl;
        int choice;
        if (!InputUtils::readInt(cin, choice)) {
            cout << "\nEOF - shutting down.\n";
            return;
        }
        if (choice == -1) {
            cout<<"Invalid input."<<endl;
            continue;
        }
        IMathFactory* f = nullptr;
        try {
            switch (choice) {
                case 1:
                    f = new MatrixFactory();
                    break;
                case 2:
                    f = new PolynomialFactory();
                    break;
                case 3:
                    f = new LinearSystemFactory();
                    break;
                case 4:
                    f = new DataEntityFactory();
                    break;
                case 5:
                    return;
                    break;
                default:
                    throw InputException("Invalid input");
            }
            if (f!=nullptr) {
                while (true) {
                    int c;
                    cout<<"Enter how you want to add it (1=read, 2=Identity) [3 to stop]: ";
                    cin>>c;
                    if (cin.fail())
                        throw InputException("Invalid input");
                    if (c==1) {
                        buildEntity(f,1);
                    }
                    else if (c==2) buildEntity(f,2);
                    else break;
                    cout<<"[+] Object added successfully";
                }
            }
        }
        catch (const MathLabException& e) {
            cout<<endl<<ConsoleIO::describe(e)<<endl;
        }catch (const exception& e) {
            cout<<"\n[Unknown exception] "<<e.what()<<endl;
        }catch (...) {
            cout<<"\n[FATAL ERROR]\n";
        }
        delete f;
        cout<<"\n\n";
    }

}

//buildType = 1 read, =2 identity
void ConsoleApp::buildEntity(const IMathFactory* factory, int buildType) {
    if (buildType == 1) {
        auto ptr = factory->readEntity();
        if (ptr!=nullptr)
            workspace.add(ptr);
    }
    else {
        auto ptr = factory->createIdentity();
        if (ptr!=nullptr)
            workspace.add(ptr);
    }
}

ConsoleApp::ConsoleApp() : workspace(Workspace::instance()) {
    Logger::getInstance().log("ConsoleApp instance created (Singleton)", "System");
}

ConsoleApp &ConsoleApp::getInstance() {
    static ConsoleApp instance;
    return instance;
}
