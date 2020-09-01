#include <apertium/stream_tagger.h>

namespace Apertium {
StreamTagger::~StreamTagger() {}

void StreamTagger::outputLexicalUnit(
    const LexicalUnit &lexical_unit, const Optional<Analysis> analysis,
    std::wostream &output) {
  Stream::outputLexicalUnit(lexical_unit, analysis, output, TheFlags);
}
}
