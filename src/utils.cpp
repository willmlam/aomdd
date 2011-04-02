#include "utils.h"

#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_real.hpp>
#include <boost/random/variate_generator.hpp>

namespace aomdd {

void PrintVector(const std::vector<int> &vec, std::ostream &out) {
    out << vec.size();
    for (unsigned int i = 0; i < vec.size(); i++)
        out << " " << vec[i];
}

void PrintList(const std::list<int> &l, std::ostream &out) {
    out << l.size();
    std::list<int>::const_iterator it = l.begin();
    for (; it != l.end(); ++it)
        out << " " << *it;
}

boost::mt19937 gen(time(0));

double UniformSample(double max) {
    boost::uniform_real<> dist(0, max);
    boost::variate_generator<boost::mt19937&, boost::uniform_real<> > sample(
            gen, dist);
    return sample();
}

} // end of aomdd namespace

