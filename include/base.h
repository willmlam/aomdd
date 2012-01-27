
// Data structures
#include <vector>
#include <list>
#include <map>
#include <set>
#include <stack>
#include <algorithm>
#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>
#include <boost/functional/hash.hpp>
#include <boost/shared_ptr.hpp>
#include <google/dense_hash_set>
#include <google/sparse_hash_set>
#include <google/dense_hash_map>
#include <google/sparse_hash_map>

#include <iostream>
#include <sstream>
#include <cassert>

// Special Boost tools
#include <boost/foreach.hpp>

#ifndef _BASE_H_
#define _BASE_H_

// Constants
const int UNKNOWN_VAL = -1;
const int ERROR_VAL = -10;
const int EMPTY_KEY = -20;
const int DELETED_KEY = -30;
const double DOUBLE_MIN = -100000000;
const double DOUBLE_MAX = 100000000;

const double MB_PER_BYTE = 1048576;

// Limits
const unsigned long OUTPUT_COMPLEXITY_LIMIT = 2048;
const double MB_LIMIT = 2048;

enum QueryType {PE, MPE};


#endif
