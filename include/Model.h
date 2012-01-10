/*
 *  Model.h
 *  aomdd
 *
 *  Created by William Lam on Mar 31, 2011
 *  Copyright 2011 UC Irvine. All rights reserved.
 *
 */

// Represents graphical models using table functions

#ifndef MODEL_H_
#define MODEL_H_

#include "base.h"
#include "TableFunction.h"

namespace aomdd {

class Model {
protected:
    int numVars;
    int maxDomain;
    Scope completeScope;
    std::vector<Scope> scopes;
    std::vector<int> domains;
    std::vector<TableFunction> functions;
    std::list<int> ordering;
public:
    Model();

    virtual ~Model();

    inline int GetNumVars() const { return numVars; }
    inline int GetMaxDomain() const { return maxDomain; }

    inline const std::vector<TableFunction> &GetFunctions() const {
        return functions;
    }

    inline const std::vector<Scope> &GetScopes() const {
        return scopes;
    }


    void SetOrdering(const std::list<int> &orderIn);

    inline const std::list<int> &GetOrdering() const {
        return ordering;
    }

    inline const Scope &GetCompleteScope() const {
        return completeScope;
    }

    // parsers
    void parseUAI(std::string filename);

    void AddFunction(const Scope &s, const std::vector<double>& values);

    inline void SetFunctionVal(int i, Assignment a, double val) {
        functions[i].SetVal(a, val);
    }

    void FreeMemory() {
        scopes.clear();
        domains.clear();
        functions.clear();
        ordering.clear();
    }

    void Save(std::ostream &out);

};

} // end of aomdd namespace


#endif /* MODEL_HPP_ */
