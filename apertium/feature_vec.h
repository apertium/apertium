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
typedef std::vector<FeatureKey> UnaryFeatureVec;

class FeatureVec
{
  friend class FeatureVecAverager;
  friend class PerceptronTagger;
public:
  typedef std::map<FeatureKey, double, CompareFeatureKey> Map;
  typedef std::pair<FeatureKey, double> Pair;
  FeatureVec();
  template <typename Container> FeatureVec(Container &container);
  template <typename Iter> FeatureVec(Iter first, Iter last);
  FeatureVec& operator+=(const UnaryFeatureVec &other);
  FeatureVec& operator+=(const FeatureVec &other);
  FeatureVec& operator-=(const UnaryFeatureVec &other);
  FeatureVec& operator-=(const FeatureVec &other);
  // dot/inner product
  double operator*(const UnaryFeatureVec &other) const;
  double operator*(const FeatureVec &other) const;
  size_t size() const;
  template <typename OStream> friend OStream& operator<<(OStream & out, FeatureVec const &fv);
private:
  static Pair initPair(const Pair &pair);
  static Pair initPair(const FeatureKey &key);
  Map data;
  template <typename Iter> void init(Iter first, Iter last);
  struct FeatOp {
    FeatOp(Map &data);
    Map &data;
  };
  struct AddFeat : FeatOp {
    AddFeat(Map &data);
    void operator() (const FeatureKey &feat);
    void operator() (const Pair &feat_val);
  };
  struct SubFeat : FeatOp {
    SubFeat(Map &data);
    void operator() (const FeatureKey &feat);
    void operator() (const Pair &feat_val);
  };
  template <typename Iter> FeatureVec& inPlaceAdd(Iter first, Iter last);
  template <typename Iter> FeatureVec& inPlaceSubtract(Iter first, Iter last);
public:
  void serialise(std::ostream &serialised) const;
  void deserialise(std::istream &serialised);
};
}

#endif
