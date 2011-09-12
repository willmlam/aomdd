#ifndef FUNCTION_H_
#define FUNCTION_H_

#include "Scope.h"
#include "GenericException.hpp"
#include "base.h"

namespace aomdd {

class Function {
    friend class Bucket;
protected:
    Scope domain;
public:
    Function() {
    }

    Function(const Scope &domainIn) :
        domain(domainIn) {
    }

    Function(const Function &f) :
        domain(f.domain) {

    }

    virtual ~Function() {
    }

    const Scope &GetScope() const {
        return domain;
    }

    virtual double GetVal(const Assignment &a, bool logOut = false) const = 0;

    virtual bool SetVal(const Assignment &a, double val) = 0;

};

} // end of aomdd namespace

#endif
