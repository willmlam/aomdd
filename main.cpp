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

int main(int argc, char **argv) {


    string inputFile, orderFile, evidFile;

    inputFile = string(argv[1]);
    orderFile = string(argv[2]);
    if (argc >= 4) {
        evidFile = string(argv[3]);
    }

    Model m;
    try {
        m.parseUAI(inputFile);
        list<int> ordering = parseOrder(orderFile);
        map<int, int> evidence;
        if (argc >= 3) {
            evidence = parseEvidence(evidFile);
        }
        m.SetOrdering(ordering);

        Graph g(m.GetScopes());
        g.InduceEdges(ordering);
        PseudoTree pt(g);


        CompileBucketTree cbt(m, &pt, ordering, false);
//        cbt.PrintBucketFunctionScopes(cout);
        AOMDDFunction combined = cbt.Compile();

        cout << "AOMDD size: " << combined.Size() << endl;




    } catch (GenericException & e) {
        cout << e.what();
    }

    return 0;
}
