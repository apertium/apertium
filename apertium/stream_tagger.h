#ifndef STREAM_TAGGER_H
#define STREAM_TAGGER_H

#include "stream.h"
#include <istream>
#include <ostream>

namespace Apertium {
class StreamTagger {
protected:
  TaggerFlags TheFlags;
public:
  StreamTagger();
  StreamTagger(TaggerFlags& Flags_);
  virtual ~StreamTagger();
  virtual void serialise(std::ostream &Serialised_basic_Tagger) const = 0;
  virtual void deserialise(std::istream &Serialised_basic_Tagger) = 0;
  virtual void tag(Stream &Input, std::wostream &Output) = 0;
  virtual void train(Stream &TaggedCorpus) = 0;
  void outputLexicalUnit(
    const LexicalUnit &lexical_unit, const Optional<Analysis> analysis,
    std::wostream &output);
};
}

#endif // STREAM_TAGGER_H
