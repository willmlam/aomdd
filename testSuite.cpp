/*
 * testSuite.cc
 *
 *  Created on: Jan 27, 2012
 *      Author: willmlam
 */


#include "NodeManager.h"
#include "AOMDDFunction.h"
#include "Scope.h"
#include <vector>
#include <sys/time.h>
#include <sys/resource.h>
#include <boost/shared_ptr.hpp>
using namespace std;
using namespace aomdd;

#define printmemory {getrusage(who, &ru); cout << "rusage: " << ru.ru_maxrss - start << endl;}
typedef boost::shared_ptr<AOMDDFunction> AOMDDFunctionPtr;

vector<double> CreateNormalizedRandomVector(int n) {
    vector<double> ret;
    for (int i = 0; i < n; ++i) {
        ret.push_back((rand() % 100) / 100.0);
    }
    double sum = 0.0;
    for (int i = 0; i < n; ++i) {
        sum += ret[i];
    }
    for (int i = 0; i < n; ++i) {
        ret[i] /= sum;
    }
    return ret;
}

vector<double> CreateBernoulli() {
    vector<double> ret;
    double p = (rand() % 100000) / 100000.0;
    ret.push_back(p);
    ret.push_back(1-p);
    return ret;
}

template<typename T>
ostream& operator<<(ostream &out, const vector<T> &vec) {
    for (int i = 0; i < int(vec.size()); ++i) {
        out << " " << vec[i];
    }
    return out;
}

int main2() {
    srand(time(NULL));
    struct rusage ru;
    int who = RUSAGE_SELF;
    getrusage(who, &ru);
    int start = ru.ru_maxrss;
    cout << "Before scope" << endl;
    printmemory

    Scope s;
    s.AddVar(0, 2);
    s.AddVar(1, 2);

    Scope s2;
    s2.AddVar(0, 2);
    cout << "After scope" << endl;
    printmemory

    vector<double> vals;
    vals.push_back(0.5);
    vals.push_back(0.5);
    vals.push_back(0.5);
    vals.push_back(0.5);

    vector<double> vals2;
    vals2.push_back(0.1);
    vals2.push_back(0.9);
    vals2.push_back(0.3);
    vals2.push_back(0.7);

    vector<double> vals3;
    vals3.push_back(0.2);
    vals3.push_back(0.8);
    vals3.push_back(0.4);
    vals3.push_back(0.6);

    NodeManager::GetNodeManager()->UTGarbageCollect();
    printmemory

    AOMDDFunction **functions = new AOMDDFunction*[20000];
    for (int i = 0; i < 20000; ++i) functions[i] = NULL;
//    MetaNode **functions = new MetaNode*[20000];

    cout << "Memory usage: " << NodeManager::GetNodeManager()->MemUsage() * (1024.0*1024) << endl;


    int cummulative = 0;

    getrusage(who, &ru);
    printmemory
    cout << endl << endl;
    int start1 = ru.ru_maxrss;
    functions[0] = new AOMDDFunction(s,vals2);
    delete functions[0];
    /*
    functions[0] = new AOMDDFunction(s,vals3);
    delete functions[0];
    */

    for(int i = 0; i < 0; ++i) {
	    cout << "Creating " << i << endl;
//	    functions[i] = AOMDDFunction(s, vals);
	    functions[i] = new AOMDDFunction(s, vals);
		    delete functions[i];
//	    NodeManager::GetNodeManager()->CreateMetaNode(s, vals);
//	    cummulative += functions[i].SelfMemUsage();
//	    cout << functions[i] << endl;
	    cout << "Memory usage: " << NodeManager::GetNodeManager()->MemUsage() * (1024.0*1024) + cummulative<< endl;
	    printmemory
	    cout << endl << endl;
    }

    cout << "Mem of array: " << sizeof(AOMDDFunction**) << endl;
    cout << "Mem of array: " << 20000*sizeof(AOMDDFunction*) << endl;
    getrusage(who, &ru);
    int start2 = ru.ru_maxrss;

    cout << "Increase = " << start2-start1 << endl;
    for(int i = 0; i < 20000; ++i) {
	    cout << "Creating " << i << endl;
	    vector<double> newVec = CreateBernoulli();
	    functions[i] = new AOMDDFunction(s2, newVec);
	    delete functions[i];
//	    functions[i] = NodeManager::GetNodeManager()->CreateMetaNode(s, newVec)->GetChildren()[0].get();
	    cout << "Memory usage: " << NodeManager::GetNodeManager()->MemUsage() * (1024.0*1024) << endl;
	    printmemory
	    cout << endl;
    }

    getrusage(who, &ru);
    int start3 = ru.ru_maxrss;
    cout << "Increase = " << start3-start2 << endl;

    unsigned long totalMem = 0;

//    for (int i = 0; i < 20000; ++i) totalMem += functions[i]->SelfMemUsage();
    cout << "Mem of functions: " << totalMem << endl;

    /*
    vals.clear();
    vals.push_back(0.24);
    vals.push_back(0.75);
    */

    NodeManager::GetNodeManager()->UTGarbageCollect();
    printmemory



    cin.get();
//    delete[] functions;
    return 0;
}
