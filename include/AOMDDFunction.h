/*
 *  AOMDDFunction.h
 *  aomdd
 *
 *  Created by William Lam on Jun 23, 2011
 *  Copyright 2011 UC Irvine. All rights reserved.
 *
 */

#ifndef AOMDDFUNCTION_H_
#define AOMDDFUNCTION_H_

#include "./Function.h"
#include "MetaNode.h"
#include "graphbase.h"
#include "NodeManager.h"
#include "./PseudoTree.h"

namespace aomdd {

class AOMDDFunction: public Function {
private:
    static NodeManager *mgr;

//    WeightedMetaNodeList root;
    ANDNodePtr root;
    const PseudoTree *pt;

public:
    AOMDDFunction();
    AOMDDFunction(const Scope &domainIn);

    AOMDDFunction(const Scope &domainIn, const std::vector<double> &valsIn);
    // The pseudo tree is preferably one for the entire problem instance
    AOMDDFunction(const Scope &domainIn, const PseudoTree *pseudoTree,
            const std::vector<double> &valsIn);

    AOMDDFunction(const AOMDDFunction &f);

    virtual double GetVal(const Assignment &a, bool logOut = false) const;

    // To do later? Difficult if weights are not pushed to bottom.
    virtual bool SetVal(const Assignment &a, double val);

    void Multiply(const AOMDDFunction &rhs);
    void Marginalize(const Scope &elimVars, bool mutableIDs = true);
    void MarginalizeFast(const Scope &elimVars, bool mutableIDs = true);
    void Maximize(const Scope &elimVars, bool mutableIDs = true);
    void MaximizeFast(const Scope &elimVars, bool mutableIDs = true);
    void Minimize(const Scope &elimVars, bool mutableIDs = true);
    void Condition(const Assignment &cond);

    double Maximum(const Assignment &cond);
    double Sum(const Assignment &cond);

    void Normalize();

    inline void ReweighRoot(double w) {
        root->SetWeight(root->GetWeight()*w);
//        root.second *= w;
    }

    inline double GetRootWeight() const {
        return root->GetWeight();
    }

    inline bool IsConstantValue() const {
        assert(root->GetChildren().size() != 0);
        return root->GetChildren().size() == 1 && root->GetChildren()[0]->IsTerminal();
    }

    inline std::pair<unsigned int, unsigned int> Size() const {
        unsigned int numMeta = 0;
        unsigned int numAND = 0;
        unsigned int numMetaTemp = 0;
        unsigned int numANDTemp = 0;
        BOOST_FOREACH(MetaNodePtr m, root->GetChildren()) {
            tie(numMetaTemp, numANDTemp) = m->NumOfNodes();
            numMeta += numMetaTemp;
            numAND += numANDTemp;
        }
        return std::pair<unsigned int, unsigned int>(numMeta, numAND);
    }

    inline std::pair<std::vector<unsigned int>, std::vector<unsigned int> > GetCounts(unsigned int n) const {
        std::vector<unsigned int> numMeta(n, 0);
        BOOST_FOREACH(MetaNodePtr m, root->GetChildren()) {
            m->GetNumNodesPerVar(numMeta);
        }
        std::vector<unsigned int> numAND(n, 0);
        for (unsigned int i = 0; i < numMeta.size(); ++i) {
            numAND[i] += numMeta[i] * domain.GetVarCard(i);
        }
        return std::pair<std::vector<unsigned int>, std::vector<unsigned int> >(numMeta, numAND);
    }

    inline void SetScopeOrdering(const std::list<int> &ordering) { domain.SetOrdering(ordering); }

    virtual ~AOMDDFunction();

    virtual void Save(std::ostream &out) const;
    void PrintAsTable(std::ostream &out) const;

    inline double MemUsage() const {
        double memUsage = 0;
        BOOST_FOREACH(MetaNodePtr m, root->GetChildren()) {
            memUsage += m->ComputeTotalMemory();
        }
        return memUsage + sizeof(AOMDDFunction) + (root->GetChildren().size() * sizeof(MetaNodePtr)) + sizeof(double);
    }

    inline double SelfMemUsage() const {
        return sizeof(AOMDDFunction) + (root->GetChildren().size() * sizeof(MetaNodePtr)) + sizeof(double);
    }
};

} // end of aomdd namespace

#endif /* AOMDDFUNCTION_H_ */
