#include <apertium/feature_vec_averager.h>

namespace Apertium {

FeatureVecAverager::FeatureVecAverager(FeatureVec &weights)
  : weights(weights), iterations(0) {}

void
FeatureVecAverager::incIteration() {
  iterations++;
}

inline void FeatureVecAverager::updateTotal(const FeatureKey &fk) {
  totals[fk] += (iterations - tstamps[fk]) * weights.data[fk];
}

inline void FeatureVecAverager::updateTotalsTimestamps(const FeatureVec &other) {
  FeatureVec::Map::const_iterator other_it;
  for (other_it = other.data.begin(); other_it != other.data.end(); other_it++) {
    updateTotal(other_it->first);
    tstamps[other_it->first] = iterations;
  }
}

FeatureVecAverager&
FeatureVecAverager::operator+=(const FeatureVec &other) {
  updateTotalsTimestamps(other);
  weights += other;
  return *this;
}

FeatureVecAverager&
FeatureVecAverager::operator-=(const FeatureVec &other) {
  updateTotalsTimestamps(other);
  weights -= other;
  return *this;
}

void
FeatureVecAverager::average() {
  FeatureVec::Map::iterator weights_it;
  for (weights_it = weights.data.begin();
       weights_it != weights.data.end(); weights_it++) {
    const FeatureKey &fk = weights_it->first;
    updateTotal(fk);
    if (totals[fk] == 0) {
      weights.data.erase(weights_it);
    } else {
      weights_it->second = totals[fk] / iterations;
    }
  }
}
}
