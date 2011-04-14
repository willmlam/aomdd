/*
 *  Bucket.cpp
 *  aomdd
 *
 *  Created by William Lam on Apr 8, 2011
 *  Copyright 2011 UC Irvine. All rights reserved.
 *
 */

#include "Bucket.h"

namespace aomdd {
using namespace std;

Bucket::Bucket() {
}

void Bucket::AddFunction(const TableFunction *f) {
    functions.push_back(f);
    s = s + f->GetScope();
}

TableFunction *Bucket::Flatten(const list<int> &ordering) {
    if (functions.size() == 0)
        return new TableFunction();
    TableFunction *message = new TableFunction(*functions[0]);
    for (unsigned int i = 1; i < functions.size(); ++i) {
        message->Multiply(*functions[i]);
    }
    message->SetOrdering(ordering);
    return message;
}

Bucket::~Bucket() {
}

void Bucket::Save(ostream &out) {
    for (unsigned int i = 0; i < functions.size(); ++i) {
        functions[i]->Save(out);
        out << endl;
    }
}

} // end of aomdd namespace
