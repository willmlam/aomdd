/*
 *  TableFunction.hpp
 *  aomdd
 *
 *  Created by William Lam on Mar 31, 2011
 *  Copyright 2011 UC Irvine. All rights reserved.
 *
 */

#ifndef TABLEFUNCTION_HPP_
#define TABLEFUNCTION_HPP_

#include "Function.hpp"

namespace aomdd {

class TableFunction: public Function {
private:
    std::vector<double> values;
public:
    TableFunction() {
    }

    TableFunction(const Scope &domainIn) :
        Function(domainIn) {
        values.resize(this->domain.GetCard());
    }

    virtual ~TableFunction() {
    }

    virtual double GetVal(const Assignment &a) const {
        int idx = a.GetIndex();
        if (idx == UNKNOWN_VAL || idx >= (int) values.size()) {
            std::cout << idx << ", Max is: " << values.size() << std::endl;
            throw GenericException("Invalid indexing of function: " + idx);
        }
        return values[idx];
    }

    virtual bool SetVal(const Assignment &a, double val) {
        int idx = a.GetIndex();
        if (idx < 0 || idx == UNKNOWN_VAL || idx >= (int) values.size())
            return false;
        values[idx] = val;
        return true;
    }

    virtual void SetOrdering(const std::list<int> &ordering)
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
            newValues[a.GetIndex(domain.GetOrdering())] = GetVal(a);
        } while (a.Iterate());
        values = newValues;
    }

    // Specify the variables to project onto
    virtual void Project(const Scope &s) {
        Scope outScope = domain * s;
        Scope elimVars = domain - outScope;
        std::vector<double> newValues;
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
    virtual void Multiply(const TableFunction &t) {
        Scope newDomain = domain + t.domain;
        std::vector<double> newValues;
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
    virtual void Marginalize(const Scope &margVars) {
        Scope outScope = domain - margVars;
        Scope elimVars = domain * margVars;
        std::vector<double> newValues;
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

    virtual void Condition(const Assignment &cond) {
        Scope outScope = domain - cond;
        std::vector<double> newValues;
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

    virtual void Save(std::ostream& out) const {
        domain.Save(out);
        out << " ";
        for (unsigned int i = 0; i < values.size(); ++i) {
            out << " " << values[i];
        }
    }

};

} // end of aomdd namespace

#endif /* TABLEFUNCTION_HPP_ */
