#ifndef _FUNCTION_HPP_
#define _FUNCTION_HPP_

#include "Scope.h"
#include "GenericException.hpp"
#include "base.h"

namespace aomdd {

class Function {
protected:
    Scope domain;
public:
    Function() {
    }

    Function(const Scope &domainIn) :
        domain(domainIn) {
    }

    virtual ~Function() {
    }

    const Scope &GetScope() {
        return domain;
    }

    virtual double GetVal(const Assignment &a) throw (GenericException) = 0;

    virtual bool SetVal(const Assignment &a, double val) = 0;
};

} // end of aomdd namespace

#endif
