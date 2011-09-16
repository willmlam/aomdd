/*
 *  BucketTree.cpp
 *  aomdd
 *
 *  Created by William Lam on Apr 8, 2011
 *  Copyright 2011 UC Irvine. All rights reserved.
 *
 */

#include "BucketTree.h"
#include <iomanip>

namespace aomdd {
using namespace std;

BucketTree::BucketTree(const Model &m, const list<int> &orderIn,
        const map<int, int> &evidIn) :
    ordering(orderIn), evidence(evidIn), globalWeight(1.0) {
    const vector<TableFunction> &functions = m.GetFunctions();

    buckets.resize(ordering.size());
    for (unsigned int i = 0; i < functions.size(); i++) {
        if (functions[i].GetScope().GetNumVars() == 0) {
            globalWeight *= functions[i].GetValues()[0];
            continue;
        }
        int idx = functions[i].GetScope().GetOrdering().back();
        buckets[idx].AddFunction(&functions[i]);
    }
}

BucketTree::~BucketTree() {

}

double BucketTree::Prob(bool logOut) {
    list<int>::reverse_iterator rit = ordering.rbegin();
    double pr;
    if (logOut) {
        pr = 0;
    }
    else {
        pr = 1;
    }
    int numBuckets = ordering.size();
    Assignment empty;
    unsigned int count = 1;
    for (; rit != ordering.rend(); ++rit) {
        cout << "Combining functions in bucket " << *rit;
        cout << " (" << count++ << " of " << numBuckets << ")" << endl;
//        buckets[*rit].PrintFunctionTables(cout); cout << endl;
        TableFunction *message = buckets[*rit].Flatten(ordering);
        cout << "After flattening" << endl;
//        message->PrintAsTable(cout); cout << endl;
        Scope elim;
        elim.AddVar(*rit, message->GetScope().GetVarCard(*rit));
        map<int, int>::iterator eit = evidence.find(*rit);
        if (eit != evidence.end()) {
            Assignment cond(elim);
            cond.SetVal(*rit, eit->second);
//            cout << "Conditioning" << endl;
            message->Condition(cond);

        }
        else {
//            cout << "Marginalizing" << endl;
            message->Marginalize(elim);
        }
        cout << "After eliminating " << *rit << endl;
//        message->PrintAsTable(cout); cout << endl;
        if (message->GetScope().IsEmpty()) {
            if (logOut) {
                double val = message->GetVal(empty, true);
                //              cout << "Updating log p(e)" << endl;
                //              cout << "with: " << val << endl;

                pr += val;
            }
            else {
                double val = message->GetVal(empty);
                //                cout << "Updating p(e)" << endl;
                //                cout << "with: " << val << endl;
                pr *= val;
            }
        }
        else {
            int destBucket = message->GetScope().GetOrdering().back();
            cout << "Sending message from <" << *rit << "> to <" << destBucket << ">" << endl;
            buckets[destBucket].AddFunction(message);
        }
    }
    if (logOut) {
        pr += log10(globalWeight);
    }
    else {
        pr *= globalWeight;
    }
    return pr;
}

double BucketTree::MPE(bool logOut) {
    list<int>::reverse_iterator rit = ordering.rbegin();
    double pr;
    if (logOut) {
        pr = 0;
    }
    else {
        pr = 1;
    }
    int numBuckets = ordering.size();
    Assignment empty;
    unsigned int count = 1;
    for (; rit != ordering.rend(); ++rit) {
        cout << "Combining functions in bucket " << *rit;
        cout << " (" << count++ << " of " << numBuckets << ")" << endl;

//        buckets[*rit].PrintFunctionTables(cout); cout << endl;

        TableFunction *message = buckets[*rit].Flatten(ordering);
        cout << "After flattening" << endl;

//        message->PrintAsTable(cout); cout << endl;

        Scope elim;
        elim.AddVar(*rit, message->GetScope().GetVarCard(*rit));
        map<int, int>::iterator eit = evidence.find(*rit);
        if (eit != evidence.end()) {
            Assignment cond(elim);
            cond.SetVal(*rit, eit->second);
//            cout << "Conditioning" << endl;
            message->Condition(cond);

        }
        else {
//            cout << "Marginalizing" << endl;
            message->Maximize(elim);
        }
        cout << "After eliminating " << *rit << endl;

//        message->PrintAsTable(cout); cout << endl;

        if (message->GetScope().IsEmpty()) {
            if (logOut) {
                double val = message->GetVal(empty, true);
                //              cout << "Updating log p(e)" << endl;
                //              cout << "with: " << val << endl;

                pr += val;
            }
            else {
                double val = message->GetVal(empty);
                //                cout << "Updating p(e)" << endl;
                //                cout << "with: " << val << endl;
                pr *= val;
            }
        }
        else {
            int destBucket = message->GetScope().GetOrdering().back();
            cout << "Sending message from <" << *rit << "> to <" << destBucket << ">" << endl;
            buckets[destBucket].AddFunction(message);
        }
    }
    if (logOut) {
        pr += log10(globalWeight);
    }
    else {
        pr *= globalWeight;
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

