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

Model::Model() {
}

Model::Model(const vector<int> &domainsIn) :
    domains(domainsIn) {
}

Model::Model(const vector<int> &domainsIn, const vector<TableFunction> &funcsIn) :
    domains(domainsIn), functions(funcsIn) {
}

void Model::SetOrdering(const list<int> &orderIn) {
    ordering = orderIn;
    for (unsigned int i = 0; i < functions.size(); ++i) {
        functions[i].SetOrdering(ordering);
    }
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
