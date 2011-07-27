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

vector<Scope> Model::GetScopes() const {
    vector<Scope> ret;
    for (unsigned int i = 0; i < functions.size(); ++i) {
        ret.push_back(functions[i].GetScope());
    }
    return ret;
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
