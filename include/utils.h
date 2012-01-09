#ifndef _UTILS_H_AOMDD
#define _UTILS_H_AOMDD

#include "base.h"
#include "graphbase.h"

namespace aomdd {

// Utilities to print out STL containers
void PrintVector(const std::vector<int> &vec, std::ostream &out);

void PrintList(const std::list<int> &l, std::ostream &out);

// Splits a vector into k equally sized vectors. Truncates trailing elements.
std::vector<std::vector<double> > SplitVector(const std::vector<double> &vec,
        int k);

// Returns the maximum element in the vector. T must be comparable
template <typename T>
std::pair<T, int> Max(const std::vector<T> &vec) {
    assert(!vec.empty());
    T ret = vec[0];
    int idx = 0;
    for (unsigned int i = 1; i < vec.size(); ++i) {
        if (vec[i] > ret) {
            ret = vec[i];
            idx = i;
        }
    }
    return std::make_pair<T, int>(ret, idx);
}

// Returns the maximum element in the vector. T must be comparable
template <typename T>
std::pair<T, int> Min(const std::vector<T> &vec) {
    assert(!vec.empty());
    T ret = vec[0];
    int idx = 0;
    for (unsigned int i = 1; i < vec.size(); ++i) {
        if (vec[i] < ret) {
            ret = vec[i];
            idx = i;
        }
    }
    return std::make_pair<T, int>(ret, idx);
}

// Returns a real number on the interval from 0 to max.
double UniformSample(double max = 1);

// Treat the input vector as a list of supports and returns a sample
int CategoricalSample(const std::vector<double> &vec);

std::vector<int> GetConnectedComponentSizes(int numComponents, const std::vector<int> &componentMap);

void Print(const DirectedGraph &dg, std::ostream &os);
void Print(const UndirectedGraph &g, std::ostream &os);

void WriteDot(const DirectedGraph &g, std::string filename);

} // end of aomdd namespace

#endif
