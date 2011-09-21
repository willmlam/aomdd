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

CompileBucketTree::CompileBucketTree() : compiled(false), globalWeight(1.0) {
}

CompileBucketTree::CompileBucketTree(const Model &m, const PseudoTree *ptIn,
        const list<int> &orderIn,
        const map<int, int> &evidIn, int bucketID)
        : pt(ptIn), ordering(orderIn), evidence(evidIn), largestBucket(bucketID),
        compiled(false), globalWeight(1.0) {

    int numBuckets = ordering.size();
    if (pt->HasDummy()) {
        ordering.push_front(numBuckets++);
    }

    buckets.resize(numBuckets);
    initialBucketSizes.resize(numBuckets);
    const vector<TableFunction> &functions = m.GetFunctions();
    for (unsigned int i = 0; i < functions.size(); i++) {
        int idx = functions[i].GetScope().GetOrdering().back();
        if (functions[i].GetScope().GetNumVars() == 0) {
            globalWeight *= functions[i].GetValues()[0];
            continue;
        }
        AOMDDFunction *f = new AOMDDFunction(functions[i].GetScope(), pt, functions[i].GetValues());
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
            // Try this for now (is it worth it for full compilation?)
            NodeManager::GetNodeManager()->UTGarbageCollect();

            cout << "Memory usage: " << NodeManager::GetNodeManager()->GetUTMemUsage() + NodeManager::GetNodeManager()->GetOCMemUsage() << endl;

            cout << "Combining functions in bucket " << *rit;
            cout << " (" << count++ << " of " << numBuckets << ")" << endl;

            /*
            buckets[*rit].PrintDiagrams(cout); cout << endl;
            buckets[*rit].PrintFunctionTables(cout); cout << endl;
            */

            AOMDDFunction *message = buckets[*rit].Flatten();
            buckets[*rit].PurgeFunctions();
            message->SetScopeOrdering(ordering);

            /*
            message->Save(cout); cout << endl;
            message->PrintAsTable(cout); cout << endl;
            */

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
        NodeManager::GetNodeManager()->PurgeOpCache();
        NodeManager::GetNodeManager()->UTGarbageCollect();
    }
    compiled = true;
    compiledDD.ReweighRoot(globalWeight);
    return compiledDD;
}

double CompileBucketTree::Prob(bool logOut) {
    double pr = logOut ? 0 : 1;

    // If it's already been compiled, we can just eliminate from the large DD
    if (compiled) {
        Assignment a;
        typedef std::map<int, int> map_t;
        BOOST_FOREACH(map_t::value_type i, evidence) {
            a.AddVar(i.first, i.second + 1);
            a.SetVal(i.first, i.second);
        }

        pr = compiledDD.Sum(a);
        if (logOut) pr = log10(pr);

        /*
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
        */
    }

    // Otherwise, compile, but eliminate variables along the way
    else {
        ResetBuckets();
        list<int>::reverse_iterator rit = ordering.rbegin();
        int numBuckets = ordering.size();
        int count = 1;

        const DirectedGraph &tree = pt->GetTree();

        for (; rit != ordering.rend(); ++rit) {
            NodeManager::GetNodeManager()->UTGarbageCollect();
            cout << "Combining functions in bucket " << *rit;
            cout << " (" << count++ << " of " << numBuckets << ")" << endl;

            /*
            buckets[*rit].PrintDiagrams(cout); cout << endl;
            buckets[*rit].PrintFunctionTables(cout); cout << endl;
            */
            AOMDDFunction *message = buckets[*rit].Flatten();
            buckets[*rit].PurgeFunctions();
            message->SetScopeOrdering(ordering);
            cout << "After flattening" << endl;

            /*
            message->Save(cout); cout << endl;
            message->PrintAsTable(cout); cout << endl;
            */

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
                if (*rit == largestBucket) {
                    tie(numMeta, numAND) = message->Size();
                    numTotal = numMeta + numAND;
                    mem = message->MemUsage();
                }
            }
            cout << "After eliminating " << *rit << endl;

            /*
            message->Save(cout); cout << endl;
            message->PrintAsTable(cout); cout << endl;
            */

            // empty scope, no need to send message, update final result
            if (message->GetScope().IsEmpty()) {
                Assignment a;
                if (logOut) {
                    pr += message->GetVal(a, logOut);
                    delete message;
                }
                else {
                    pr *= message->GetVal(a, logOut);
                    delete message;
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
                    delete message;
                }
                else {
                    pr *= message->GetVal(a, logOut);
                    delete message;
                }
            }
        }
        if (logOut) {
            pr += log10(globalWeight);
        }
        else {
            pr *= globalWeight;
        }
        NodeManager::GetNodeManager()->UTGarbageCollect();
    }
    return pr;
}

double CompileBucketTree::MPE(bool logOut) {
    double pr = logOut ? 0 : 1;

    // If it's already been compiled, we can just eliminate from the large DD
    if (compiled) {
        Assignment a;
        typedef std::map<int, int> map_t;
        BOOST_FOREACH(map_t::value_type i, evidence) {
            a.AddVar(i.first, i.second + 1);
            a.SetVal(i.first, i.second);
        }

        pr = compiledDD.Maximum(a);
        if (logOut) pr = log10(pr);

        /*
        const DirectedGraph &tree = pt->GetTree();
        AOMDDFunction probFunction(compiledDD);
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
                probFunction.Maximize(elim, false);
            }
            // At root
            if (ei == ei_end) {
                Assignment a;
                pr = probFunction.GetVal(a, logOut);
            }
        }
        */
    }

    // Otherwise, compile, but eliminate variables along the way
    else {
        ResetBuckets();
        list<int>::reverse_iterator rit = ordering.rbegin();
        int numBuckets = ordering.size();
        int count = 1;

        const DirectedGraph &tree = pt->GetTree();

        for (; rit != ordering.rend(); ++rit) {
            NodeManager::GetNodeManager()->UTGarbageCollect();
            cout << "Memory usage: " << NodeManager::GetNodeManager()->GetUTMemUsage() + NodeManager::GetNodeManager()->GetOCMemUsage() << endl;
            cout << "Combining functions in bucket " << *rit;
            cout << " (" << count++ << " of " << numBuckets << ")" << endl;

//            buckets[*rit].PrintDiagrams(cout); cout << endl;
//            buckets[*rit].PrintFunctionTables(cout); cout << endl;
            AOMDDFunction *message = buckets[*rit].Flatten();
            buckets[*rit].PurgeFunctions();
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
                message->Maximize(elim);
                if (*rit == largestBucket) {
                    tie(numMeta, numAND) = message->Size();
                    numTotal = numMeta + numAND;
                    mem = message->MemUsage();
                }
            }
            cout << "After eliminating " << *rit << endl;

//            message->Save(cout); cout << endl;
//            message->PrintAsTable(cout); cout << endl;

            // empty scope, no need to send message, update final result
            if (message->GetScope().IsEmpty()) {
                Assignment a;
                if (logOut) {
                    pr += message->GetVal(a, logOut);
                    delete message;
                }
                else {
                    pr *= message->GetVal(a, logOut);
                    delete message;
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
                    delete message;
                }
                else {
                    pr *= message->GetVal(a, logOut);
                    delete message;
                }
            }
        }
        if (logOut) {
            pr += log10(globalWeight);
        }
        else {
            pr *= globalWeight;
        }
        NodeManager::GetNodeManager()->UTGarbageCollect();
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
}

} // end of aomdd namespace
