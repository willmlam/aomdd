/*
 *  CompileBucketTree.h
 *  aomdd
 *
 *  Created by William Lam on Jul 15, 2011
 *  Copyright 2011 UC Irvine. All rights reserved.
 *
 */

#ifndef COMPILEBUCKETTREE_H_
#define COMPILEBUCKETTREE_H_

#include "base.h"
#include "Model.h"
#include "CompileBucket.h"
#include "PseudoTree.h"

namespace aomdd {

class CompileBucketTree {
    std::vector<CompileBucket> buckets;
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

    double maxUTMem;
    double maxOCMem;

    void ResetBuckets();

public:
    CompileBucketTree();
    CompileBucketTree(const Model &m,
            const PseudoTree *ptIn,
            const std::list<int> &orderIn,
            const std::map<int, int> &evidIn, int bucketID);

    AOMDDFunction Compile();

    double Prob(bool logOut = false);
    double MPE(bool logOut = false);

    inline long GetLargestNumMeta() { return numMeta; }
    inline long GetLargestNumAND() { return numAND; }
    inline long GetLargestNumTotal() { return numTotal; }
    inline double GetLargestMem() { return mem; }

    inline void UpdateMaxUTMem() {
        double utMem = NodeManager::GetNodeManager()->MemUsage();
        if (utMem > maxUTMem) maxUTMem = utMem;
    }
    inline double GetMaxUTMem() { return maxUTMem; }

    inline void UpdateMaxOCMem() {
        double ocMem = NodeManager::GetNodeManager()->OpCacheMemUsage();
        if (ocMem > maxOCMem) maxOCMem = ocMem;
    }
    inline double GetMaxOCMem() { return maxOCMem; }

    void PrintBucketFunctionScopes(std::ostream &out) const;
    void PrintBuckets(std::ostream &out) const;
    virtual ~CompileBucketTree();
};

}

#endif /* COMPILEBUCKETTREE_H_ */
