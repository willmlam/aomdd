#ifndef _UTILS_H_
#define _UTILS_H_

#include "base.h"

namespace aomdd {

    void PrintVector(const std::vector<int> &vec, std::ostream &out);
    
    void PrintList(const std::list<int> &l, std::ostream &out);
    
    double UniformSample(double max = 1);

} // end of aomdd namespace

#endif
