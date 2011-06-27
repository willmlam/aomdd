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
    std::vector<int> domains;
    std::vector<TableFunction> functions;
    std::list<int> ordering;
public:
    Model();

    Model(const std::vector<int> &domainsIn);

    Model(const std::vector<int> &domainsIn,
            const std::vector<TableFunction> &funcsIn);

    virtual ~Model();

    inline const std::vector<TableFunction> &GetFunctions() const {
        return functions;
    }

    std::vector<Scope> GetScopes() const;


    void SetOrdering(const std::list<int> &orderIn);

    // parsers
    void parseUAI(std::string filename);

    void Save(std::ostream &out);

};

} // end of aomdd namespace


#endif /* MODEL_HPP_ */
