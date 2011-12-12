/*
 *  DDMiniBucketTree.h
 *  aomdd
 *
 *  Created by William Lam on Jul 15, 2011
 *  Copyright 2011 UC Irvine. All rights reserved.
 *
 */

#ifndef DDMINIBUCKETTREE_H_
#define DDMINIBUCKETTREE_H_

#include "base.h"
#include "Model.h"
#include "DDMiniBucket.h"
#include "PseudoTree.h"

namespace aomdd {
enum QueryType {PE, MPE};

class DDMiniBucketTree {
    std::vector<DDMiniBucket> buckets;
    const PseudoTree *pt;
    std::list<int> ordering;
    std::map<int, int> evidence;
    std::vector<int> initialBucketSizes;
    int largestBucket;

    bool fullReduce;
    bool compiled;
    AOMDDFunction compiledDD;

    double globalWeight;

    long numMeta, numAND, numTotal;
    double mem;

    void ResetBuckets();

public:
    DDMiniBucketTree();
    DDMiniBucketTree(const Model &m,
            const PseudoTree *ptIn,
            const std::list<int> &orderIn,
            const std::map<int, int> &evidIn, int bucketID, unsigned long bound);

    // Hmm...compiling with mini buckets to get an approximate structure?
    // (we may be constantly partitioning as a result though, what to do at root?)
    AOMDDFunction Compile();

    // Combination operator is product only (for now).
    double Query(QueryType q, bool logOut = false);

    inline long GetLargestNumMeta() { return numMeta; }
    inline long GetLargestNumAND() { return numAND; }
    inline long GetLargestNumTotal() { return numTotal; }
    inline double GetLargestMem() { return mem; }

    void PrintBucketFunctionScopes(std::ostream &out) const;
    void PrintBuckets(std::ostream &out) const;
    virtual ~DDMiniBucketTree();

    /*
    inline double SelfMemUsage() {
        double mem = 0.0;
        for (unsigned int i = 0; i < buckets.size(); ++i) {
            mem += buckets[i].SelfMemUsage();
        }
        return mem + sizeof(*this);
    }
    */
};

}

#endif /* COMPILEBUCKETTREE_H_ */
