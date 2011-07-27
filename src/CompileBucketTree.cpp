/*
 *  CompileBucketTree.cpp
 *  aomdd
 *
 *  Created by William Lam on Jul 15, 2011
 *  Copyright 2011 UC Irvine. All rights reserved.
 *
 */

#include "CompileBucketTree.h"
#include "TableFunction.h"
using namespace std;

namespace aomdd {

CompileBucketTree::CompileBucketTree() : compiled(false) {
}

CompileBucketTree::CompileBucketTree(const Model &m, const PseudoTree *ptIn,
        const list<int> &orderIn,
        const map<int, int> &evidIn,
        bool fr)
        : pt(ptIn), ordering(orderIn), evidence(evidIn),
        fullReduce(fr), compiled(false) {

    int numBuckets = ordering.size();
    if (pt->HasDummy()) {
        ordering.push_front(numBuckets++);
    }

    buckets.resize(numBuckets);
    initialBucketSizes.resize(numBuckets);
    const vector<TableFunction> &functions = m.GetFunctions();
    for (unsigned int i = 0; i < functions.size(); i++) {
        int idx = functions[i].GetScope().GetOrdering().back();
        AOMDDFunction *f = new AOMDDFunction(functions[i].GetScope(), pt, functions[i].GetValues(), fr);
        buckets[idx].AddFunction(f);
    }
    for (unsigned int i = 0; i < buckets.size(); i++) {
        initialBucketSizes[i] = buckets[i].GetBucketSize();
    }
}

AOMDDFunction CompileBucketTree::Compile() {
    if (!compiled) {
        list<int>::reverse_iterator rit = ordering.rbegin();
        int numBuckets = ordering.size();
        int count = 1;

        const DirectedGraph &tree = pt->GetTree();

        for (; rit != ordering.rend(); ++rit) {
            cout << "Combining functions in bucket " << *rit;
            cout << " (" << count++ << " of " << numBuckets << ")" << endl;
            AOMDDFunction *message = buckets[*rit].Flatten();
            message->SetScopeOrdering(ordering);
            DInEdge ei, ei_end;
            tie(ei, ei_end) = in_edges(*rit, tree);
            // Not at root
            if (ei != ei_end) {
                int parent = source(*ei, tree);
                cout << "Sending message from <" << *rit << "> to <" << parent << ">" << endl;
                buckets[parent].AddFunction(message);
            }
            // At root
            else {
                compiledDD = *message;
            }
        }
    }
    compiled = true;
    return compiledDD;
}

double CompileBucketTree::Prob(bool logOut) {
    double pr = logOut ? 0 : 1;

    // If it's already been compiled, we can just eliminate from the large DD
    if (compiled) {
        AOMDDFunction probFunction(compiledDD);
        const DirectedGraph &tree = pt->GetTree();

        list<int>::reverse_iterator rit = ordering.rbegin();
        int numBuckets = ordering.size();
        int count = 1;

        for (; rit != ordering.rend(); ++rit) {
            DInEdge ei, ei_end;
            tie(ei, ei_end) = in_edges(*rit, tree);
            Scope elim;
            int card = probFunction.GetScope().GetVarCard(*rit);
            elim.AddVar(*rit, card);
            cout << "Eliminating <" << *rit
                    << "> (" << count++ << " of " << numBuckets << ")" << endl;
            map<int, int>::iterator eit = evidence.find(*rit);
            if (eit != evidence.end()) {
                Assignment cond(elim);
                cond.SetVal(*rit, eit->second);
                probFunction.Condition(cond);
            }
            else {
                probFunction.Marginalize(elim, false);
            }
            // At root
            if (ei == ei_end) {
                Assignment a;
                pr = probFunction.GetVal(a, logOut);
            }
        }
    }

    // Otherwise, compile, but eliminate variables along the way
    else {
        ResetBuckets();
        list<int>::reverse_iterator rit = ordering.rbegin();
        int numBuckets = ordering.size();
        int count = 1;

        const DirectedGraph &tree = pt->GetTree();

        for (; rit != ordering.rend(); ++rit) {
            cout << "Combining functions in bucket " << *rit;
            cout << " (" << count++ << " of " << numBuckets << ")" << endl;

//            buckets[*rit].PrintDiagrams(cout); cout << endl;
//            buckets[*rit].PrintFunctionTables(cout); cout << endl;
            AOMDDFunction *message = buckets[*rit].Flatten();
            message->SetScopeOrdering(ordering);
            cout << "After flattening" << endl;

//            message->Save(cout); cout << endl;
//            message->PrintAsTable(cout); cout << endl;

            DInEdge ei, ei_end;
            tie(ei, ei_end) = in_edges(*rit, tree);
            Scope elim;
            int card = message->GetScope().GetVarCard(*rit);
            elim.AddVar(*rit, card);

            map<int, int>::iterator eit = evidence.find(*rit);

            if (eit != evidence.end()) {
                Assignment cond(elim);
                cond.SetVal(*rit, eit->second);
//                cout << "Conditioning with "; cond.Save(cout); cout << endl;
                message->Condition(cond);
            }
            else {
                message->Marginalize(elim);
            }
            cout << "After eliminating " << *rit << endl;

//            message->Save(cout); cout << endl;
//            message->PrintAsTable(cout); cout << endl;

            // empty scope, no need to send message, update final result
            if (message->GetScope().IsEmpty()) {
                Assignment a;
                if (logOut) {
                    pr += message->GetVal(a, logOut);
                }
                else {
                    pr *= message->GetVal(a, logOut);
                }
            }
            // Not at root
            else if (ei != ei_end) {
                int parent = source(*ei, tree);
                cout << "Sending message from <" << *rit << "> to <" << parent << ">" << endl;
                buckets[parent].AddFunction(message);
            }
            // At root
            else {
                Assignment a;
                if (logOut) {
                    pr += message->GetVal(a, logOut);
                }
                else {
                    pr *= message->GetVal(a, logOut);
                }
            }
        }
    }
    return pr;
}

void CompileBucketTree::PrintBucketFunctionScopes(ostream &out) const {
    for (unsigned int i = 0; i < buckets.size(); ++i) {
        out << "Bucket " << i << ":" << endl;
        buckets[i].PrintFunctionScopes(out);
        out << endl;
    }
}

void CompileBucketTree::PrintBuckets(ostream &out) const {
    BOOST_FOREACH(const CompileBucket &b, buckets) {
        b.PrintDiagrams(out); out << endl;
    }
    out << endl;
}

void CompileBucketTree::ResetBuckets() {
    for (unsigned int i = 0; i < buckets.size(); ++i) {
        buckets[i].ResizeBucket(initialBucketSizes[i]);
    }
}

CompileBucketTree::~CompileBucketTree() {
    // TODO Auto-generated destructor stub
}

} // end of aomdd namespace
