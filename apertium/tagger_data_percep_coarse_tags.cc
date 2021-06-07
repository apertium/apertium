#include <apertium/tagger_data_percep_coarse_tags.h>
#include <lttoolbox/serialiser.h>
#include <lttoolbox/deserialiser.h>
#include <lttoolbox/match_state.h>
#include <iostream>

TaggerDataPercepCoarseTags::TaggerDataPercepCoarseTags() {}

TaggerDataPercepCoarseTags::TaggerDataPercepCoarseTags(TaggerDataPercepCoarseTags const &o)
{
  TaggerData::copy(o);
}

TaggerDataPercepCoarseTags::TaggerDataPercepCoarseTags(TaggerData const &o)
{
  TaggerData::copy(o);
}

TaggerDataPercepCoarseTags& TaggerDataPercepCoarseTags::operator=(TaggerDataPercepCoarseTags const &o)
{
  TaggerData::copy(o);
  return *this;
}

TaggerDataPercepCoarseTags::~TaggerDataPercepCoarseTags() {}

void TaggerDataPercepCoarseTags::serialise(std::ostream &serialised) const
{
  Serialiser<set<TTag> >::serialise(open_class, serialised);
  Serialiser<vector<UString> >::serialise(array_tags, serialised);
  Serialiser<map<UString, TTag> >::serialise(tag_index, serialised);
  constants.serialise(serialised);
  output.serialise(serialised);
  plist.serialise(serialised);
}

void TaggerDataPercepCoarseTags::deserialise(std::istream &serialised)
{
  open_class = Deserialiser<set<TTag> >::deserialise(serialised);
  array_tags = Deserialiser<vector<UString> >::deserialise(serialised);
  tag_index = Deserialiser<map<UString, TTag> >::deserialise(serialised);
  constants.deserialise(serialised);
  output.deserialise(serialised);
  plist.deserialise(serialised);
}

const UString& TaggerDataPercepCoarseTags::coarsen(const Apertium::Morpheme &wrd) const
{
  // Init fine -> coarse tags matching machinary
  MatchState ms;
  MatchExe *me = plist.newMatchExe();
  const Alphabet alphabet = plist.getAlphabet();
  int ca_any_char = alphabet(PatternList::ANY_CHAR);
  int ca_any_tag = alphabet(PatternList::ANY_TAG);
  map<UString, int>::const_iterator undef_it = tag_index.find("TAG_kUNDEF"_u);
  int ca_tag_kundef = undef_it->second;
  // Input lemma
  ms.init(me->getInitial());
  for (size_t i = 0; i < wrd.TheLemma.size(); i++) {
    ms.step(std::towlower(wrd.TheLemma[i]), ca_any_char);
  }
  // Input fine tags
  for (size_t i = 0; i < wrd.TheTags.size(); i++) {
    UString tag;
    tag += '<';
    tag.append(wrd.TheTags[i].TheTag);
    tag += '>';
    int symbol = alphabet(tag);
    if (symbol) {
      ms.step(symbol, ca_any_tag);
    }
  }
  // Output result
  int val = ms.classifyFinals(me->getFinals());
  if (val == -1) {
    val = ca_tag_kundef;
  }
  delete me;
  return array_tags[val];
}
