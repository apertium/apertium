#include <apertium/feature_vec.h>
#include <algorithm>
#include <vector>

namespace Apertium {

bool CompareFeatureKey::operator() (FeatureKey const& lhs, FeatureKey const& rhs) const {
  size_t min_size = std::min(lhs.size(), rhs.size());
  for (size_t i = 0; i < min_size; i++) {
    if (lhs[i] < rhs[i]) {
      return true;
    } else if (lhs[i] > rhs[i]) {
      return false;
    }
  }
  return lhs.size() < rhs.size();
}

FeatureVec::FeatureVec() : data() {}

template <typename Container>
FeatureVec::FeatureVec(Container &container) : data()
{
  init(container.begin(), container.end());
}

template <typename Iter>
FeatureVec::FeatureVec(Iter first, Iter last) : data()
{
  init(first, last);
}

template <typename Container>
FeatureVec&
FeatureVec::operator+=(const Container &container)
{
  return inPlaceAdd(container.begin(), container.end());
}

template FeatureVec& FeatureVec::operator+=(const std::vector<FeatureKey> &other);

FeatureVec&
FeatureVec::operator+=(const FeatureVec &other)
{
  return inPlaceAdd(other.data.begin(), other.data.end());
}

template <typename Container> FeatureVec&
FeatureVec::operator-=(const Container &container)
{
  return inPlaceSubtract(container.begin(), container.end());
}

template FeatureVec& FeatureVec::operator-=(const std::vector<FeatureKey> &other);

template <typename OStream>
OStream&
operator<<(OStream & out, FeatureVec const &fv) {
  FeatureVecMap::const_iterator it = fv.data.begin();
  for (; it != fv.data.end(); it++) {
    out << it->first << ": " << it->second << "\n";
  }
}

FeatureVec&
FeatureVec::operator-=(const FeatureVec &other)
{
  return inPlaceSubtract(other.data.begin(), other.data.end());
}

template <typename Container>
double FeatureVec::operator*(const Container &other) const
{
  double result = 0.0L;
  typename Container::const_iterator other_it = other.begin();
  for (; other_it != other.end(); other_it++) {
    FeatureVecMap::const_iterator fv_it = data.find(*other_it);
    if (fv_it != data.end()) {
      result += fv_it->second;
    }
  }
  return result;
}

template double FeatureVec::operator*(const std::vector<FeatureKey> &other) const;

double FeatureVec::operator*(const FeatureVec &other) const
{
  // This is O(N) in the size of the largest vector.
  // With a hash table we would have O(n) in the size of the smaller vector.
  double result = 0.0L;
  FeatureVecMap::const_iterator il = data.begin();
  FeatureVecMap::const_iterator ir = other.data.begin();
  while (il != data.end() && ir != other.data.end()) {
    if (il->first < ir->first) {
      ++il;
    } else if (ir->first < il->first) {
      ++ir;
    } else {
      result += il->second * ir->second;
      ++il;
      ++ir;
    }
  }
  return result;
}

template <typename Iter>
void
FeatureVec::init(Iter first, Iter last)
{
  std::transform(first, last, data.begin(), initPair);
}

FeatureVecPair FeatureVec::initPair(FeatureVecPair &pair)
{
  return pair;
}

FeatureVecPair FeatureVec::initPair(FeatureKey &key)
{
  return make_pair(key, 1.0L);
}

FeatureVec::FeatOp::FeatOp(FeatureVecMap &data) : data(data) {}
FeatureVec::AddFeat::AddFeat(FeatureVecMap &data) : FeatOp(data) {}
FeatureVec::SubFeat::SubFeat(FeatureVecMap &data) : FeatOp(data) {}

void
FeatureVec::AddFeat::operator()(const FeatureKey &feat)
{
  data[feat] += 1.0L;
}

void
FeatureVec::AddFeat::operator()(const FeatureVecPair &feat_val)
{
  data[feat_val.first] += feat_val.second;
}

void
FeatureVec::SubFeat::operator()(const FeatureKey &feat)
{
  data[feat] -= 1.0L;
}

void
FeatureVec::SubFeat::operator()(const FeatureVecPair &feat_val)
{
  data[feat_val.first] -= feat_val.second;
}

template <typename Iter>
FeatureVec&
FeatureVec::inPlaceAdd(Iter first, Iter last)
{
  for_each(first, last, AddFeat(data));
  return *this;
}

template <typename Iter>
FeatureVec&
FeatureVec::inPlaceSubtract(Iter first, Iter last)
{
  for_each(first, last, SubFeat(data));
  return *this;
}
}
