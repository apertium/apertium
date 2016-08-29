#include <vector>

template<typename A, typename B>
class SortByComparer {
private:
  std::vector<A> &sort_by;
public:
  SortByComparer(std::vector<A> &sort_by) : sort_by(sort_by) {}
  bool operator()(B a, B b) {
    return sort_by[a] < sort_by[b];
  }
};
