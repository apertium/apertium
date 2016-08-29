#ifndef STREAM_TAGGER_H
#define STREAM_TAGGER_H

#include "stream.h"
#include "basic_tagger.h"
#include <istream>
#include <ostream>

namespace Apertium {
class StreamTagger : protected virtual basic_Tagger {
public:
  virtual ~StreamTagger();
  virtual void deserialise(std::istream &Serialised_basic_Tagger) = 0;
  virtual void tag(Stream &Input, std::wostream &Output) const = 0;
  void outputLexicalUnit(
    const LexicalUnit &lexical_unit, const Optional<Analysis> analysis,
    std::wostream &output) const;
};
}

#endif // STREAM_TAGGER_H
