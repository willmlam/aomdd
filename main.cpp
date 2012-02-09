#include "Scope.h"
#include "TableFunction.h"
#include "AOMDDFunction.h"
#include "BucketTree.h"
#include "CompileBucketTree.h"
#include "DDMiniBucketTree.h"
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
#include <boost/program_options.hpp>
using namespace aomdd;
using namespace std;

namespace po = boost::program_options;

time_t timeStart, timeEnd;
double timePassed;
double MBLimit = 8192;
double OCMBLimit = 2048;

unsigned long mbeSizeBound = 0;
unsigned long mbeIBound = 0;

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
//    getline(infile, buffer);

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
bool miniBucketMode;
bool outCompile;
bool verifyVals;
bool vbeSpace;

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
            else if (token.substr(1, len-1) == "mlim") {
                if (++i >= argc) return false;
                MBLimit = atof(argv[i]);
            }
            else if (token.substr(1, len-1) == "oclim") {
                if (++i >= argc) return false;
                OCMBLimit = atof(argv[i]);
            }
            else if (token.substr(1, len-1) == "mbebound") {
                if (++i >= argc) return false;
                mbeSizeBound = atol(argv[i]);
            }
            else if (token.substr(1, len-1) == "mbeibound") {
                if (++i >= argc) return false;
                mbeIBound = atol(argv[i]);
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
            else if (token.substr(1, len-1) == "mbe") {
                miniBucketMode = true;
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
            else if (token.substr(1, len-1) == "vbespace") {
                vbeSpace = true;
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
    cout << "====================================================" << endl;
    cout << endl;

    po::options_description inputOptions("Input files");
    inputOptions.add_options()
            ("file,f", po::value<string>(), "path to problem file (UAI format) [required]")
            ("order,o", po::value<string>(), "path to elimination ordering file [required]")
            ("evid,e", po::value<string>(), "path to evidence file")
            ;
    po::options_description outputOptions("Output files");
    outputOptions.add_options()
            ("treedot,t", po::value<string>(), "path to DOT file to output generated pseudo-tree")
            ("res,r", po::value<string>(), "path to output results")
            ;
    po::options_description infOptions("Inference options");
    infOptions.add_options()
            ("compile,c", "compile full AOMDD")
            ("pe", "compute P(e)")
            ("mpe", "computer MPE(e) cost")
            ("mbe", "use minibucket approximation")
            ("mbebound", po::value<unsigned long>(), "diagram size bound for minibucket")
            ("mbeibound", po::value<unsigned long>(), "i-bound for minibucket")
            ("vbe", "use standard function tables")
            ("log", "output results in log space")
            ;
    po::options_description otherOptions("Other options");
    otherOptions.add_options()
            ("mlim", po::value<double>(), "Memory limit (MB) for nodes")
            ("oclim", po::value<double>(), "Memory limit (MB) for operation cache")
            ("outcompile", "Output compiled AOMDD")
            ("bespace", "Output space for standard table BE (only)")
            ("help,h", "Output list of options")
            ;

    /*
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
        cout << "  -mbe             use minibucket approximation" << endl;
        cout << "  -mbebound        size bound for minibuckets" << endl;
        cout << "  -mbeibound       i-bound for minibuckets" << endl;
        cout << "  -vbe             use vanilla bucket elimination" << endl;
        cout << "  -log             output results in log space" << endl;
        cout << endl;
        cout << "Other options:" << endl;
        cout << "  -mlim            specify memory limit (MB) for nodes" << endl;
//        cout << "  -oclim           specify memory limit (MB) for operations" << endl;
        cout << "  -outcompile      output compiled AOMDD" << endl;
        cout << "  -vbespace        compute space needed by vanilla bucket elimination (only)" << endl;
        return 0;
    }
    */

    po::options_description allOptions;
    allOptions.add(inputOptions).add(outputOptions).add(infOptions).add(otherOptions);

    po::variables_map vm;
    po::store(po::parse_command_line(argc,argv,allOptions),vm);
    po::notify(vm);


    if (vm.count("help")) {
        cout << allOptions << endl;
        return 1;
    }

    if (!vm.count("file") || !vm.count("order")) {
        cout << "Missing UAI file or ordering file" << endl;
        cout << allOptions << endl;
        return 1;
    }

    if (vm.count("pe") && vm.count("mpe")) {
        cout << "Please choose a single query type (P(e) or MPE)" << endl;
        cout << allOptions << endl;
        return 1;
    }

    try {
        inputFile = vm["file"].as<string>();
        orderFile = vm["order"].as<string>();
        if (vm.count("evid"))
            evidFile = vm["evid"].as<string>();
        if (vm.count("treedot"))
            dotFile = vm["treedot"].as<string>();
        if (vm.count("res"))
            outputResultFile = vm["res"].as<string>();
        compileMode = vm.count("compile");
        peMode = vm.count("pe");
        mpeMode = vm.count("mpe");
        vbeMode = vm.count("vbe");
        logMode = vm.count("log");
        miniBucketMode = vm.count("mbe");
        outCompile = vm.count("outcompile");
        vbeSpace = vm.count("bespace");

        if (vm.count("mlim"))
            MBLimit = vm["mlim"].as<double>();
        if (vm.count("oclim"))
            OCMBLimit = vm["oclim"].as<double>();
        if (vm.count("mbebound"))
            mbeSizeBound = vm["mbebound"].as<unsigned long>();
        if (vm.count("mbeibound"))
            mbeIBound = vm["mbeibound"].as<unsigned long>();
    } catch (const std::exception &e) {
        cout << allOptions << endl;
        return 0;
    }

    ofstream out;
    out << setprecision(15);

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
    const Scope &completeScope = m.GetCompleteScope();
    g.InduceEdges(ordering);
    PseudoTree pt(g, completeScope);
//    m.ApplyEvidence(evidence);

    cout << endl << "Problem information:" << endl;

    cout << "n=" << m.GetNumVars() << endl;
    cout << "w=" << pt.GetInducedWidth() << endl;
    cout << "h=" << pt.GetHeight() << endl;
    cout << "k=" << m.GetMaxDomain() << endl;
    cout << "Number of functions=" << m.GetScopes().size() << endl << endl;

    if (outputToFile) {
        out << endl << "Problem information:" << endl;
        out << "n=" << m.GetNumVars() << endl;
        out << "w=" << pt.GetInducedWidth() << endl;
        out << "h=" << pt.GetHeight() << endl;
        out << "k=" << m.GetMaxDomain() << endl;
        out << "Number of functions=" << m.GetScopes().size() << endl << endl;
    }

    // Compute CM-graph space
    unsigned long numOR = 0;
    unsigned long numAND = 0;

    unsigned long largestMessageSize = 0;
    unsigned int bucketID = 0;

    for (int i = 0; i < m.GetNumVars(); ++i) {
        unsigned long stateSpace = 1;
        BOOST_FOREACH(int v, pt.GetContexts()[i]) {
            if (v == -1) continue;
            stateSpace *= completeScope.GetVarCard(v);
        }
        numOR += stateSpace;
        unsigned long totalSpace = stateSpace * completeScope.GetVarCard(i);
        if (stateSpace > largestMessageSize) {
            largestMessageSize = stateSpace;
            bucketID = i;
        }
        numAND += totalSpace;
    }

    cout << "CM AO Graph Information:" << endl;
    cout << "Number of OR nodes=" << numOR << endl;
    cout << "Number of AND nodes=" << numAND << endl;
    cout << "Total CM nodes=" << numOR + numAND << endl << endl;

    cout << "Space information:" << endl;
    cout << "Largest message size=" << largestMessageSize << endl;
    cout << "Memory (MBytes)=" << (double)largestMessageSize * 8 / pow(2.0, 20) << endl << endl;
    if (outputToFile) {
        out << "CM AO Graph Information:" << endl;
        out << "Number of OR nodes=" << numOR << endl;
        out << "Number of AND nodes=" << numAND << endl;
        out << "Total CM nodes=" << numOR + numAND << endl << endl;

        out << "Space information:" << endl;
        out << "Largest message size=" << largestMessageSize << endl;
        out << "Memory (MBytes)=" << (double)largestMessageSize * 8 / pow(2.0, 20) << endl << endl;
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
    DDMiniBucketTree *mbt = NULL;

    cout << "MB Limit=" << MBLimit << endl;
    cout << "OC MB Limit=" << OCMBLimit << endl;
    if (outputToFile) {
        out << "MB Limit=" << MBLimit << endl;
        out << "OC MB Limit=" << OCMBLimit << endl;
    }

    if (vbeMode) {
        cout << "Starting vanilla BE..." << endl;
        if (double(largestMessageSize) * 8 / pow(2.0, 20) > MBLimit && !vbeSpace) {
            cout << "Largest message exceeds memory bound." << endl;
            if (outputToFile) {
                out << "Largest message exceeds memory bound." << endl;
            }
//            return 0;
        }
        bt = new BucketTree(m, ordering, evidence);
	    if (vbeSpace) {
	        cout << "Simulating for max number of entries..." << endl;
	        long maxEntries = bt->ComputeMaxEntriesInMemory();
	        cout << "Max entries: " << maxEntries << endl;
	        cout << "Max entries (MB): " << maxEntries * 8 / pow(2.0, 20) << endl;
	        return 0;
	    }
    }
    else {
        NodeManager::GetNodeManager()->SetMBLimit(MBLimit);
        NodeManager::GetNodeManager()->SetOCMBLimit(OCMBLimit);
        if (miniBucketMode && (mbeSizeBound > 0 || mbeIBound > 0)) {
	        cout << "Starting AOMDD-MBE..." << endl;
	        if (mbeSizeBound > 0) {
		        mbt = new DDMiniBucketTree(m, &pt, evidence, bucketID, mbeSizeBound);
		        mbt->SetPartitionMetric(DIAGRAM_SIZE);
	        }
	        else if (mbeIBound > 0) {
		        mbt = new DDMiniBucketTree(m, &pt, evidence, bucketID, mbeIBound);
		        mbt->SetPartitionMetric(I_BOUND);
	        }
	        else {
	            // should not get here
	            cerr << "No bound set, exiting..." << endl;
	            exit(0);
	        }
        }
        else {
            if (miniBucketMode) {
                cout << "No bound set, switching to standard BE." << endl;
                miniBucketMode = false;
            }
	        cout << "Starting AOMDD-BE..." << endl;
	        cbt = new CompileBucketTree(m, &pt, ordering, evidence, bucketID);
        }
        unsigned int uniqueMetaNodes = NodeManager::GetNodeManager()->GetNumberOfNodes();
        unsigned int numANDNodes = NodeManager::GetNodeManager()->GetNumberOfANDNodes();
//        unsigned int ocEntries = NodeManager::GetNodeManager()->GetNumberOfOpCacheEntries();
        double utMemUsage = NodeManager::GetNodeManager()->MemUsage();
//        double opMemUsage = NodeManager::GetNodeManager()->OpCacheMemUsage();
        if (uniqueMetaNodes > 0) {
            cout << endl;
            cout << "Initial Number of nodes in cache=" << uniqueMetaNodes << endl;
            cout << "Initial Number of AND nodes in cache=" << numANDNodes << endl;
            //        cout << "Number of nodes in op-cache=" << ocEntries << endl << endl;

            cout << "Initial cache memory (MBytes)=" << utMemUsage << endl << endl;
            //        cout << "op-cache memory (MBytes)=" << opMemUsage << endl << endl;
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
                out << "Initial Number of nodes in cache=" << uniqueMetaNodes << endl;
                out << "Initial Number of AND nodes in cache=" << numANDNodes << endl;
                //            out << "Number of nodes in op-cache=" << ocEntries << endl << endl;

                out << "Initial cache memory (MBytes)=" << utMemUsage << endl << endl;
                //            out << "op-cache memory (MBytes)=" << opMemUsage << endl << endl;
            }
        }
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
        double totalLogCard = combined.GetScope().GetLogCard();
        map<int, unsigned int> cardExp = combined.GetScope().GetCardExp();
        if (outCompile && totalCard <= OUTPUT_COMPLEXITY_LIMIT && totalLogCard <= log(OUTPUT_COMPLEXITY_LIMIT)) {
            combined.Save(cout); cout << endl;
            combined.PrintAsTable(cout); cout << endl;
        }

        vector<unsigned int> numMeta;
        vector<unsigned int> numANDMDD;
        vector<double> varESemanticWidth(m.GetNumVars(), 0.0);
        tie(numMeta, numANDMDD) = combined.GetCounts(m.GetNumVars());

        // Compute semantic widths
        /*
        for (int i = 0; i < m.GetNumVars(); ++i) {
            // Compute log-average of domain size in context
            double logSum = 0;
            BOOST_FOREACH(int v, pt.GetContexts()[i]) {
                logSum += log(double(completeScope.GetVarCard(v)));
            }
            if (pt.GetContexts()[i].size() == 1 && pt.GetContexts()[i].count(-1) == 1) {
	            varESemanticWidth[i] = 0;
            }
            else {
                double logAvgDomain = logSum / pt.GetContexts()[i].size();
                if (logSum == 0) {
                    varESemanticWidth[i] = 0;
                }
                else {
                    varESemanticWidth[i] = log(double(numMeta[i])) / logAvgDomain;
                }
            }
        }
        */
        for (int i = 0; i < m.GetNumVars(); ++i) {
            if (pt.GetContexts()[i].size() == 1 && pt.GetContexts()[i].count(-1) == 1) {
	            varESemanticWidth[i] = 0;
            }
            else {
                varESemanticWidth[i] = log(double(numMeta[i]));
            }
        }

        /*
        for (unsigned int i = 0; i < numMeta.size(); ++i) {
            cout << i << ":" << numMeta[i] << endl;
        }
        cout << endl;
        for (unsigned int i = 0; i < varSemanticWidth.size(); ++i) {
            cout << "sw(" << i << ")=" << varSemanticWidth[i] << endl;
        }
        */

        double probESemanticWidth = 0;
        BOOST_FOREACH(double w, varESemanticWidth) {
            if (w > probESemanticWidth) {
                probESemanticWidth = w;
            }
        }


        unsigned int countMeta = 0;
        unsigned int countAND = 0;
        BOOST_FOREACH(int i, numMeta) {
            countMeta += i;
        }
        BOOST_FOREACH(int i, numANDMDD) {
            countAND += i;
        }
//        tie(countMeta, countAND) = combined.Size();
        cout << endl;
        cout << "Time=" << timePassed << "s" << endl;
        cout << "Total flat state space=" << totalCard << endl;
        cout << "Total log flat state space=" << totalLogCard << endl;
        typedef map<int, unsigned int> map_t;
        BOOST_FOREACH(map_t::value_type &c, cardExp) {
            cout << " " << c.first << "^" << c.second;
        }
        cout << endl;
        cout << "Number of AOMDD metanodes=" << countMeta << endl;
        cout << "Number of AOMDD AND nodes=" << countAND << endl;
        cout << "Total AOMDD nodes=" << countMeta + countAND << endl;
        cout << "Compression ratio (wrtOR)=" << double(numOR) / countMeta << endl;
        cout << "Compression ratio (wrtAND)=" << double(numAND) / countAND << endl;
        cout << "AOMDD Memory (MBytes)=" << combined.MemUsage() / (1024.0*1024) << endl;
        cout << "Effective semantic width=" << probESemanticWidth << endl;
        if (outputToFile) {
            out << endl;
            out << "Time=" << timePassed << "s" << endl;
            out << "Total flat state space=" << totalCard << endl;
	        out << "Total log flat state space=" << totalLogCard << endl;
            out << "Number of AOMDD metanodes=" << countMeta << endl;
            out << "Number of AOMDD AND nodes=" << countAND << endl;
            out << "Total AOMDD nodes=" << countMeta + countAND << endl;
            out << "Compression ratio (wrtOR)=" << double(numOR) / countMeta << endl;
            out << "Compression ratio (wrtAND)=" << double(numAND) / countAND << endl;
            out << "AOMDD Memory (MBytes)=" << combined.MemUsage() / (1024.0*1024) << endl;
            out << "Effective semantic width=" << probESemanticWidth << endl;
        }
    }

    QueryType q;
    if (peMode) {
        q = PE;
    }
    else if (mpeMode) {
        q = MPE;
    }

    double pr;
    time(&timeStart);
    if (vbeMode) {
        pr = bt->Query(q, logMode);
    }
    else if (miniBucketMode) {
        pr = mbt->Query(q, logMode);
        cout << endl;
        cout << "Largest Message (AOMDD Meta)=" << mbt->GetLargestNumMeta() << endl;
        cout << "Largest Message (AOMDD AND)=" << mbt->GetLargestNumAND() << endl;
        cout << "Largest Message (AOMDD Total)= " << mbt->GetLargestNumTotal() << endl;
        cout << "Largest Message (AOMDD Memory)=" << mbt->GetLargestMem() << endl;
        if (outputToFile) {
            out << endl;
            out << "Largest Message (AOMDD Meta)=" << mbt->GetLargestNumMeta() << endl;
            out << "Largest Message (AOMDD AND)=" << mbt->GetLargestNumAND() << endl;
            out << "Largest Message (AOMDD Total)= " << mbt->GetLargestNumTotal() << endl;
            out << "Largest Message (AOMDD Memory)=" << mbt->GetLargestMem() << endl;
        }
    }
    else {
        pr = cbt->Query(q, logMode);
        if (!compileMode) {
	        cout << endl;
	        cout << "Largest Message (AOMDD Meta)=" << cbt->GetLargestNumMeta() << endl;
	        cout << "Largest Message (AOMDD AND)=" << cbt->GetLargestNumAND() << endl;
	        cout << "Largest Message (AOMDD Total)= " << cbt->GetLargestNumTotal() << endl;
	        cout << "Largest Message (AOMDD Memory)=" << cbt->GetLargestMem() << endl;
	        if (outputToFile) {
	            out << endl;
	            out << "Largest Message (AOMDD Meta)=" << cbt->GetLargestNumMeta() << endl;
	            out << "Largest Message (AOMDD AND)=" << cbt->GetLargestNumAND() << endl;
	            out << "Largest Message (AOMDD Total)= " << cbt->GetLargestNumTotal() << endl;
	            out << "Largest Message (AOMDD Memory)=" << cbt->GetLargestMem() << endl;
	        }
        }
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
//    unsigned int ocEntries = NodeManager::GetNodeManager()->GetNumberOfOpCacheEntries();
//    double utMemUsage = NodeManager::GetNodeManager()->MemUsage();
    double utMemUsage = NodeManager::GetNodeManager()->GetUTMemUsage();
//    double opMemUsage = NodeManager::GetNodeManager()->OpCacheMemUsage();
    if (uniqueMetaNodes > 0) {
        cout << endl;
        cout << "Number of nodes in cache=" << uniqueMetaNodes << endl;
        cout << "Number of AND nodes in cache=" << numANDNodes << endl;
//        cout << "Number of nodes in op-cache=" << ocEntries << endl << endl;

        cout << "UT memory (MBytes)=" << utMemUsage << endl << endl;
//        cout << "op-cache memory (MBytes)=" << opMemUsage << endl << endl;
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
//            out << "Number of nodes in op-cache=" << ocEntries << endl << endl;

            out << "UT memory (MBytes)=" << utMemUsage << endl << endl;
//            out << "op-cache memory (MBytes)=" << opMemUsage << endl << endl;
        }
    }

    if (bt) delete bt;
    if (cbt || mbt) {
        double maxUT = NodeManager::GetNodeManager()->GetMaxUTMemUsage();
        double maxOC = NodeManager::GetNodeManager()->GetMaxOCMemUsage();
        cout << "MAX UT mem=" << maxUT << endl;
        cout << "MAX OC mem=" << maxOC << endl;
        if (outputToFile) {
            out << "MAX UT mem=" << maxUT << endl;
            out << "MAX OC mem=" << maxOC << endl;
        }
        if (cbt)
	        delete cbt;
        if (mbt)
            delete mbt;
    }
    return 0;
}
