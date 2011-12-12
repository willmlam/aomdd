/*
 *  CompileBucket.h
 *  aomdd
 *
 *  Created by William Lam on Jul 15, 2011
 *  Copyright 2011 UC Irvine. All rights reserved.
 *
 */

#ifndef COMPILEBUCKET_H_
#define COMPILEBUCKET_H_

#include "AOMDDFunction.h"

namespace aomdd {

class CompileBucket {
    Scope s;
    std::vector<const AOMDDFunction*> functions;
    double weight;
public:
    CompileBucket();

    void AddFunction(const AOMDDFunction *f);

    void PurgeFunctions();

    AOMDDFunction *Flatten();

    inline int GetBucketSize() const { return functions.size(); }

    inline void ResizeBucket(int n) { functions.resize(n); }

    void PrintFunctionScopes(std::ostream &out) const;
    void PrintFunctionTables(std::ostream &out) const;
    void PrintDiagrams(std::ostream &out) const;
    void PrintDiagramSizes(std::ostream &out) const;
    virtual ~CompileBucket();

    inline double SelfMemUsage() {
        double mem = 0.0;
        for (unsigned int i = 0; i < functions.size(); ++i) {
            mem += functions[i]->SelfMemUsage();
        }
        return mem;
    }

    inline void Reweigh(double w) {
        weight *= w;
    }
};

} // end of aomdd namespace

#endif /* COMPILEBUCKET_H_ */
