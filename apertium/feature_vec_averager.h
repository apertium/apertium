#ifndef _FEATURE_VEC_AVERAGER_H
#define _FEATURE_VEC_AVERAGER_H

#include <apertium/feature_vec.h>

namespace Apertium {
class FeatureVecAverager {
std::map<FeatureKey, unsigned int> tstamps;
std::map<FeatureKey, double> totals;
FeatureVec& weights;
unsigned int iterations;

inline void updateTotal(const FeatureKey &fk);
inline void updateTotalsTimestamps(const FeatureVec &other);

public:
FeatureVecAverager(FeatureVec &weights);
void incIteration();
FeatureVecAverager& operator+=(const FeatureVec &other);
FeatureVecAverager& operator-=(const FeatureVec &other);
void average();
};
}

#endif
