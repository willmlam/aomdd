/*
 *  BucketTree.h
 *  aomdd
 *
 *  Created by William Lam on Apr 8, 2011
 *  Copyright 2011 UC Irvine. All rights reserved.
 *
 */

#ifndef BUCKETTREE_H_
#define BUCKETTREE_H_

#include "base.h"
#include "Model.h"
#include "TableFunction.h"
#include "Bucket.h"

namespace aomdd {

class BucketTree {
    std::vector<Bucket> buckets;
    std::vector<Scope> bucketScopes;
    std::vector<int> parents;
    std::list<int> ordering;
    std::map<int, int> evidence;

	// The index stores the sum of the #entries from the incoming messages.
    std::vector<unsigned long> messageSizes;

    Scope s;

    double globalWeight;
public:
    BucketTree(const Model &m, const std::list<int> &orderIn,
            const std::map<int, int> &evidIn);

    virtual ~BucketTree();

    double Prob(bool logOut = false);

    double MPE(bool logOut = false);

    unsigned long ComputeMaxEntriesInMemory();

    void Save(std::ostream &out);

};

} // end of aomdd namespace

#endif /* BUCKETTREE_H_ */
