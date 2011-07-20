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
        const list<int> &orderIn, bool fr) : pt(ptIn), ordering(orderIn),
        fullReduce(fr), compiled(false) {
    const vector<TableFunction> &functions = m.GetFunctions();

    int numBuckets = ordering.size();
    if (pt->HasDummy()) {
        ordering.push_front(numBuckets++);
    }
    buckets.resize(numBuckets);
    for (unsigned int i = 0; i < functions.size(); i++) {
        int idx = functions[i].GetScope().GetOrdering().back();
        AOMDDFunction *f = new AOMDDFunction(functions[i].GetScope(), pt, functions[i].GetValues(), fr);
        buckets[idx].AddFunction(f);
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
            Scope s;
            int card = probFunction.GetScope().GetVarCard(*rit);
            s.AddVar(*rit, card);
            cout << "Eliminating <" << *rit
                    << "> (" << count++ << " of " << numBuckets << ")" << endl;
            probFunction.Marginalize(s);
            // At root
            if (ei == ei_end) {
                Assignment a;
                pr = probFunction.GetVal(a, logOut);
            }
        }
    }

    // Otherwise, compile, but eliminate variables along the way
    else {
        list<int>::reverse_iterator rit = ordering.rbegin();
        int numBuckets = ordering.size();
        int count = 1;

        const DirectedGraph &tree = pt->GetTree();

        for (; rit != ordering.rend(); ++rit) {
            cout << "Combining functions in bucket " << *rit;
            cout << " (" << count++ << " of " << numBuckets << ")" << endl;
            buckets[*rit].PrintFunctionTables(cout); cout << endl;
            AOMDDFunction *message = buckets[*rit].Flatten();
            message->SetScopeOrdering(ordering);
            cout << "After flattening" << endl;
            message->PrintAsTable(cout); cout << endl;
            DInEdge ei, ei_end;
            tie(ei, ei_end) = in_edges(*rit, tree);
            Scope s;
            int card = message->GetScope().GetVarCard(*rit);
            s.AddVar(*rit, card);
            message->Marginalize(s);
            cout << "After eliminating " << *rit << endl;
            message->PrintAsTable(cout); cout << endl;
            message->Save(cout); cout << endl;
            // Not at root
            if (ei != ei_end) {
                int parent = source(*ei, tree);
                cout << "Sending message from <" << *rit << "> to <" << parent << ">" << endl;
                buckets[parent].AddFunction(message);
            }
            // At root
            else {
                Assignment a;
                pr = message->GetVal(a, logOut);
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

CompileBucketTree::~CompileBucketTree() {
    // TODO Auto-generated destructor stub
}

} // end of aomdd namespace
