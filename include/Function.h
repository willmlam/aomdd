#ifndef FUNCTION_H_
#define FUNCTION_H_

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

    const Scope &GetScope() const {
        return domain;
    }

    virtual double GetVal(const Assignment &a, bool logOut = false) const = 0;

    virtual bool SetVal(const Assignment &a, double val) = 0;

    virtual void Project(const Scope &s) = 0;

};

} // end of aomdd namespace

#endif
