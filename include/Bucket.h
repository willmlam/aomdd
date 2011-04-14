/*
 *  Bucket.h
 *  aomdd
 *
 *  Created by William Lam on Apr 8, 2011
 *  Copyright 2011 UC Irvine. All rights reserved.
 *
 */

// A bucket (or clique) of the junction tree
// Does not store the actual original functions

#ifndef BUCKET_H_
#define BUCKET_H_

#include "base.h"
#include "Scope.h"
#include "TableFunction.h"

namespace aomdd {

class Bucket {
    Scope s;
    std::vector<const TableFunction*> functions;
public:
    Bucket();
    void AddFunction(const TableFunction *f);

    TableFunction *Flatten(const std::list<int> &ordering);

    inline unsigned int GetCliqueSize() const {
        return s.GetOrdering().size();
    }
    virtual ~Bucket();

    void Save(std::ostream &out);
};

} // end of aomdd namespace
#endif /* BUCKET_H_ */
