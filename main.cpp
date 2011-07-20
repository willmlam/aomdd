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
#include <fstream>
#include <sstream>
using namespace aomdd;
using namespace std;

list<int> parseOrder(string filename) {
    ifstream infile(filename.c_str());

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

bool compileMode, peMode;

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
            else if (token.substr(1, len-1) == "p") {
                peMode = true;
            }
            else {
                return false;
            }
        }
        else {
            return false;
        }
    }
    return haveInputFile && haveOrderingFile;
}

int main(int argc, char **argv) {
    cout << "====================================================" << endl;
    cout << "AOMDD-BE Compiler v0.1" << endl;
    cout << "  by William Lam, UC Irvine <willmlam@ics.uci.edu>" << endl;
    cout << "    (original algorithm by Robert Mateescu)" << endl;
    cout << "====================================================" << endl;
    cout << endl;

    if ( !ParseCommandLine(argc, argv) ) {
        cout << "Invalid arguments given" << endl;
        cout << "Options:" << endl;
        cout << "  -f <file>        path to problem file (UAI format)" << endl;
        cout << "  -o <file>        path to elimination ordering file" << endl;
        cout << "  -e <file>        path to evidence file" << endl;
        cout << endl;
        cout << "  -t <file>        path to DOT file to output generated pseudo-tree" << endl;
        cout << endl;
        cout << "  -c               compile full AOMDD" << endl;
        cout << "  -p               compute P(e)" << endl;
        cout << endl;
        return 0;
    }

    Model m;
        m.parseUAI(inputFile);
        list<int> ordering = parseOrder(orderFile);
        map<int, int> evidence;
        if (evidFile != "") evidence = parseEvidence(evidFile);
        m.SetOrdering(ordering);

        Graph g(m.GetScopes());
        Scope completeScope = m.GetScopes()[0];
        for (unsigned int i = 1; i < m.GetScopes().size(); ++i) {
            completeScope = completeScope + m.GetScopes()[i];
        }
        g.InduceEdges(ordering);
        PseudoTree pt(g, completeScope);

        if (dotFile != "") {
            cout << "Writing pseudo tree to: " << dotFile << endl;
            WriteDot(pt.GetTree(), dotFile);
        }


        CompileBucketTree cbt(m, &pt, ordering);
        AOMDDFunction combined;
        if (compileMode) {
            combined = cbt.Compile();
            cout << "AOMDD size: " << combined.Size() << endl;
        }
        if (peMode) {
            cout << "log P(e) = " << cbt.Prob(true) << endl;
            cout << "P(e) = " << cbt.Prob() << endl;
        }





    return 0;
}
