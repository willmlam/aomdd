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

    // Use the pseudotree to build the descendant sets
    descendants.resize(ordering.size());

    // Terminals are always descendants of any variable
    BOOST_FOREACH(set<int> &dSet, descendants) {
        dSet.insert(-1);
        dSet.insert(-2);
    }
    DescendantGenerator vis(descendants);
    depth_first_search(pt->GetTree(), root_vertex(VertexDesc(pt->GetRoot())).
            visitor(vis).
            edge_color_map(get(edge_color,pt->GetTree())));
}

AOMDDFunction CompileBucketTree::Compile() {
    NodeManager::GetNodeManager()->SetDescendantsList(&descendants);
    NodeManager::GetNodeManager()->SetOrdering(&ordering);
    if (!compiled) {
        list<int>::reverse_iterator rit = ordering.rbegin();
        int numBuckets = ordering.size();
        int count = 1;

        const DirectedGraph &tree = pt->GetTree();

        for (; rit != ordering.rend(); ++rit) {
            // Try this for now (is it worth it for full compilation?)
            NodeManager::GetNodeManager()->UTGarbageCollect();
            cout << "."; cout.flush();

            /*
            cout << "Memory usage: " << NodeManager::GetNodeManager()->GetUTMemUsage() + NodeManager::GetNodeManager()->GetOCMemUsage() << endl;

            cout << "Combining functions in bucket " << *rit;
            cout << " (" << count++ << " of " << numBuckets << ")" << endl;
            */

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
//                cout << "Sending message from <" << *rit << "> to <" << parent << ">" << endl;
                buckets[parent].AddFunction(message);
            }
            // At root
            else {
                compiledDD = *message;
            }
        }
        NodeManager::GetNodeManager()->UTGarbageCollect();
    }
    cout << "compiled." << endl;
    compiled = true;
    compiledDD.ReweighRoot(globalWeight);

    NodeManager::GetNodeManager()->PurgeOpCache();
    NodeManager::GetNodeManager()->SetDescendantsList(NULL);
    NodeManager::GetNodeManager()->SetOrdering(NULL);
    return compiledDD;
}

double CompileBucketTree::Query(QueryType q, bool logOut) {
    double pr = logOut ? 0 : 1;

    NodeManager::GetNodeManager()->SetDescendantsList(&descendants);
    NodeManager::GetNodeManager()->SetOrdering(&ordering);
    // If it's already been compiled, we can just eliminate from the large DD
    if (compiled) {
        cout << "AOMDD already compiled, querying AOMDD directly" << endl;
        Assignment a;
        typedef std::map<int, int> map_t;
        BOOST_FOREACH(map_t::value_type i, evidence) {
            a.AddVar(i.first, i.second + 1);
            a.SetVal(i.first, i.second);
        }

        if (q == PE) {
	        pr = compiledDD.Sum(a);
        }
        else if (q == MPE) {
            pr = compiledDD.Maximum(a);
        }

        if (logOut) pr = log10(pr);

    }

    // Otherwise, compile, but eliminate variables along the way
    else {
        ResetBuckets();
        list<int>::reverse_iterator rit = ordering.rbegin();
        int numBuckets = ordering.size();
        int count = 1;

        const DirectedGraph &tree = pt->GetTree();

        for (; rit != ordering.rend(); ++rit) {
            cout << "."; cout.flush();
//            cout << "Memory usage: " << NodeManager::GetNodeManager()->GetUTMemUsage() + NodeManager::GetNodeManager()->GetOCMemUsage() << endl;
//            cout << "Combining functions in bucket " << *rit;
//            cout << " (" << count++ << " of " << numBuckets << ")" << endl;

//            buckets[*rit].PrintDiagrams(cout); cout << endl;
//            buckets[*rit].PrintFunctionTables(cout); cout << endl;
            AOMDDFunction *message = buckets[*rit].Flatten();
            buckets[*rit].PurgeFunctions();
            message->SetScopeOrdering(ordering);
//            cout << "After flattening" << endl;

//            message->Save(cout); cout << endl;
//            message->PrintAsTable(cout); cout << endl;

            DInEdge ei, ei_end;
            tie(ei, ei_end) = in_edges(*rit, tree);
            Scope elim;

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
                continue;
            }
            int card = message->GetScope().GetVarCard(*rit);
            elim.AddVar(*rit, card);

            map<int, int>::iterator eit = evidence.find(*rit);
//            if (*rit == 452) buckets[122].PrintFunctionTables(cout);

            if (eit != evidence.end()) {
                Assignment cond(elim);
                cond.SetVal(*rit, eit->second);
//                cout << "Conditioning with "; cond.Save(cout); cout << endl;
                message->ConditionFast(cond);
            }
            else {
                if (q == PE) {
	                message->MarginalizeFast(elim);
                }
                else if (q == MPE) {
                    message->MaximizeFast(elim);
                }
                if (*rit == largestBucket) {
                    tie(numMeta, numAND) = message->Size();
                    numTotal = numMeta + numAND;
                    mem = message->MemUsage() / MB_PER_BYTE;
                }
            }
//            cout << "After eliminating " << *rit << endl;

//            message->Save(cout); cout << endl;
//           message->PrintAsTable(cout); cout << endl;

//            if (*rit == 452) buckets[122].PrintFunctionTables(cout);

//            if (*rit == 286) exit(0);

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
            /*
            // message is a constant value
            else if (message->IsConstantValue() && ei != ei_end) {
                int parent = source(*ei, tree);
//                cout << "Removing irrelevant bucket" << endl;
                buckets[parent].Reweigh(message->GetRootWeight());
                delete message;
            }
            */
            // Not at root
            else if (ei != ei_end) {
                int bParent = message->GetScope().GetOrdering().back();
//                cout << "True bParent: " << bParent << endl;
//                int parent = source(*ei, tree);
//                cout << "Sending message from <" << *rit << "> to <" << bParent << ">" << endl;
                buckets[bParent].AddFunction(message);
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

//            if (count-1 == 69) exit(0);
        }
        if (logOut) {
            pr += log10(globalWeight);
        }
        else {
            pr *= globalWeight;
        }
//        NodeManager::GetNodeManager()->PurgeOpCache();
        NodeManager::GetNodeManager()->UTGarbageCollect();
    }
    cout << "done." << endl;
    NodeManager::GetNodeManager()->PurgeOpCache();
    NodeManager::GetNodeManager()->SetDescendantsList(NULL);
    NodeManager::GetNodeManager()->SetOrdering(NULL);
    return pr;
}

/*
double CompileBucketTree::Prob(bool logOut) {
    double pr = logOut ? 0 : 1;

    NodeManager::GetNodeManager()->SetDescendantsList(&descendants);
    NodeManager::GetNodeManager()->SetOrdering(&ordering);
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

    }

    // Otherwise, compile, but eliminate variables along the way
    else {
        ResetBuckets();
        list<int>::reverse_iterator rit = ordering.rbegin();
        int numBuckets = ordering.size();
        int count = 1;

        const DirectedGraph &tree = pt->GetTree();

        for (; rit != ordering.rend(); ++rit) {
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
                continue;
            }
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
                message->MarginalizeFast(elim);
                if (*rit == largestBucket) {
                    tie(numMeta, numAND) = message->Size();
                    numTotal = numMeta + numAND;
                    mem = message->MemUsage() / MB_PER_BYTE;
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
            // message is a constant value
            else if (false &&message->IsConstantValue() && ei != ei_end) {
                int parent = source(*ei, tree);
                cout << "Removing irrelevant bucket" << endl;
                buckets[parent].Reweigh(message->GetRootWeight());
                delete message;
            }
            // Not at root
            else if (ei != ei_end) {
                int bParent = message->GetScope().GetOrdering().back();
                cout << "True bParent: " << bParent << endl;
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
        NodeManager::GetNodeManager()->PurgeOpCache();
        NodeManager::GetNodeManager()->UTGarbageCollect();
    }
    NodeManager::GetNodeManager()->SetDescendantsList(NULL);
    NodeManager::GetNodeManager()->SetOrdering(NULL);
    return pr;
}

double CompileBucketTree::MPE(bool logOut) {
    double pr = logOut ? 0 : 1;

    NodeManager::GetNodeManager()->SetDescendantsList(&descendants);
    NodeManager::GetNodeManager()->SetOrdering(&ordering);

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

    }

    // Otherwise, compile, but eliminate variables along the way
    else {
        ResetBuckets();
        list<int>::reverse_iterator rit = ordering.rbegin();
        int numBuckets = ordering.size();
        int count = 1;

        const DirectedGraph &tree = pt->GetTree();

        for (; rit != ordering.rend(); ++rit) {
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
                continue;
            }
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
//	                message->Save(cout); cout << endl;
//	                message->PrintAsTable(cout); cout << endl;
                message->MaximizeFast(elim);
//	                message->Save(cout); cout << endl;
                if (*rit == largestBucket) {
                    tie(numMeta, numAND) = message->Size();
                    numTotal = numMeta + numAND;
                    mem = message->MemUsage() / MB_PER_BYTE;
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
            NodeManager::GetNodeManager()->PurgeOpCache();
            NodeManager::GetNodeManager()->UTGarbageCollect();
        }
        if (logOut) {
            pr += log10(globalWeight);
        }
        else {
            pr *= globalWeight;
        }
    }
    NodeManager::GetNodeManager()->SetDescendantsList(NULL);
    NodeManager::GetNodeManager()->SetOrdering(NULL);
    return pr;
}
*/

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
