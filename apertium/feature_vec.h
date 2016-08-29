#ifndef _FEATURE_VEC_H
#define _FEATURE_VEC_H

#include <map>
#include <vector>
#include <string>
#include <utility>
#include <iostream>

namespace Apertium {

typedef std::vector<std::string> FeatureKey;
struct CompareFeatureKey {
  bool operator() (FeatureKey const& lhs, FeatureKey const& rhs) const;
};
typedef std::map<FeatureKey, double, CompareFeatureKey> FeatureVecMap;
typedef std::pair<FeatureKey, double> FeatureVecPair;

class FeatureVec
{
public:
  FeatureVec();
  template <typename Container> FeatureVec(Container &container);
  template <typename Iter> FeatureVec(Iter first, Iter last);
  template <typename Container> FeatureVec& operator+=(const Container &container);
  FeatureVec& operator+=(const FeatureVec &other);
  template <typename Container> FeatureVec& operator-=(const Container &container);
  FeatureVec& operator-=(const FeatureVec &other);
  // dot/inner product
  template <typename Container> double operator*(const Container &other) const;
  double operator*(const FeatureVec &other) const;
  template <typename OStream> friend OStream& operator<<(OStream & out, FeatureVec const &fv);
private:
  static FeatureVecPair initPair(FeatureVecPair &pair);
  static FeatureVecPair initPair(FeatureKey &key);
  FeatureVecMap data;
  template <typename Iter> void init(Iter first, Iter last);
  struct FeatOp {
    FeatOp(FeatureVecMap &data);
    FeatureVecMap &data;
  };
  struct AddFeat : FeatOp {
    AddFeat(FeatureVecMap &data);
    void operator() (const FeatureKey &feat);
    void operator() (const FeatureVecPair &feat_val);
  };
  struct SubFeat : FeatOp {
    SubFeat(FeatureVecMap &data);
    void operator() (const FeatureKey &feat);
    void operator() (const FeatureVecPair &feat_val);
  };
  template <typename Iter> FeatureVec& inPlaceAdd(Iter first, Iter last);
  template <typename Iter> FeatureVec& inPlaceSubtract(Iter first, Iter last);
};
}

#endif
