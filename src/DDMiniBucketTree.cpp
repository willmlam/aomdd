/*
 *  DDMiniBucketTree.cpp
 *  aomdd
 *
 *  Created by William Lam on Jul 15, 2011
 *  Copyright 2011 UC Irvine. All rights reserved.
 *
 */

#include "DDMiniBucketTree.h"
#include "TableFunction.h"
using namespace std;

namespace aomdd {

DDMiniBucketTree::DDMiniBucketTree() : compiled(false), globalWeight(1.0) {
}

DDMiniBucketTree::DDMiniBucketTree(const Model &m, const PseudoTree *ptIn,
        const map<int, int> &evidIn, int bucketID, unsigned long bound)
        : pt(ptIn), ordering(m.GetOrdering()), evidence(evidIn), largestBucket(bucketID),
        compiled(false), globalWeight(1.0) {

    int numBuckets = ordering.size();
    if (pt->HasDummy()) {
        ordering.push_front(numBuckets++);
    }

    buckets.resize(numBuckets);
    interMessages.resize(numBuckets);
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
        buckets[i].SetBound(bound);
    }
    numMeta = 0;
    numAND = 0;
    numTotal = 0;
    mem = 0;
}

AOMDDFunction DDMiniBucketTree::Compile() {
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

//            AOMDDFunction *message = buckets[*rit].Flatten();
            vector<AOMDDFunction *> messages = buckets[*rit].GenerateMessages();
            buckets[*rit].PurgeFunctions();
            for (unsigned int i = 0; i < messages.size(); ++i) {
	            messages[i]->SetScopeOrdering(ordering);
            }

            /*
            message->Save(cout); cout << endl;
            message->PrintAsTable(cout); cout << endl;
            */

            DInEdge ei, ei_end;
            tie(ei, ei_end) = in_edges(*rit, tree);
            // Not at root
            if (ei != ei_end) {
                int parent = source(*ei, tree);
                cout << "Sending messages from <" << *rit << "> to <" << parent << ">" << endl;
                for (unsigned int i = 0; i < messages.size(); ++i) {
                    buckets[parent].AddFunction(messages[i]);
                }
            }
            // At root
            else {
                compiledDD = *messages[0];
                for (unsigned int i = 1; i < messages.size(); ++i) {
                    compiledDD.Multiply(*messages[i]);
                }
            }
        }
        NodeManager::GetNodeManager()->PurgeOpCache();
        NodeManager::GetNodeManager()->UTGarbageCollect();
    }
    compiled = true;
    compiledDD.ReweighRoot(globalWeight);
    return compiledDD;
}

double DDMiniBucketTree::Query(QueryType q, bool logOut) {
    double pr = logOut ? 0 : 1;

    // If it's already been compiled, we can just eliminate from the large DD
    if (compiled) {
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
        else {
            cerr << "Invalid query type!" << endl;
            exit(0);
        }
        if (logOut) pr = log10(pr);

    }

    // Otherwise, compile, but eliminate variables along the way
    else {
        ResetBuckets();
        list<int>::reverse_iterator rit = ordering.rbegin();
//        int numBuckets = ordering.size();
//        int count = 1;

        const DirectedGraph &tree = pt->GetTree();
        bool encPart = false;

        for (; rit != ordering.rend(); ++rit) {
            cout << "." ;
            cout.flush();
            /*
            cout << "Memory usage: " << NodeManager::GetNodeManager()->GetUTMemUsage() + NodeManager::GetNodeManager()->GetOCMemUsage() << endl;
            cout << "Combining functions in bucket " << *rit;
            cout << " (" << count++ << " of " << numBuckets << ")" << endl;
            */

            /*
            buckets[*rit].PrintDiagrams(cout); cout << endl;
            buckets[*rit].PrintFunctionTables(cout); cout << endl;
            */
            vector<AOMDDFunction *> messages = buckets[*rit].GenerateMessages();
                buckets[*rit].PurgeFunctions();
            for (unsigned int i = 0; i < messages.size(); ++i) {
	            messages[i]->SetScopeOrdering(ordering);
            }
//            cout << "After flattening" << endl;

//            message->Save(cout); cout << endl;
//            message->PrintAsTable(cout); cout << endl;

            if (messages.size() > 1) encPart = true;

            // will need to repeat for each partition start here
            for (unsigned int i = 0; i < messages.size(); ++i) {
                AOMDDFunction *message = messages[i];
//                message->PrintAsTable(cout);
//                if (encPart)
//	                cin.get();

                DInEdge ei, ei_end;
                tie(ei, ei_end) = in_edges(*rit, tree);
                Scope elim;
                int card = message->GetScope().GetVarCard(*rit);
                elim.AddVar(*rit, card);

                Scope newScope = message->GetScope();
                newScope = newScope - elim;

                map<int, int>::iterator eit = evidence.find(*rit);

                if (eit != evidence.end()) {
                    Assignment cond(elim);
                    cond.SetVal(*rit, eit->second);
                    //                cout << "Conditioning with "; cond.Save(cout); cout << endl;
                    message->Condition(cond);
                }
                else {
                    if (q == PE && i == 0) {
                        message->Marginalize(elim);
                    }
                    else if (q == MPE || (q == PE && i > 0)) {
                        message->Maximize(elim);
                    }
                    else {
                        cerr << "Invalid query type!" << endl;
                        exit(0);
                    }
                    if (*rit == largestBucket) {
                        long tnumMeta, tnumAND, tnumTotal;
                        double tmem;

                        tie(tnumMeta, tnumAND) = message->Size();
                        tnumTotal = tnumMeta + tnumAND;
                        tmem = message->MemUsage() / MB_PER_BYTE;

                        numMeta += tnumMeta;
                        numAND += tnumAND;
                        numTotal += tnumTotal;
                        mem += tmem;
                    }
                }
//                cout << "After eliminating " << *rit << endl;

                //            message->Save(cout); cout << endl;
//                            message->PrintAsTable(cout); cout << endl;

                // empty scope, no need to send message, update final result
                if (newScope.IsEmpty()) {
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
                else {
                    int parent = newScope.GetOrdering().back();
//                    cout << "Sending message from <" << *rit << "> to <" << parent << ">" << endl;
                    interMessages[parent].push_back(new AOMDDFunction(*message));
                    buckets[parent].AddFunction(message);
                }
            }
        }
        cout << "done."<< endl;
        treeCompiled = true;
        if (logOut) {
            pr += log10(globalWeight);
        }
        else {
            pr *= globalWeight;
        }
//        NodeManager::GetNodeManager()->PurgeOpCache();
        NodeManager::GetNodeManager()->UTGarbageCollect();
    }
    if (logOut)
        logPR = pr;
    else
        logPR = log10(pr);
    return pr;
}

double DDMiniBucketTree::GetHeur(int var, const Assignment &a) const {
    double h = 1;
    if (!treeCompiled) return h;
    assert(var >= 0 && var < int(buckets.size()));
    BOOST_FOREACH(const AOMDDFunction *f, interMessages[var]) {
        h *= f->GetVal(a);
    }
    return h;
}

void DDMiniBucketTree::PrintBucketFunctionScopes(ostream &out) const {
    for (unsigned int i = 0; i < buckets.size(); ++i) {
        out << "Bucket " << i << ":" << endl;
        buckets[i].PrintFunctionScopes(out);
        out << endl;
    }
}

void DDMiniBucketTree::PrintBuckets(ostream &out) const {
    BOOST_FOREACH(const DDMiniBucket &b, buckets) {
        b.PrintDiagrams(out); out << endl;
    }
    out << endl;
}

void DDMiniBucketTree::ResetBuckets() {
    for (unsigned int i = 0; i < buckets.size(); ++i) {
        buckets[i].ResizeBucket(initialBucketSizes[i]);
    }
}

DDMiniBucketTree::~DDMiniBucketTree() {
}

} // end of aomdd namespace
