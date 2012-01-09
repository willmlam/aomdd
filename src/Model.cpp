/*
 *  Model.cpp
 *  aomdd
 *
 *  Created by William Lam on Mar 31, 2011
 *  Copyright 2011 UC Irvine. All rights reserved.
 *
 */

// Represents graphical models using table functions

#include "Model.h"

namespace aomdd {
using namespace std;

Model::Model() : numVars(0), maxDomain(0) {
}

void Model::SetOrdering(const list<int> &orderIn) {
    ordering = orderIn;
    for (unsigned int i = 0; i < scopes.size(); ++i) {
        scopes[i].SetOrdering(ordering);
    }
    for (unsigned int i = 0; i < functions.size(); ++i) {
        functions[i].SetOrdering(ordering);
    }
    completeScope.SetOrdering(ordering);
}

void Model::AddFunction(const Scope &s, const vector<double> &vals) {
    assert(s.GetCard() == vals.size());
    scopes.push_back(s);
    functions.push_back(TableFunction(s));
    Assignment a(s);
    a.SetAllVal(0);
    int i = 0;
    do {
        functions.back().SetVal(a, vals[i++]);
    } while (a.Iterate());
    completeScope = completeScope + s;
}

void Model::Save(ostream &out) {
    for (unsigned int i = 0; i < functions.size(); ++i) {
        functions[i].Save(out);
        out << endl;
    }
}

Model::~Model() {
}

} // end of aomdd namespace
