/*
 *  CompileBucket.cpp
 *  aomdd
 *
 *  Created by William Lam on Jul 15, 2011
 *  Copyright 2011 UC Irvine. All rights reserved.
 *
 */

#include "CompileBucket.h"
using namespace std;

namespace aomdd {

CompileBucket::CompileBucket() {
}

void CompileBucket::AddFunction(const AOMDDFunction *f) {
    functions.push_back(f);
    s = s + f->GetScope();
}

AOMDDFunction *CompileBucket::Flatten() {
    AOMDDFunction *message;
    if (functions.size() == 0) {
        message = new AOMDDFunction();
    }
    else {
        message = new AOMDDFunction(*functions[0]);
        for (unsigned int i = 1; i < functions.size(); ++i) {
            message->Multiply(*functions[i]);
        }
    }
    return message;
}

void CompileBucket::PrintFunctionScopes(ostream &out) const {
    BOOST_FOREACH(const AOMDDFunction *f, functions) {
        f->GetScope().Save(out); out << endl;
    }
    out << endl;
}

CompileBucket::~CompileBucket() {
}

} // end of aomdd namespace
