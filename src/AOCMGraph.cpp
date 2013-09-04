#include "AOCMGraph.h"
#include <stack>

using namespace std;

namespace aomdd {

    void AOCMGraph::GenerateDot(ostream &out) const {
    }

    void AOCMGraph::GenerateGraph() {
        stack<AOCMGraphNode*> stk;       
        int root = pt->GetRoot();
        int rootDomainSize;
        if (pt->HasDummy()) {
            rootDomainSize = 1;
        }
        else {
            rootDomainSize = m->GetDomains()[root];
        }
        Assignment a;
        a.AddVar(root,1);
        a.SetVal(root,0);
        
        stk.push(new AOCMGraphNode(root,a,OR,1.0));
    }

} // end of aomdd namespace
