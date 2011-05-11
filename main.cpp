#include "Scope.h"
#include "TableFunction.h"
#include "BucketTree.h"
#include "utils.h"
#include "base.h"
#include "Model.h"
#include "MetaNode.h"
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
    Scope s;
    s.AddVar(0, 2);

    vector<MetaNode*> ANDchildren;
    ANDchildren.push_back(MetaNode::GetOne());

    vector<MetaNode::ANDNode*> children;
    children.resize(s.GetCard());
    children[0] = new MetaNode::ANDNode(0.436, ANDchildren);
    children[1] = new MetaNode::ANDNode(0.564, ANDchildren);

    MetaNode* x = new MetaNode(s, children);

    Assignment a(s);
    a.SetVal(0, 0);

    cout << x->Evaluate(a) << endl;

    Scope s2;
    s2.AddVar(1, 2);

    children.clear();
    children.resize(s2.GetCard());
    children[0] = new MetaNode::ANDNode(0.128, ANDchildren);
    children[1] = new MetaNode::ANDNode(0.872, ANDchildren);
    MetaNode* x0_y = new MetaNode(s2, children);

    children.clear();
    children.resize(s2.GetCard());
    children[0] = new MetaNode::ANDNode(0.920, ANDchildren);
    children[1] = new MetaNode::ANDNode(0.080, ANDchildren);
    MetaNode* x1_y = new MetaNode(s2, children);

    children.clear();
    children.resize(s.GetCard());
    children[0] = new MetaNode::ANDNode(1, vector<MetaNode*>(1, x0_y));
    children[1] = new MetaNode::ANDNode(1, vector<MetaNode*>(1, x1_y));
    MetaNode* x_y = new MetaNode(s, children);

    Assignment a2(s+s2);
    a2.SetAllVal(0);

    do {
        a2.Save(cout);
        cout << " value=" << x_y->Evaluate(a2) << endl;
    } while(a2.Iterate());

    boost::unordered_set<MetaNode*> uniqueTable;

    cout << "Pointer values:" << endl;
    cout << "Zero=" << MetaNode::GetZero() << endl;
    cout << "One=" << MetaNode::GetOne() << endl;
    cout << "x_y=" << x_y << endl;
    cout << "x0_y=" << x0_y << endl;
    cout << "x1_y=" << x1_y << endl;
    uniqueTable.insert(MetaNode::GetZero());
    uniqueTable.insert(MetaNode::GetOne());
    uniqueTable.insert(x_y);
    uniqueTable.insert(x0_y);
    uniqueTable.insert(x1_y);

    int count = 1;
    BOOST_FOREACH(MetaNode *i, uniqueTable) {
        cout << count++ << endl;
        i->Save(cout); cout << endl;
    }







    return 0;

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
        BucketTree bt(m, ordering, evidence);
        bt.Save(cout);
        cout << endl;
        cout << "Pr= " << bt.Prob();

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
