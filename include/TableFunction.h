/*
 *  TableFunction.h
 *  aomdd
 *
 *  Created by William Lam on Mar 31, 2011
 *  Copyright 2011 UC Irvine. All rights reserved.
 *
 */

#ifndef TABLEFUNCTION_H_
#define TABLEFUNCTION_H_

#include "Function.h"

namespace aomdd {

class TableFunction: public Function {
private:
    std::vector<double> values;
public:
    TableFunction();

    TableFunction(const Scope &domainIn);

    virtual ~TableFunction();

    virtual double GetVal(const Assignment &a, bool logOut = false) const;

    virtual bool SetVal(const Assignment &a, double val);

    virtual void SetOrdering(const std::list<int> &ordering)
            throw (GenericException);

    // Specify the variables to project onto
    virtual void Project(const Scope &s);

    // Multiplies this function with another one
    virtual void Multiply(const TableFunction &t);

    // Specify the variables to eliminate
    virtual void Marginalize(const Scope &margVars);

    // Conditions the function on evidence (irreversible)
    virtual void Condition(const Assignment &cond);

    virtual void Save(std::ostream& out) const;

};

} // end of aomdd namespace

#endif /* TABLEFUNCTION_HPP_ */
