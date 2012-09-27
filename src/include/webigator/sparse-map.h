#ifndef SPARSE_MAP_H__
#define SPARSE_MAP_H__

#include <webigator/util.h>

namespace webigator {

typedef std::pair<int, double> SparsePair;
typedef std::tr1::unordered_map<int, double> SparseMap;

bool operator==(const SparseMap & lhs, const SparseMap & rhs);
bool operator!=(const SparseMap & lhs, const SparseMap & rhs);
std::ostream & operator<<(std::ostream & out, const SparseMap & rhs);
SparseMap & operator+=(SparseMap & lhs, const SparseMap & rhs);
SparseMap operator+(const SparseMap & lhs, const SparseMap & rhs);
double operator*(const SparseMap & lhs, const SparseMap & rhs);
SparseMap operator*(const SparseMap & lhs, double rhs);

}

#endif
