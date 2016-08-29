#include <apertium/stream_tagger.h>
#include <apertium/stream.h>
#include <utility>

namespace Apertium {
StreamTagger::~StreamTagger() {}

void StreamTagger::outputLexicalUnit(
    const LexicalUnit &lexical_unit, const Optional<Analysis> analysis,
    std::wostream &output) const {
  Stream::outputLexicalUnit(lexical_unit, analysis, output, TheFlags);
}
}
