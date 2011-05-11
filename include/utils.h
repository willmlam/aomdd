#ifndef _UTILS_H_
#define _UTILS_H_

#include "base.h"

namespace aomdd {

// Utilities to print out STL containers
void PrintVector(const std::vector<int> &vec, std::ostream &out);

void PrintList(const std::list<int> &l, std::ostream &out);

// Returns a real number on the interval from 0 to max.
double UniformSample(double max = 1);

// Treat the input vector as a list of supports and returns a sample
int CategoricalSample(const std::vector<double> &vec);

} // end of aomdd namespace

#endif
