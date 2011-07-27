/*
 *  TableFunction.cpp
 *  aomdd
 *
 *  Created by William Lam on Mar 31, 2011
 *  Copyright 2011 UC Irvine. All rights reserved.
 *
 */

#include "TableFunction.h"

namespace aomdd {
using namespace std;

TableFunction::TableFunction() {
}

TableFunction::TableFunction(const Scope &domainIn) :
    Function(domainIn) {
    values.resize(this->domain.GetCard());
}

TableFunction::TableFunction(const Scope &domainIn,
        const vector<double> &valsIn) :
    Function(domainIn), values(valsIn) {
}

TableFunction::~TableFunction() {
}

double TableFunction::GetVal(const Assignment &a, bool logOut) const {
    Assignment at(domain);
    at.SetAssign(a);
    int idx = at.GetIndex();
    if (idx == UNKNOWN_VAL || idx >= (int) values.size()) {
        cout << idx << ", Max is: " << values.size() << std::endl;
        throw GenericException("Invalid indexing of function: " + idx);
    }
    return !logOut ? values[idx] : log(values[idx]);
}

double TableFunction::GetValForceOldOrder(const Assignment &a, bool logOut) const {
    int idx = a.GetIndex();
    if (idx == UNKNOWN_VAL || idx >= (int) values.size()) {
        cout << idx << ", Max is: " << values.size() << std::endl;
        throw GenericException("Invalid indexing of function: " + idx);
    }
    return !logOut ? values[idx] : log(values[idx]);
}

bool TableFunction::SetVal(const Assignment &a, double val) {
    int idx = a.GetIndex();
    if (idx < 0 || idx == UNKNOWN_VAL || idx >= (int) values.size())
        return false;
    values[idx] = val;
    return true;
}

void TableFunction::SetOrdering(const std::list<int> &ordering)
        throw (GenericException) {
    Assignment a(domain);
    domain.SetOrdering(ordering);
    /*
     // Ensure the set of variables is the same
     std::list<int> tempOrdering(ordering);
     std::list<int>::iterator it = tempOrdering.begin();
     for( ; it != tempOrdering.end(); ++it) {
     if(!domain.VarExists(*it)) tempOrdering.erase(it);
     }
     */

    std::vector<double> newValues(values.size(), 0);
    a.SetAllVal(0);
    do {
        newValues[a.GetIndex(domain.GetOrdering())] = GetValForceOldOrder(a);
    } while (a.Iterate());
    values = newValues;
}

// Specify the variables to project onto
void TableFunction::Project(const Scope &s) {
    Scope outScope = domain * s;
    Scope elimVars = domain - outScope;
    vector<double> newValues;
    Assignment a(domain);
    Assignment outScopeA(outScope);
    Assignment elimVarsA(elimVars);
    outScopeA.SetAllVal(0);
    elimVarsA.SetAllVal(0);

    do {
        a.SetAssign(outScopeA);
        double newVal = 0;
        do {
            a.SetAssign(elimVarsA);
            newVal += GetVal(a);
        } while (elimVarsA.Iterate());
        newValues.push_back(newVal);
    } while (outScopeA.Iterate());

    assert(newValues.size() == outScope.GetCard());
    domain = outScope;
    values = newValues;
}

// Multiplies this function with another one
void TableFunction::Multiply(const TableFunction &t) {
    Scope newDomain = domain + t.domain;
    vector<double> newValues;
    Assignment a(newDomain);
    a.SetAllVal(0);
    Assignment lhsA(domain);
    Assignment rhsA(t.domain);
    do {
        lhsA.SetAssign(a);
        rhsA.SetAssign(a);
        newValues.push_back(GetVal(lhsA) * t.GetVal(rhsA));
    } while (a.Iterate());
    assert(newValues.size() == newDomain.GetCard());
    domain = newDomain;
    values = newValues;
}

// Specify the variables to eliminate
void TableFunction::Marginalize(const Scope &margVars) {
    Scope outScope = domain - margVars;
    Scope elimVars = domain * margVars;
    vector<double> newValues;
    Assignment a(domain);
    Assignment outScopeA(outScope);
    Assignment elimVarsA(elimVars);
    outScopeA.SetAllVal(0);
    elimVarsA.SetAllVal(0);

    do {
        a.SetAssign(outScopeA);
        double newVal = 0;
        do {
            a.SetAssign(elimVarsA);
            newVal += GetVal(a);
        } while (elimVarsA.Iterate());
        newValues.push_back(newVal);
    } while (outScopeA.Iterate());

    assert(newValues.size() == outScope.GetCard());
    domain = outScope;
    values = newValues;
}

void TableFunction::Maximize(const Scope &maxVars) {
    Scope outScope = domain - maxVars;
    Scope elimVars = domain * maxVars;
    vector<double> newValues;
    Assignment a(domain);
    Assignment outScopeA(outScope);
    Assignment elimVarsA(elimVars);
    outScopeA.SetAllVal(0);
    elimVarsA.SetAllVal(0);

    do {
        a.SetAssign(outScopeA);
        double newVal = DOUBLE_MIN;
        do {
            a.SetAssign(elimVarsA);
            double temp = GetVal(a);
            if (temp > newVal) {
                newVal = temp;
            }
        } while (elimVarsA.Iterate());
        newValues.push_back(newVal);
    } while (outScopeA.Iterate());

    assert(newValues.size() == outScope.GetCard());
    domain = outScope;
    values = newValues;
}

void TableFunction::Condition(const Assignment &cond) {
    Scope outScope = domain - cond;
    vector<double> newValues;
    Assignment a(domain);
    Assignment outScopeA(outScope);
    outScopeA.SetAllVal(0);

    do {
        a.SetAssign(outScopeA);
        a.SetAssign(cond);
        newValues.push_back(GetVal(a));
    } while (outScopeA.Iterate());

    assert(newValues.size() == outScope.GetCard());
    domain = outScope;
    values = newValues;
}

void TableFunction::Save(ostream& out) const {
    domain.Save(out);
    out << " ";
    for (unsigned int i = 0; i < values.size(); ++i) {
        out << " " << values[i];
    }
}

void TableFunction::PrintAsTable(ostream &out) const {
    Assignment a(domain);
    a.SetAllVal(0);
    do {
        a.Save(out); out << " value=" << GetVal(a) << endl;
    } while (a.Iterate());
}

} // end of aomdd namespace
