/*
 *  Model.cpp
 *  aomdd
 *
 *  Created by William Lam on Mar 31, 2011
 *  Copyright 2011 UC Irvine. All rights reserved.
 *
 */

// Represents graphical models using table functions

#include "Model.h"

namespace aomdd {

Model::Model() {
}

Model::Model(const std::vector<int> &domainsIn) :
domains(domainsIn) {
}

Model::Model(const std::vector<int> &domainsIn,
        const std::vector<TableFunction> &funcsIn) :
domains(domainsIn), functions(funcsIn) {
}

Model::~Model() {
}

} // end of aomdd namespace
