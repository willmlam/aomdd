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
#include "NodeManager.h"
#include "PseudoTree.h"

namespace aomdd {

class AOMDDFunction: public Function {
private:
    static NodeManager *mgr;

    MetaNodePtr root;
    const PseudoTree *pt;

    bool fullReduce;

public:
    AOMDDFunction();
    AOMDDFunction(const Scope &domainIn);

    AOMDDFunction(const Scope &domainIn, const std::vector<double> &valsIn);
    // The pseudo tree is preferably one for the entire problem instance
    AOMDDFunction(const Scope &domainIn, const PseudoTree *pseudoTree,
            const std::vector<double> &valsIn, bool fr = true);

    virtual double GetVal(const Assignment &a, bool logOut = false) const;

    // To do later? Difficult if weights are not pushed to bottom.
    virtual bool SetVal(const Assignment &a, double val);

    void Multiply(const AOMDDFunction &rhs);
    void Marginalize(const Scope &elimVars, bool mutableIDs = true);
    void Condition(const Assignment &cond);

    void Normalize();

    inline int Size() const { return root->NumOfNodes(); }
    inline int RootRefCount() const { return root.use_count(); }

    inline void SetScopeOrdering(const std::list<int> &ordering) { domain.SetOrdering(ordering); }

    virtual ~AOMDDFunction();

    virtual void Save(std::ostream &out) const;
    void PrintAsTable(std::ostream &out) const;
};

} // end of aomdd namespace

#endif /* AOMDDFUNCTION_H_ */
