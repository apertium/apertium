#ifndef _TAGGER_DATA_PERCEP_COARSE_TAGS_H
#define _TAGGER_DATA_PERCEP_COARSE_TAGS_H

#include <apertium/tagger_data.h>
#include <apertium/morpheme.h>

class TaggerDataPercepCoarseTags : public TaggerData
{
public:
  TaggerDataPercepCoarseTags();
  TaggerDataPercepCoarseTags(TaggerDataPercepCoarseTags const &o);
  TaggerDataPercepCoarseTags(TaggerData const &o);
  TaggerDataPercepCoarseTags & operator =(TaggerDataPercepCoarseTags const &o);
  virtual ~TaggerDataPercepCoarseTags();
  void serialise(std::ostream &serialised) const;
  void deserialise(std::istream &serialised);
  const wstring& coarsen(const Apertium::Morpheme &wrd) const;
};

#endif
