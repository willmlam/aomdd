#ifndef _BASE_H_
#define _BASE_H_

// Data structures
#include <vector>
#include <list>
#include <map>
#include <set>
#include <stack>
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

// Exceptions
#include "GenericException.hpp"

// Special Boost tools
#include <boost/foreach.hpp>

// Constants
const int UNKNOWN_VAL = -1;
const int ERROR_VAL = -10;
const double DOUBLE_MIN = -100000000;

#endif
