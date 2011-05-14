#include "utils.h"

#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_real.hpp>
#include <boost/random/variate_generator.hpp>
#include <time.h>
#include <algorithm>
#include <numeric>

namespace aomdd {
using namespace std;

void PrintVector(const vector<int> &vec, ostream &out) {
    out << vec.size();
    BOOST_FOREACH(int i, vec) {
        out << " " << i;
    }
}

void PrintList(const list<int> &l, ostream &out) {
    out << l.size();
    BOOST_FOREACH(int i, l) {
        out << " " << i;
    }
}

vector<vector<double> > SplitVector(const vector<double> &vec, int k) {
    vector<vector<double> > ret;
    vector<double>::const_iterator it = vec.begin();
    int offset = (double)vec.size() / k;
    for (int i = 0; i < k; ++i) {
        ret.push_back(vector<double>(it, it + offset));
        it += offset;
    }
    return ret;
}

boost::mt19937 gen(time(0));

double UniformSample(double max) {
    boost::uniform_real<> dist(0, max);
    boost::variate_generator<boost::mt19937&, boost::uniform_real<> > sample(
            gen, dist);
    return sample();
}

int CategoricalSample(const vector<double> &vec) {
    vector<double> cumulative;
    partial_sum(vec.begin(), vec.end(), back_inserter(cumulative));
    boost::uniform_real<> dist(0, cumulative.back());
    boost::variate_generator<boost::mt19937&, boost::uniform_real<> > sample(
            gen, dist);
    return lower_bound(cumulative.begin(), cumulative.end(),
            sample()) - cumulative.begin();
}

} // end of aomdd namespace

