#include "Scope.h"
#include "TableFunction.h"
#include "AOMDDFunction.h"
#include "BucketTree.h"
#include "CompileBucketTree.h"
#include "utils.h"
#include "base.h"
#include "Model.h"
#include "Graph.h"
#include "PseudoTree.h"
#include "MetaNode.h"
#include "NodeManager.h"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
using namespace aomdd;
using namespace std;

const unsigned long OUTPUT_COMPLEXITY_LIMIT = 2048;
time_t timeStart, timeEnd;
double timePassed;

list<int> parseOrder(string filename) {
    ifstream infile(filename.c_str());
    if (infile.fail()) {
        cerr << "Error opening file: " << filename << endl;
        exit(-1);
    }

    string buffer;
    int nv, intBuffer;
    getline(infile, buffer);

    infile >> nv;

    list<int> ordering;
    for (int i = 0; i < nv; i++) {
        infile >> intBuffer;
        ordering.push_front(intBuffer);
    }
    return ordering;
}

map<int, int> parseEvidence(string filename) {
    ifstream infile(filename.c_str());

    if (infile.fail()) {
        cerr << "Error opening file: " << filename << endl;
        exit(-1);
    }

    string buffer;
    int ne, intBuffer;
    getline(infile, buffer);

    infile >> ne;

    map<int, int> evidence;
    for (int i = 0; i < ne; i++) {
        infile >> intBuffer;
        int var = intBuffer;
        infile >> intBuffer;
        int val = intBuffer;
        cout << var << " " << val << endl;
        evidence.insert(make_pair<int, int> (var, val));
    }
    return evidence;
}

string inputFile, orderFile, evidFile, dotFile, outputResultFile;
bool compileMode, peMode, mpeMode, vbeMode, logMode;
bool outCompile;
bool verifyVals;

bool ParseCommandLine(int argc, char **argv) {
    bool haveInputFile = false;
    bool haveOrderingFile = false;
    for (int i = 1; i < argc; ++i) {
        string token(argv[i]);
        int len = token.length();
        if (token.substr(0, 1) == "-") {
            if (token.substr(1, len-1) == "f") {
                if (++i >= argc) return false;
                inputFile = string(argv[i]);
                haveInputFile = true;
            }
            else if (token.substr(1, len-1) == "o") {
                if (++i >= argc) return false;
                orderFile = string(argv[i]);
                haveOrderingFile = true;
            }
            else if (token.substr(1, len-1) == "e") {
                if (++i >= argc) return false;
                evidFile = string(argv[i]);
            }
            else if (token.substr(1, len-1) == "t") {
                if (++i >= argc) return false;
                dotFile = string(argv[i]);
            }
            else if (token.substr(1, len-1) == "r") {
                if (++i >= argc) return false;
                outputResultFile = string(argv[i]);
            }
            else if (token.substr(1, len-1) == "c") {
                compileMode = true;
            }
            else if (token.substr(1, len-1) == "pe") {
                peMode = true;
            }
            else if (token.substr(1, len-1) == "mpe") {
                mpeMode = true;
            }
            else if (token.substr(1, len-1) == "vbe") {
                vbeMode = true;
            }
            else if (token.substr(1, len-1) == "log") {
                logMode = true;
            }
            else if (token.substr(1, len-1) == "verify") {
                verifyVals = true;
            }
            else if (token.substr(1, len-1) == "outcompile") {
                outCompile = true;
            }
            else {
                return false;
            }
        }
        else {
            return false;
        }
    }
    if (!haveInputFile) {
        cerr << "Missing problem file." << endl;
        return false;
    }
    else if (!haveOrderingFile) {
        cerr << "Missing ordering file." << endl;
        return false;
    }
    if (peMode && mpeMode) {
        cerr << "Please choose a single query type (P(e) or MPE)." << endl;
        return false;
    }
    return true;
}

int main(int argc, char **argv) {
    cout << setprecision(15);
    cout << "====================================================" << endl;
    cout << "AOMDD-BE Compiler v0.1" << endl;
    cout << "  by William Lam, UC Irvine <willmlam@ics.uci.edu>" << endl;
    cout << "    (original algorithm by Robert Mateescu)" << endl;
    cout << "====================================================" << endl;
    cout << endl;

    if ( !ParseCommandLine(argc, argv) ) {
        cout << "Invalid arguments given" << endl;
        cout << "Options list" << endl;
        cout << "Input files:" << endl;
        cout << "  -f <file>        path to problem file (UAI format) [required]" << endl;
        cout << "  -o <file>        path to elimination ordering file [required]" << endl;
        cout << "  -e <file>        path to evidence file" << endl;
        cout << endl;
        cout << "Output files:" << endl;
        cout << "  -t <file>        path to DOT file to output generated pseudo-tree" << endl;
        cout << "  -r <file>        path to output results" << endl;
        cout << endl;
        cout << "Inference options:" << endl;
        cout << "  -c               compile full AOMDD" << endl;
        cout << "  -pe              compute P(e)" << endl;
        cout << "  -mpe             compute MPE(e) cost" << endl;
        cout << "  -vbe             use vanilla bucket elimination" << endl;
        cout << "  -log             output results in log space" << endl;
        cout << endl;
        cout << "Other options:" << endl;
        cout << "  -outcompile      output compiled AOMDD" << endl;
        return 0;
    }

    ofstream out;

    bool outputToFile = false;
    if (outputResultFile != "") {
        out.open(outputResultFile.c_str());
        outputToFile = true;
    }


    Model m;
    cout << "Reading from input file: " << inputFile << endl;
    if (outputToFile) {
        out << "Reading from input file: " << inputFile << endl;
    }

    m.parseUAI(inputFile);
    cout << "Reading from ordering file: " << orderFile << endl;
    if (outputToFile) {
        out << "Reading from ordering file: " << orderFile << endl;
    }
    list<int> ordering = parseOrder(orderFile);
    map<int, int> evidence;
    if (evidFile != "") {
        cout << "Reading from evidence file: " << evidFile << endl;
        if (outputToFile) {
            out << "Reading from evidence file: " << evidFile << endl;
        }
        evidence = parseEvidence(evidFile);
    }

    m.SetOrdering(ordering);

    Graph g(m.GetNumVars(), m.GetScopes());
    Scope completeScope = m.GetScopes()[0];
    for (unsigned int i = 1; i < m.GetScopes().size(); ++i) {
        completeScope = completeScope + m.GetScopes()[i];
    }
    g.InduceEdges(ordering);
    PseudoTree pt(g, completeScope);

    cout << endl << "Problem information:" << endl;

    cout << "n=" << m.GetNumVars() << endl;
    cout << "w=" << pt.GetInducedWidth() << endl;
    cout << "h=" << pt.GetHeight() << endl;
    cout << "k=" << m.GetMaxDomain() << endl;
    cout << "Number of functions=" << m.GetScopes().size() << endl;

    if (outputToFile) {
        out << endl << "Problem information:" << endl;
        out << "n=" << m.GetNumVars() << endl;
        out << "w=" << pt.GetInducedWidth() << endl;
        out << "h=" << pt.GetHeight() << endl;
        out << "k=" << m.GetMaxDomain() << endl;
        out << "Number of functions=" << m.GetScopes().size() << endl;
    }

    // Compute CM-graph space
    unsigned int numOR = 0;
    unsigned int numAND = 0;

    for (int i = 0; i < m.GetNumVars(); ++i) {
        int stateSpace = 1;
        BOOST_FOREACH(int v, pt.GetContexts()[i]) {
            if (v == -1) continue;
            stateSpace *= completeScope.GetVarCard(v);
        }
        numOR += stateSpace;
        numAND += stateSpace * completeScope.GetVarCard(i);
    }

    cout << "Number of OR nodes=" << numOR << endl;
    cout << "Number of AND nodes=" << numAND << endl;
    cout << "Total CM nodes=" << numOR + numAND << endl << endl;
    if (outputToFile) {
        out << "Number of OR nodes=" << numOR << endl;
        out << "Number of AND nodes=" << numAND << endl;
        out << "Total CM nodes=" << numOR + numAND << endl << endl;
    }

    /*
    // output contexts
    for (int i = 0; i < m.GetNumVars(); ++i) {
        cout << i << ":";
        BOOST_FOREACH(int v, pt.GetContexts()[i]) {
            if (v == -1) break;
            cout << " " << v;
        }
        cout << endl;
    }
    cout << endl;
    */

    if (dotFile != "") {
        cout << "Writing pseudo tree to: " << dotFile << endl;
        WriteDot(pt.GetTree(), dotFile);
    }


    CompileBucketTree *cbt = NULL;
    BucketTree *bt = NULL;

    if (vbeMode) {
        bt = new BucketTree(m, ordering, evidence);
    }
    else {
        cbt = new CompileBucketTree(m, &pt, ordering, evidence);
    }

    AOMDDFunction combined;
    if (compileMode) {
        time(&timeStart);
        combined = cbt->Compile();
        /*
        if (!evidence.empty()) {
            Assignment cond;
            map<int, int>::iterator it = evidence.begin();
            for(; it != evidence.end(); ++it) {
                int var = it->first;
                int assign = it->second;
                cond.AddVar(var, combined.GetScope().GetVarCard(var));
                cond.SetVal(var, assign);
            }
            combined.Condition(cond);
        }
        */
        time(&timeEnd);
        timePassed = difftime(timeEnd, timeStart);
        unsigned long totalCard = combined.GetScope().GetCard();
        if (outCompile && totalCard <= OUTPUT_COMPLEXITY_LIMIT) {
            combined.Save(cout); cout << endl;
            combined.PrintAsTable(cout); cout << endl;
        }

        int numMeta, numAND;
        tie(numMeta, numAND) = combined.Size();
        cout << endl;
        cout << "Time=" << timePassed << "s" << endl;
        cout << "Total flat state space=" << totalCard << endl;
        cout << "Number of AOMDD metanodes=" << numMeta << endl;
        cout << "Number of AOMDD AND nodes=" << numAND << endl;
        cout << "Total AOMDD nodes=" << numMeta + numAND << endl;
        cout << setprecision(2) << fixed;
        cout << "Compression ratio=" << double(numOR) / numMeta << endl;
        if (outputToFile) {
            out << endl;
            out << "Time=" << timePassed << "s" << endl;
            out << "Total flat state space=" << totalCard << endl;
            out << "Number of AOMDD metanodes=" << numMeta << endl;
            out << "Number of AOMDD AND nodes=" << numAND << endl;
            out << "Total AOMDD nodes=" << numMeta + numAND << endl;
            out << setprecision(2) << fixed;
            out << "Compression ratio=" << double(numOR) / numMeta;
        }
    }
    if (peMode) {
        double pr;
        time(&timeStart);
        if (vbeMode) {
            pr = bt->Prob(logMode);
        }
        else {
            pr = cbt->Prob(logMode);
        }
        time(&timeEnd);
        timePassed = difftime(timeEnd, timeStart);
        string prefix = logMode ? "log P(e)=" : "P(e)=";
        cout << endl;
        cout << "Time=" << timePassed << "s" << endl;
        cout << prefix << pr << endl;
        if (outputToFile) {
            out << endl;
            out << "Time=" << timePassed << "s" << endl;
            out << prefix << pr << endl;
        }
    }
    else if (mpeMode) {
        double pr;
        time(&timeStart);
        if (vbeMode) {
            pr = bt->MPE(logMode);
        }
        else {
            pr = cbt->MPE(logMode);
        }
        time(&timeEnd);
        timePassed = difftime(timeEnd, timeStart);
        string prefix = logMode ? "log MPE=" : "MPE=";
        cout << endl;
        cout << "Time=" << timePassed << "s" << endl;
        cout << prefix << pr << endl;
        if (outputToFile) {
            out << endl;
            out << "Time=" << timePassed << "s" << endl;
            out << prefix << pr << endl;
        }
    }

    if (verifyVals) {
        if (compileMode) {
            Assignment a(completeScope);
            a.SetAllVal(0);
            int misses = 0;
            for (unsigned int i = 0; i < OUTPUT_COMPLEXITY_LIMIT; i++) {
                double compVal = combined.GetVal(a, logMode);
                double flatVal = logMode ? 0 : 1;
                BOOST_FOREACH(const TableFunction &tf, m.GetFunctions()) {
                    if (logMode) {
                        flatVal += tf.GetVal(a, logMode);
                    }
                    else {
                        flatVal *= tf.GetVal(a, logMode);
                    }
                }
                cout << "cv=" << compVal << ", fv=" << flatVal;
                if (fabs(compVal - flatVal) > 1e-10) {
                    misses++;
                    cout << "...not matching!" << endl;
                }
                else {
                    cout << endl;
                }
                if(!a.Iterate()) break;
            }
            cout << "Number of incorrect values: " << misses << endl;
        }
    }

    unsigned int uniqueMetaNodes = NodeManager::GetNodeManager()->GetNumberOfNodes();
    unsigned int numANDNodes = NodeManager::GetNodeManager()->GetNumberOfANDNodes();
    unsigned int ocEntries = NodeManager::GetNodeManager()->GetNumberOfOpCacheEntries();
    if (uniqueMetaNodes > 0) {
        cout << endl;
        cout << "Number of nodes in cache=" << uniqueMetaNodes << endl;
        cout << "Number of AND nodes in cache=" << numANDNodes << endl;
        cout << "Number of nodes in op-cache=" << ocEntries << endl << endl;
        /*
    cout << "(Bucket count):" << NodeManager::GetNodeManager()->utBucketCount() << endl << endl;
    cout << "Bucket sizes:" << endl;
    NodeManager::GetNodeManager()->PrintUTBucketSizes(); cout << endl;
    cout << "Number of nodes in op-cache: "
            << NodeManager::GetNodeManager()->GetNumberOfOpCacheEntries() << endl << endl;
    cout << "(Bucket count):" << NodeManager::GetNodeManager()->ocBucketCount() << endl << endl;
         */
        if (outputToFile) {
            out << endl;
            out << "Number of nodes in cache=" << uniqueMetaNodes << endl;
            out << "Number of AND nodes in cache=" << numANDNodes << endl;
            out << "Number of nodes in op-cache=" << ocEntries << endl << endl;
        }
    }

    if (bt) delete bt;
    if (cbt) delete cbt;

    return 0;
}
