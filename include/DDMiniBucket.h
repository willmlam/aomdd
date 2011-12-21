/*
 *  DDMiniBucket.h
 *  aomdd
 *
 *  Created by William Lam on Nov 1, 2011
 *  Copyright 2011 UC Irvine. All rights reserved.
 *
 */

#ifndef DDMINIBUCKET_H_
#define DDMINIBUCKET_H_

#include "AOMDDFunction.h"

namespace aomdd {

// i-bound is the number of variables allowed in the scope+1
// diagram size is the number of ANDNode + MetaNode of a function
enum PartitionMetric {I_BOUND, DIAGRAM_SIZE};

class DDMiniBucket {
    Scope s;
    std::vector<const AOMDDFunction *> functions;

    PartitionMetric metric;
    unsigned long bound;

public:
    DDMiniBucket();

    void AddFunction(const AOMDDFunction *f);

    void PurgeFunctions();

    // This generates the messages before elimination
    std::vector<AOMDDFunction *> GenerateMessages();

    inline void SetPartitionMetric(PartitionMetric m) {
        metric = m;
    }

    inline void SetBound(unsigned long b) {
        bound = b;
    }

    inline const std::vector<const AOMDDFunction *> &GetFunctions() const {
        return functions;
    }

    inline int GetBucketSize() const { return functions.size(); }

    inline void ResizeBucket(int n) { functions.resize(n); }


    void PrintFunctionScopes(std::ostream &out) const;
    void PrintFunctionTables(std::ostream &out) const;
    void PrintDiagrams(std::ostream &out) const;
    void PrintDiagramSizes(std::ostream &out) const;

    virtual ~DDMiniBucket();
};

} /* namespace aomdd */
#endif /* DDMINIBUCKET_H_ */
