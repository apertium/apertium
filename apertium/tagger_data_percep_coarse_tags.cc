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
  Serialiser<vector<wstring> >::serialise(array_tags, serialised);
  Serialiser<map<wstring, TTag, Ltstr> >::serialise(tag_index, serialised);
  constants.serialise(serialised);
  output.serialise(serialised);
  plist.serialise(serialised);
}

void TaggerDataPercepCoarseTags::deserialise(std::istream &serialised)
{
  open_class = Deserialiser<set<TTag> >::deserialise(serialised);
  array_tags = Deserialiser<vector<wstring> >::deserialise(serialised);
  tag_index = Deserialiser<map<wstring, TTag, Ltstr> >::deserialise(serialised);
  constants.deserialise(serialised);
  output.deserialise(serialised);
  plist.deserialise(serialised);
}

const wstring& TaggerDataPercepCoarseTags::coarsen(const Apertium::Morpheme &wrd) const
{
  // Init fine -> coarse tags matching machinary
  MatchState ms;
  MatchExe *me = plist.newMatchExe();
  const Alphabet alphabet = plist.getAlphabet();
  int ca_any_char = alphabet(PatternList::ANY_CHAR);
  int ca_any_tag = alphabet(PatternList::ANY_TAG);
  map<wstring, int, Ltstr>::const_iterator undef_it = tag_index.find(L"TAG_kUNDEF");
  int ca_tag_kundef = undef_it->second;
  // Input lemma
  ms.init(me->getInitial());
  for (size_t i = 0; i < wrd.TheLemma.size(); i++) {
    ms.step(std::towlower(wrd.TheLemma[i]), ca_any_char);
  }
  // Input fine tags
  for (size_t i = 0; i < wrd.TheTags.size(); i++) {
    int symbol = alphabet(L"<" + wrd.TheTags[i].TheTag + L">");
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
