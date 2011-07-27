/*
 *  parsers.cpp
 *  aomdd
 *
 *  Created by William Lam on 3/23/11.
 *  Copyright 2011 UC Irvine. All rights reserved.
 *
 */

#ifndef foreach
#define foreach BOOST_FOREACH
#endif

#ifndef reverse_foreach
#define reverse_foreach BOOST_REVERSE_FOREACH
#endif

#include "base.h"
#include "Model.h"
#include "Scope.h"
#include <sstream>
#include <fstream>

namespace aomdd {
using namespace std;

void Model::parseUAI(string filename) {
    ifstream infile(filename.c_str());

    string type;
    int nv, nf;
    int intBuffer;
    double doubleBuffer;
    vector<Scope> fScopes;

    // Parse type
    infile >> type;

    // Parse domains
    infile >> nv;
    numVars = nv;
    for (int i = 0; i < nv; i++) {
        infile >> intBuffer;
        domains.push_back(intBuffer);
    }

    // Parse function scopes
    infile >> nf;
    for (int i = 0; i < nf; i++) {
        infile >> intBuffer;
        int scopeSize = intBuffer;
        list<int> ordering;
        Scope newScope;
        for (int j = 0; j < scopeSize; j++) {
            infile >> intBuffer;
            assert(intBuffer < int(domains.size()));
            ordering.push_back(intBuffer);
            newScope.AddVar(intBuffer, domains[intBuffer]);
        }
        newScope.SetOrdering(ordering);
        fScopes.push_back(newScope);
    }

    // Parse function tables
    for (int i = 0; i < nf; i++) {
        infile >> intBuffer;
        int scopeSize = intBuffer;
        assert(scopeSize == int(fScopes[i].GetCard()));
        TableFunction newFunction(fScopes[i]);
        Assignment a(fScopes[i]);
        a.SetAllVal(0);
        int count = 0;
        do {
            assert(count++ < scopeSize);
            infile >> doubleBuffer;
            newFunction.SetVal(a, doubleBuffer);
        } while (a.Iterate());
        functions.push_back(newFunction);
    }

    /* DEBUG */
    /*

     cout << "Type: " << type << endl;
     cout << "# vars: " << nv << endl;
     cout << "# funcs: " << nf << endl;

     foreach(int i, domains)
     {
     cout << i << " ";
     }

     cout << endl;

     foreach(Scope s, fScopes)
     {
     s.Save(cout);
     cout << endl;
     }

     for(int i=0; i < nf; i++) {
     Assignment a(fScopes[i]);
     a.SetAllVal(0);
     do {
     cout << functions[i].GetVal(a) << " ";
     } while(a.Iterate());
     cout << endl;
     }
     list<int> newOrder;
     newOrder.push_back(1);
     newOrder.push_back(10);
     newOrder.push_back(2);
     newOrder.push_back(11);
     newOrder.push_back(1);
     newOrder.push_back(13);

     functions[nf-1].SetOrdering(newOrder);
     Assignment a(functions[nf-1].GetScope());
     a.SetAllVal(0);
     do {
     cout << functions[nf-1].GetVal(a) << " ";
     } while(a.Iterate());
     cout << endl;


     exit(1);
     */
    /* DEBUG */

}

} // end of aomdd namespace
