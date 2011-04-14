#include "Scope.h"
#include "TableFunction.h"
#include "BucketTree.h"
#include "utils.h"
#include "base.h"
#include <iostream>
#include "Model.h"
#include <fstream>
using namespace aomdd;
using namespace std;

list < int     >
parseOrder(string filename)
{
	ifstream        infile(filename.c_str());

	string          buffer;
	int             nv, intBuffer;
	getline(infile, buffer);

	infile >> nv;

	list < int     >ordering;
	for (int i = 0; i < nv; i++) {
		infile >> intBuffer;
		ordering.push_front(intBuffer);
	}
	return ordering;
}

void 
IterateTester(Assignment & a)
{
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

int 
main(int argc, char **argv)
{

	string          inputFile(argv[1]);
	string          orderFile(argv[2]);

	Model           m;
	try {
		m.parseUAI(inputFile);
		//m.Save(cout);
		cout << endl;
		list < int     >ordering = parseOrder(orderFile);
		m.SetOrdering(ordering);
		BucketTree      bt(m, ordering);
		bt.Save(cout);
		cout << endl;
		cout << "logPr= " << bt.Prob(true);

	} catch(GenericException & e) {
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
