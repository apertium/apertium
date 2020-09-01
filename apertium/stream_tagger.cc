#include <apertium/stream_tagger.h>

namespace Apertium {
StreamTagger::StreamTagger() : TheFlags() {}

StreamTagger::StreamTagger(TaggerFlags& Flags_) : TheFlags(Flags_) {}

StreamTagger::~StreamTagger() {}

void StreamTagger::outputLexicalUnit(
    const LexicalUnit &lexical_unit, const Optional<Analysis> analysis,
    std::wostream &output) {
  Stream::outputLexicalUnit(lexical_unit, analysis, output, TheFlags);
}
}
