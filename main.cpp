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
#include <fstream>
#include <sstream>
using namespace aomdd;
using namespace std;

list<int> parseOrder(string filename) {
    ifstream infile(filename.c_str());

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

void IterateTester(Assignment & a) {
    a.SetAllVal(0);

    cout << "Total card: " << a.GetCard() << endl;

    cout << "Testing iteration" << endl;
    do {
        cout << "Index: " << a.GetIndex() << "  ";
        a.Save(cout);
        cout << endl;
    } while (a.Iterate());

    cout << endl;
}

typedef boost::unordered_map<Assignment, double> FTable;

int main(int argc, char **argv) {


    /*
    Scope s;
    s.AddVar(0, 2);
    s.AddVar(1, 2);
    vector<double> vals;
    vals.push_back(0.6);
    vals.push_back(0.6);
    vals.push_back(0.3);
    vals.push_back(0.3);

    NodeManager *mgr = NodeManager::GetNodeManager();
    MetaNodePtr f = mgr->CreateMetaNode(s, vals);

    Assignment a(s);
    a.SetAllVal(0);
    do {
        a.Save(cout);
        cout << "  value = "<< f->Evaluate(a) << endl;
    } while (a.Iterate());

    f->RecursivePrint(cout);

    cout << "Number of nodes: " << mgr->GetNumberOfNodes() << endl;
    double w = 1;
    f = mgr->FullReduce(f, w)[0];
    do {
        a.Save(cout);
        cout << "  value = "<< f->Evaluate(a) << endl;
    } while (a.Iterate());
    f->RecursivePrint(cout);
    cout << "Number of nodes: " << mgr->GetNumberOfNodes() << endl;

    mgr->PrintUniqueTable(cout); cout << endl;

    return 0;
    */

    /*
     Scope s;
     s.AddVar(0, 2);
     s.AddVar(1, 2);

     TableFunction f(s);

     Assignment a(s);
     a.SetAllVal(0);
     double count = 1;
     do {
     f.SetVal(a, count++ / 10);

     } while(a.Iterate());

     f.Save(cout); cout << endl;

     Scope s2;
     s2.AddVar(1, 2);
     s2.AddVar(2, 2);

     TableFunction f2(s2);

     Assignment a2(s2);
     a2.SetAllVal(0);
     double count2 = 4;
     do {
     f2.SetVal(a2, count2-- / 10);

     } while(a2.Iterate());

     f2.Save(cout); cout << endl;

     f.Multiply(f2);

     f.Save(cout); cout << endl;



     Scope marg;
     marg.AddVar(0, 2);
     f.Marginalize(marg);
     f.Save(cout);
     return 0;
     */

    string inputFile(argv[1]);
    string orderFile(argv[2]);
    string evidFile(argv[3]);

    Model m;

    try {
        m.parseUAI(inputFile);
        //m.Save(cout);
        cout << endl;
        list<int> ordering = parseOrder(orderFile);
        map<int, int> evidence = parseEvidence(evidFile);
        m.SetOrdering(ordering);

        Graph g(m.GetScopes());
        g.InduceEdges(ordering);
        PseudoTree pt(g);
        string dotfilename = "276pt.dot";
        WriteDot(pt.GetTree(), dotfilename);

        BucketTree bt(m, ordering, evidence);
        cout << "P(e) = "<< bt.Prob() << endl;

        AOMDDFunction f0(m.GetScopes()[0], &pt, m.GetFunctions()[0].GetValues());
        AOMDDFunction f4(m.GetScopes()[4], &pt, m.GetFunctions()[4].GetValues());

        f0.PrintAsTable(cout); cout << endl;
        f4.PrintAsTable(cout); cout << endl;

        f0.Multiply(f4);
        f0.PrintAsTable(cout); cout << endl;
        f0.Save(cout); cout << endl;
        return 0;

        CompileBucketTree cbt(m, &pt, ordering, false);
        cbt.PrintBucketFunctionScopes(cout);
        AOMDDFunction combined = cbt.Compile();

        int nvars = ordering.size();

        TableFunction combinedt(m.GetFunctions()[nvars-1]);
        for (int i = nvars - 2; i >= 0; --i) {
            combinedt.Multiply(m.GetFunctions()[i]);
        }

        Assignment a(combined.GetScope());
        a.SetAllVal(0);
        double total = 0;
        double ttotal = 0;
        do {
            double dv = combined.GetVal(a);
            double tv = combinedt.GetVal(a);
            a.Save(cout); cout << " dv = " << dv
            << ", tv = " << tv << endl;
            assert(fabs(dv - tv) < 1e-10);
            total += dv;
            ttotal += tv;
        } while (a.Iterate());
        cout << endl;
        cout << "Total(d): " << total << endl;
        cout << "Total(t): " << ttotal << endl;

        cout << "Number of nodes in manager: "
                << NodeManager::GetNodeManager()->GetNumberOfNodes() << endl;
//        cout << "Reference counts: " << endl;
//        NodeManager::GetNodeManager()->PrintReferenceCount(cout);
//        cout << endl;

        combined.Save(cout);
        cout << "Combined AOMDD size: " << combined.Size() << endl;




    } catch (GenericException & e) {
        cout << e.what();
    }

    //Old, basic testing of function table and scope
    /*
     Scope test1;
     test1.AddVar(0, 3);
     test1.AddVar(1, 2);
     test1.Save(cout); cout << endl;

     Scope test2;
     test2.AddVar(1, 2);
     test2.AddVar(2, 2);
     test2.Save(cout); cout << endl;

     Scope test3 = test1 + test2;
     test3.Save(cout); cout << endl;

     Scope test4 = test1 * test2;
     test4.Save(cout); cout << endl;
     Scope test5 = test1 - test2;
     test5.Save(cout); cout << endl;

     Assignment a1(test3);

     IterateTester(a1);

     a1.AddVar(3, 4);

     IterateTester(a1);

     a1.RemoveVar(3);

     TableFunction<double> testf(test3);
     do {
     testf.SetVal(a1, UniformSample());
     } while (a1.Iterate());


     do {
     a1.Save(cout);
     cout << " value=" << testf.GetVal(a1) << endl;
     } while (a1.Iterate());


     cout << endl;

     TableFunction<string> testf2(test3);
     do {
     testf2.SetVal(a1, "pickle");
     } while (a1.Iterate());

     do {
     a1.Save(cout);
     cout << " value=" << testf2.GetVal(a1) << endl;
     } while (a1.Iterate());
     cout << endl;

     a1.UnsetVal(0);

     //Test for errors
     try {
     cout << testf2.GetVal(a1) << endl;
     }
     catch (GenericException &e) {
     cout << e.what() << endl;
     }

     cout << endl;
     */
    return 0;
}
