#include <stdint.h>
#include <apertium/feature_vec.h>
#include <apertium/deserialiser.h>
#include <apertium/serialiser.h>
#include <algorithm>
#include <iterator>
#include <vector>

namespace Apertium {

bool 
CompareFeatureKey::operator() (FeatureKey const& lhs, FeatureKey const& rhs) const 
{
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

template
FeatureVec::FeatureVec(UnaryFeatureVec &ufv);

template <typename Iter>
FeatureVec::FeatureVec(Iter first, Iter last) : data()
{
  init(first, last);
}

FeatureVec&
FeatureVec::operator+=(const UnaryFeatureVec &other)
{
  return inPlaceAdd(other.begin(), other.end());
}

FeatureVec&
FeatureVec::operator+=(const FeatureVec &other)
{
  return inPlaceAdd(other.data.begin(), other.data.end());
}

FeatureVec&
FeatureVec::operator-=(const UnaryFeatureVec &other)
{
  return inPlaceSubtract(other.begin(), other.end());
}

template <typename OStream>
OStream&
operator<<(OStream & out, FeatureVec const &fv) 
{
  FeatureVec::Map::const_iterator feat_it = fv.data.begin();
  for (; feat_it != fv.data.end(); feat_it++) 
  {
    FeatureKey::const_iterator bc_it = feat_it->first.begin();
    out << std::dec << (int)(*(bc_it++))[0] << "; ";
    for (;bc_it != feat_it->first.end(); bc_it++) 
    {
      out << bc_it->c_str();
      if (bc_it + 1 != feat_it->first.end()) 
      {
        out << ", ";
      }
    }
    out << ": " << feat_it->second << "\n";
  }
  return out;
}

template std::wostream&
operator<<(std::wostream& out, FeatureVec const &fv);

template std::ostream&
operator<<(std::ostream& out, FeatureVec const &fv);

FeatureVec&
FeatureVec::operator-=(const FeatureVec &other)
{
  return inPlaceSubtract(other.data.begin(), other.data.end());
}

double 
FeatureVec::operator*(const UnaryFeatureVec &other) const
{
  double result = 0.0L;
  UnaryFeatureVec::const_iterator other_it;
  for (other_it = other.begin(); other_it != other.end(); other_it++) {
    Map::const_iterator fv_it = data.find(*other_it);
    if (fv_it != data.end()) {
      result += fv_it->second;
    }
  }
  return result;
}

double 
FeatureVec::operator*(const FeatureVec &other) const
{
  // This is O(N) in the size of the largest vector.
  // With a hash table we would have O(n) in the size of the smaller vector.
  double result = 0.0L;
  FeatureVec::Map::const_iterator il = data.begin();
  FeatureVec::Map::const_iterator ir = other.data.begin();
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

size_t
FeatureVec::size() const
{
  return data.size();
};

template <typename Iter>
void
FeatureVec::init(Iter first, Iter last)
{
  for (;first!=last;first++) {
    data.insert(initPair(*first));
  }
}

FeatureVec::Pair
FeatureVec::initPair(const FeatureVec::Pair &pair)
{
  return pair;
}

FeatureVec::Pair
FeatureVec::initPair(const FeatureKey &key)
{
  return make_pair(key, 1.0L);
}

FeatureVec::FeatOp::FeatOp(FeatureVec::Map &data)
  : data(data) {}

FeatureVec::AddFeat::AddFeat(FeatureVec::Map &data)
  : FeatOp(data) {}

FeatureVec::SubFeat::SubFeat(FeatureVec::Map &data)
  : FeatOp(data) {}

void
FeatureVec::AddFeat::operator()(const FeatureKey &feat)
{
  data[feat] += 1.0L;
}

void
FeatureVec::AddFeat::operator()(const FeatureVec::Pair &feat_val)
{
  data[feat_val.first] += feat_val.second;
}

void
FeatureVec::SubFeat::operator()(const FeatureKey &feat)
{
  data[feat] -= 1.0L;
}

void
FeatureVec::SubFeat::operator()(const FeatureVec::Pair &feat_val)
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

void 
FeatureVec::serialise(std::ostream &serialised) const 
{
  Serialiser<FeatureVec::Map>::serialise(data, serialised);
}

void 
FeatureVec::deserialise(std::istream &serialised) 
{
  data = Deserialiser<FeatureVec::Map>::deserialise(serialised);
}

}
