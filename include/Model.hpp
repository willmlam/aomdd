/*
 *  Model.hpp
 *  aomdd
 *
 *  Created by William Lam on Mar 31, 2011
 *  Copyright 2011 UC Irvine. All rights reserved.
 *
 */

// Represents graphical models using table functions

#ifndef MODEL_HPP_
#define MODEL_HPP_

#include "base.h"
#include "TableFunction.hpp"

namespace aomdd {

class Model {
protected:
    std::vector<int> domains;
    std::vector<TableFunction> functions;
public:
    Model() {
    }

    Model(const std::vector<int> &domainsIn) :
        domains(domainsIn) {
    }

    Model(const std::vector<int> &domainsIn,
            const std::vector<TableFunction> &funcsIn) :
        domains(domainsIn), functions(funcsIn) {
    }

    virtual ~Model() {
    }

    // parsers
    void parseUAI(std::string filename);

};

} // end of aomdd namespace


#endif /* MODEL_HPP_ */
