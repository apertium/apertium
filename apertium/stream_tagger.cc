#include <apertium/stream_tagger.h>
#include <utility>

namespace Apertium {
StreamTagger::~StreamTagger() {}

void StreamTagger::outputLexicalUnit(
    const LexicalUnit &lexical_unit, const Optional<Analysis> analysis,
    std::wostream &output) const {
  using namespace std::rel_ops;
  output << L"^";

  if (lexical_unit.TheAnalyses.empty()) {
    if (TheFlags.getShowSuperficial())
      output << lexical_unit.TheSurfaceForm << L"/";

    output << L"*" << lexical_unit.TheSurfaceForm << L"$";
    return;
  }

  if (TheFlags.getMark()) {
    if (lexical_unit.TheAnalyses.size() != 1)
      output << L"=";
  }

  if (TheFlags.getShowSuperficial())
    output << lexical_unit.TheSurfaceForm << L"/";

  output << *analysis;

  if (TheFlags.getFirst()) {
    for (std::vector<Analysis>::const_iterator other_analysis =
             lexical_unit.TheAnalyses.begin();
         // Call .end() each iteration to save memory.
         other_analysis != lexical_unit.TheAnalyses.end(); ++other_analysis) {
      if (*other_analysis != *analysis)
        output << L"/" << *other_analysis;
    }
  }

  output << L"$";
}
}
