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

class TableFunction : public Function {
private:
    std::vector<double> values;
public:
    TableFunction() {
    }

    TableFunction(const Scope &domainIn) : Function(domainIn) {
        values.resize(this->domain.GetCard());
    }

    virtual ~TableFunction() {
    }

    virtual double GetVal(const Assignment &a) throw (GenericException) {
        int idx = a.GetIndex();
        if (idx == UNKNOWN_VAL || idx >= (int) values.size())
            throw GenericException("Invalid indexing of function");
        return values[idx];
    }

    virtual bool SetVal(const Assignment &a, double val) {
        int idx = a.GetIndex();
        if (idx < 0 || idx == UNKNOWN_VAL || idx >= (int) values.size())
            return false;
        values[idx] = val;
        return true;
    }

    virtual void SetOrdering(const std::list<int> &ordering) {
        // Ensure the set of variables is the same
        std::list<int> tempOrdering(ordering);
        std::list<int>::iterator it = tempOrdering.begin();
        for( ; it != tempOrdering.end(); ++it) {
            if(!domain.VarExists(*it)) tempOrdering.erase(it);
        }

        std::vector<double> newValues(values.size(), 0);
        Assignment a(domain);
        a.SetAllVal(0);
        do {
           newValues[a.GetIndex(tempOrdering)] = GetVal(a);
        } while(a.Iterate());
        domain.SetOrdering(tempOrdering);
        values = newValues;
    }


};

} // end of aomdd namespace

#endif /* TABLEFUNCTION_HPP_ */
