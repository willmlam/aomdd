/*
 *  AOMDDFunction.h
 *  aomdd
 *
 *  Created by William Lam on Jun 23, 2011
 *  Copyright 2011 UC Irvine. All rights reserved.
 *
 */

#ifndef AOMDDFUNCTION_H_
#define AOMDDFUNCTION_H_

#include "Function.h"
#include "MetaNode.h"
#include "graphbase.h"

namespace aomdd {

class AOMDDFunction: public Function {
private:
    MetaNodePtr root;
    DirectedGraph pseudoTree;

public:
    AOMDDFunction();
    AOMDDFunction(const Scope &domainIn);

    AOMDDFunction(const Scope &domainIn, const std::vector<double> &valsIn);

    virtual double GetVal(const Assignment &a, bool logOut = false) const;

    virtual bool SetVal(const Assignment &a, double val);
    virtual ~AOMDDFunction();
};

} // end of aomdd namespace

#endif /* AOMDDFUNCTION_H_ */
