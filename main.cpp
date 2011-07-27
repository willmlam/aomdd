#include "Scope.h"
#include "TableFunction.h"
#include "AOMDDFunction.h"
#include "BucketTree.h"
#include "CompileBucketTree.h"
#include "utils.h"
#include "base.h"
#include "Model.h"
#include "Graph.h"
#include "PseudoTree.h"
#include "MetaNode.h"
#include "NodeManager.h"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
using namespace aomdd;
using namespace std;

const int OUTPUT_COMPLEXITY_LIMIT = 2048;

list<int> parseOrder(string filename) {
    ifstream infile(filename.c_str());
    if (infile.fail()) {
        cerr << "Error opening file." << endl;
        exit(-1);
    }

    string buffer;
    int nv, intBuffer;
    getline(infile, buffer);

    infile >> nv;

    list<int> ordering;
    for (int i = 0; i < nv; i++) {
        infile >> intBuffer;
        ordering.push_front(intBuffer);
    }
    return ordering;
}

map<int, int> parseEvidence(string filename) {
    ifstream infile(filename.c_str());

    if (infile.fail()) {
        cerr << "Error opening file." << endl;
        exit(-1);
    }

    string buffer;
    int ne, intBuffer;
    getline(infile, buffer);

    infile >> ne;

    map<int, int> evidence;
    for (int i = 0; i < ne; i++) {
        infile >> intBuffer;
        int var = intBuffer;
        infile >> intBuffer;
        int val = intBuffer;
        cout << var << " " << val << endl;
        evidence.insert(make_pair<int, int> (var, val));
    }
    return evidence;
}

void IterateTester(Assignment & a) {
    a.SetAllVal(0);

    cout << "Total card: " << a.GetCard() << endl;

    cout << "Testing iteration" << endl;
    do {
        cout << "Index: " << a.GetIndex() << "  ";
        a.Save(cout);
        cout << endl;
    } while (a.Iterate());

    cout << endl;
}


string inputFile, orderFile, evidFile, dotFile;

bool compileMode, peMode, mpeMode, vbeMode, logMode;

bool verifyVals;

bool ParseCommandLine(int argc, char **argv) {
    bool haveInputFile = false;
    bool haveOrderingFile = false;
    for (int i = 1; i < argc; ++i) {
        string token(argv[i]);
        int len = token.length();
        if (token.substr(0, 1) == "-") {
            if (token.substr(1, len-1) == "f") {
                if (++i >= argc) return false;
                inputFile = string(argv[i]);
                haveInputFile = true;
            }
            else if (token.substr(1, len-1) == "o") {
                if (++i >= argc) return false;
                orderFile = string(argv[i]);
                haveOrderingFile = true;
            }
            else if (token.substr(1, len-1) == "e") {
                if (++i >= argc) return false;
                evidFile = string(argv[i]);
            }
            else if (token.substr(1, len-1) == "t") {
                if (++i >= argc) return false;
                dotFile = string(argv[i]);
            }
            else if (token.substr(1, len-1) == "c") {
                compileMode = true;
            }
            else if (token.substr(1, len-1) == "pe") {
                peMode = true;
            }
            else if (token.substr(1, len-1) == "mpe") {
                mpeMode = true;
            }
            else if (token.substr(1, len-1) == "vbe") {
                vbeMode = true;
            }
            else if (token.substr(1, len-1) == "log") {
                logMode = true;
            }
            else if (token.substr(1, len-1) == "verify") {
                verifyVals = true;
            }
            else {
                return false;
            }
        }
        else {
            return false;
        }
    }
    if (!haveInputFile) {
        cerr << "Missing problem file." << endl;
        return false;
    }
    else if (!haveOrderingFile) {
        cerr << "Missing ordering file." << endl;
        return false;
    }
    return true;
}

int main(int argc, char **argv) {
    cout << setprecision(15);
    cout << "====================================================" << endl;
    cout << "AOMDD-BE Compiler v0.1" << endl;
    cout << "  by William Lam, UC Irvine <willmlam@ics.uci.edu>" << endl;
    cout << "    (original algorithm by Robert Mateescu)" << endl;
    cout << "====================================================" << endl;
    cout << endl;

    if ( !ParseCommandLine(argc, argv) ) {
        cout << "Invalid arguments given" << endl;
        cout << "Options list" << endl;
        cout << "Input files:" << endl;
        cout << "  -f <file>        path to problem file (UAI format) [required]" << endl;
        cout << "  -o <file>        path to elimination ordering file [required]" << endl;
        cout << "  -e <file>        path to evidence file" << endl;
        cout << endl;
        cout << "Output files:" << endl;
        cout << "  -t <file>        path to DOT file to output generated pseudo-tree" << endl;
        cout << endl;
        cout << "Inference options:" << endl;
        cout << "  -c               compile full AOMDD first" << endl;
        cout << "  -pe              compute P(e)" << endl;
        cout << "  -mpe             compute MPE(e) cost" << endl;
        cout << "  -vbe             use vanilla bucket elimination" << endl;
        cout << "  -log             output results in log space" << endl;
        cout << endl;
        return 0;
    }


    Model m;
    cout << "Reading from input file: " << inputFile << endl;
    m.parseUAI(inputFile);
    cout << "Reading from ordering file: " << orderFile << endl;
    list<int> ordering = parseOrder(orderFile);
    map<int, int> evidence;
    if (evidFile != "") {
        cout << "Reading from evidence file: " << evidFile << endl;
        evidence = parseEvidence(evidFile);
    }
    m.SetOrdering(ordering);

    Graph g(m.GetNumVars(), m.GetScopes());
    Scope completeScope = m.GetScopes()[0];
    for (unsigned int i = 1; i < m.GetScopes().size(); ++i) {
        completeScope = completeScope + m.GetScopes()[i];
    }
    g.InduceEdges(ordering);
    PseudoTree pt(g, completeScope);

    cout << "w/h : " << pt.GetInducedWidth() << "/" << pt.GetHeight() << endl;

    if (dotFile != "") {
        cout << "Writing pseudo tree to: " << dotFile << endl;
        WriteDot(pt.GetTree(), dotFile);
    }


    CompileBucketTree cbt(m, &pt, ordering, evidence);
//    cbt.PrintBuckets(cout); cout << endl;

    AOMDDFunction combined;
    if (compileMode) {
        combined = cbt.Compile();
        if (!evidence.empty()) {
            Assignment cond;
            map<int, int>::iterator it = evidence.begin();
            for(; it != evidence.end(); ++it) {
                int var = it->first;
                int assign = it->second;
                cond.AddVar(var, combined.GetScope().GetVarCard(var));
                cond.SetVal(var, assign);
            }
            combined.Condition(cond);
        }
        int totalCard = combined.GetScope().GetCard();
        if (totalCard <= OUTPUT_COMPLEXITY_LIMIT) {
            combined.Save(cout); cout << endl;
            combined.PrintAsTable(cout); cout << endl;
        }
        cout << "Total complexity: " << totalCard << endl;
        cout << "AOMDD size: " << combined.Size() << endl;
    }
    if (peMode) {
        double pr;
        if (vbeMode) {
            BucketTree bt(m, ordering, evidence);
            pr = bt.Prob(logMode);
        }
        else {
            pr = cbt.Prob(logMode);
        }
        string prefix = logMode ? "log P(e) = " : "P(e) = ";
        cout << prefix << pr << endl;
    }

    if (mpeMode) {
        double pr;
        if (vbeMode) {
            BucketTree bt(m, ordering, evidence);
            pr = bt.MPE(logMode);
        }
        else {
            pr = cbt.MPE(logMode);
        }
        string prefix = logMode ? "log MPE = " : "MPE = ";
        cout << prefix << pr << endl;
    }

    if (verifyVals) {
        if (compileMode) {
            Assignment a(completeScope);
            a.SetAllVal(0);
            int misses = 0;
            for (int i = 0; i < OUTPUT_COMPLEXITY_LIMIT; i++) {
                double compVal = combined.GetVal(a, logMode);
                double flatVal = logMode ? 0 : 1;
                BOOST_FOREACH(const TableFunction &tf, m.GetFunctions()) {
                    if (logMode) {
                        flatVal += tf.GetVal(a, logMode);
                    }
                    else {
                        flatVal *= tf.GetVal(a, logMode);
                    }
                }
                cout << "cv=" << compVal << ", fv=" << flatVal;
                if (fabs(compVal - flatVal) > 1e-20) {
                    misses++;
                    cout << "...not matching!" << endl;
                }
                else {
                    cout << endl;
                }
                if(!a.Iterate()) break;
            }
            cout << "Number of incorrect values: " << misses << endl;
        }
    }

    return 0;
}
