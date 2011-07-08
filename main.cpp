#include "Scope.h"
#include "TableFunction.h"
#include "BucketTree.h"
#include "utils.h"
#include "base.h"
#include "Model.h"
#include "Graph.h"
#include "PseudoTree.h"
#include "MetaNode.h"
#include "NodeManager.h"
#include <iostream>
#include <fstream>
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
        Print(g.GetGraph(), cout);
        cout << endl;

        cout << "InducedGraph" << endl;
        g.InduceEdges(ordering);
        Print(g.GetGraph(), cout);
        cout << endl;
        cout << "w=" << g.GetInducedWidth() << endl;
        cout << endl;

        cout << "PseudoTree" << endl;
        PseudoTree pt(g);
        Print(pt.GetTree(), cout);
        cout << endl;
        cout << "w=" << pt.GetInducedWidth() << endl;
        cout << "h=" << pt.GetHeight() << endl;
        cout << endl;
        string fname = "276hw5p4.dot";
        WriteDot(pt.GetTree(), fname);

        cout << "Number of Nodes: " << pt.GetNumberOfNodes() << endl;

        NodeManager *mgr = NodeManager::GetNodeManager();
        MetaNodePtr f3 = mgr->CreateMetaNode(m.GetFunctions()[3].GetScope(),
                m.GetFunctions()[3].GetValues());

        cout << "Building f6 DD" << endl;
        cout << "==============" << endl;
        MetaNodePtr f6 = mgr->CreateMetaNode(m.GetFunctions()[6].GetScope(),
                m.GetFunctions()[6].GetValues());

        DirectedGraph embedpt;
        int embedptroot;
        Scope s = m.GetScopes()[3] + m.GetScopes()[6];
        tie(embedpt, embedptroot) = pt.GenerateEmbeddable(s);

        cout << "Before normalize" << endl;
        cout << "================" << endl << endl;
        f6->RecursivePrint(cout);
        f6 = mgr->Normalize(f6);
        cout << "After normalize" << endl;
        cout << "================" << endl << endl;
        f6->RecursivePrint(cout);
        vector<MetaNodePtr> rhs(1, f6);
        double w = 1.0;
//        MetaNodePtr h = mgr->FullReduce(mgr->Apply(f6, rhs, SUM, embedpt), w)[0];
//        h = mgr->FullReduce(mgr->Apply(h, rhs, SUM, embedpt), w)[0];
        /*
        cout << "This is f3" << endl;
        cout << "==========" << endl;
        f3->RecursivePrint(cout); cout << endl;
        */
//        f6->Normalize();
        /*
        cout << "This is f6" << endl;
        cout << "==========" << endl;
        f6->RecursivePrint(cout); cout << endl;
        */
        /*
        cout << "This is h" << endl;
        cout << "=========" << endl;
        h->RecursivePrint(cout); cout << endl;
        */
        cout << "Number of nodes: " << mgr->GetNumberOfNodes() << endl;
        Assignment a(m.GetScopes()[6]);
        a.SetAllVal(0);
        do {
            a.Save(cout);
            cout << "  value = "<< f6->Evaluate(a) << endl;
        } while (a.Iterate());

        cout << "Result from table representation" << endl;
        TableFunction ht = m.GetFunctions()[3];
        TableFunction f6t = m.GetFunctions()[6];
        ht.Multiply(f6t);
        do {
            a.Save(cout);
            cout << "  value = "<< f6t.GetVal(a) << endl;
        } while (a.Iterate());

        Scope v3;
        v3.AddVar(2,2);
        MetaNodePtr f6m3 = mgr->Marginalize(f6, v3, embedpt);
        f6m3 = mgr->FullReduce(f6m3, w)[0];
        f6m3->Normalize();
        cout << "Before marginalizing the diagram" << endl;
        do {
            a.Save(cout);
            cout << "  value = "<< f6->Evaluate(a) << endl;
        } while (a.Iterate());
        a.RemoveVar(2);
        a.SetAllVal(0);
        cout << "Marginalize diagram result" << endl;
        do {
            a.Save(cout);
            cout << "  value = "<< f6m3->Evaluate(a) << endl;
        } while (a.Iterate());
        cout << endl;
        f6m3->RecursivePrint(cout);
        cout << endl;

        a.RemoveVar(2);
        a.SetAllVal(0);
        f6t.Marginalize(v3);
        cout << "Result from table representation" << endl;
        do {
            a.Save(cout);
            cout << "  value = "<< f6t.GetVal(a) << endl;
        } while (a.Iterate());
        cout << "Number of Nodes: " << mgr->GetNumberOfNodes() << endl;


                /*
        BucketTree bt(m, ordering, evidence);
        bt.Save(cout);
        cout << endl;
        cout << "Pr= " << bt.Prob();
        */

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
