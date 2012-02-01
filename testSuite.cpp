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

#define printmemory {getrusage(who, &ru); cout << "rusage: " << ru.ru_maxrss << endl;}
typedef boost::shared_ptr<AOMDDFunction> AOMDDFunctionPtr;

int main2() {
    struct rusage ru;
    int who = RUSAGE_SELF;
    cout << "Before scope" << endl;
    printmemory

    Scope s;
    s.AddVar(0, 2);
    cout << "After scope" << endl;
    printmemory

    vector<double> vals;
    vals.push_back(0.25);
    vals.push_back(0.75);
    cout << "After vector pb" << endl;
    printmemory

    NodeManager::GetNodeManager()->UTGarbageCollect();
    printmemory

    cout << "Memory usage: " << NodeManager::GetNodeManager()->MemUsage() * (1024.0*1024) << endl;
    AOMDDFunctionPtr f = make_shared<AOMDDFunction>(s, vals);
    cout << "Memory usage: " << NodeManager::GetNodeManager()->MemUsage() * (1024.0*1024) << endl;
    cout << "After f create" << endl;
    printmemory
    /*
    vals.clear();
    vals.push_back(0.24);
    vals.push_back(0.75);
    */
    AOMDDFunctionPtr g = make_shared<AOMDDFunction>(s, vals);
    cout << "Memory usage: " << NodeManager::GetNodeManager()->MemUsage() * (1024.0*1024) << endl;
    cout << "After g create" << endl;
    printmemory
    AOMDDFunctionPtr h = make_shared<AOMDDFunction>(s, vals);
    cout << "Memory usage: " << NodeManager::GetNodeManager()->MemUsage() * (1024.0*1024) << endl;
    cout << "After h create" << endl;
    printmemory

    AOMDDFunctionPtr i = make_shared<AOMDDFunction>(s, vals);
    cout << "Memory usage: " << NodeManager::GetNodeManager()->MemUsage() * (1024.0*1024) << endl;
    cout << "After i create" << endl;
    printmemory

    AOMDDFunctionPtr j = make_shared<AOMDDFunction>(s, vals);
    cout << "Memory usage: " << NodeManager::GetNodeManager()->MemUsage() * (1024.0*1024) << endl;
    cout << "After j create" << endl;
    printmemory

    AOMDDFunctionPtr k = make_shared<AOMDDFunction>(s, vals);
    cout << "Memory usage: " << NodeManager::GetNodeManager()->MemUsage() * (1024.0*1024) << endl;
    cout << "After k create" << endl;
    printmemory

    AOMDDFunctionPtr l = make_shared<AOMDDFunction>(s, vals);
    cout << "Memory usage: " << NodeManager::GetNodeManager()->MemUsage() * (1024.0*1024) << endl;
    cout << "After l create" << endl;
    printmemory

    /*
    vector<AOMDDFunctionPtr> af;
    af.resize(1000);
    for (int i = 0; i < 1000; ++i) {
        af.push_back(make_shared<AOMDDFunction>(s, vals));
	    cout << "Memory usage: " << NodeManager::GetNodeManager()->MemUsage() * (1024.0*1024) << endl;
	    cout << "After create " << i << endl;
	    printmemory
    }
    */


    NodeManager::GetNodeManager()->UTGarbageCollect();
    printmemory

    cout << sizeof(AOMDDFunctionPtr) << endl;


//    cin.get();
    return 0;
}
