/*
 *  BucketTree.cpp
 *  aomdd
 *
 *  Created by William Lam on Apr 8, 2011
 *  Copyright 2011 UC Irvine. All rights reserved.
 *
 */

#include "BucketTree.h"

namespace aomdd {
using namespace std;

BucketTree::BucketTree(const Model &m, const list<int> &orderIn) :
    ordering(orderIn) {
    const vector<TableFunction> &functions = m.GetFunctions();

    buckets.resize(ordering.size());
    for (unsigned int i = 0; i < functions.size(); i++) {
        int idx = functions[i].GetScope().GetOrdering().back();
        buckets[idx].AddFunction(&functions[i]);
    }
}

BucketTree::~BucketTree() {

}

// Remember to get rid of table leaks later
double BucketTree::Prob(bool logOut) {
    list<int>::reverse_iterator rit = ordering.rbegin();
    double pr;
    if (logOut) {
        pr = 0;
    } else {
        pr = 1;
    }
    Assignment empty;
    for (; rit != ordering.rend(); ++rit) {
        cout << "Bucket #" << *rit << endl;
        TableFunction *message = buckets[*rit].FlattenFast(ordering);
        Scope elim;
        elim.AddVar(*rit, message->GetScope().GetVarCard(*rit));
        message->Marginalize(elim);
        if (message->GetScope().IsEmpty()) {
            if (logOut) {
                double val = message->GetVal(empty, true);
                //              cout << "Updating log p(e)" << endl;
                //              cout << "with: " << val << endl;

                pr += val;
            } else {
                double val = message->GetVal(empty);
                //                cout << "Updating p(e)" << endl;
                //                cout << "with: " << val << endl;
                pr *= val;
            }
        } else {
            int destBucket = message->GetScope().GetOrdering().back();
            cout << "Sending message to " << destBucket << endl;
            buckets[destBucket].AddFunction(message);
        }
    }
    return pr;
}

void BucketTree::Save(ostream &out) {
    list<int>::reverse_iterator rit = ordering.rbegin();
    for (; rit != ordering.rend(); ++rit) {
        out << "Bucket #" << *rit << endl;
        buckets[*rit].Save(out);
        out << endl;
    }
}

} // end of aomdd namespace

