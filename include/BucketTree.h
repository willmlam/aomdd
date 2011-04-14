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
    std::vector<int> parents;
    std::list<int> ordering;
public:
    BucketTree(const Model &m, const std::list<int> &orderIn);

    virtual ~BucketTree();

    double Prob(bool logOut = false);

    void Save(std::ostream &out);

};

} // end of aomdd namespace

#endif /* BUCKETTREE_H_ */
