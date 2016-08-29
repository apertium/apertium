// Copyright (C) 2005 Universitat d'Alacant / Universidad de Alicante
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 2 of the
// License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.

#include "basic_stream_tagger.h"

#include "apertium_config.h"

#include "basic_tagger.h"
#include "lexical_unit.h"
#include "stream.h"
#include "streamed_type.h"

#include <ostream>

#if ENABLE_DEBUG

#include <iomanip>
#include <iostream>
#include <limits>

#endif // ENABLE_DEBUG

namespace Apertium {
basic_StreamTagger::~basic_StreamTagger() {}

void basic_StreamTagger::tag(Stream &Input, std::wostream &Output) const {
  while (true) {
    StreamedType StreamedType_ = Input.get();
    Output << StreamedType_.TheString;

    if (!StreamedType_.TheLexicalUnit) {
      if (!Input.flush_())
        break;

      Output << std::flush;
      continue;
    }

#if ENABLE_DEBUG

    std::wcerr << L"\n\n";

#endif // ENABLE_DEBUG

    tag(*StreamedType_.TheLexicalUnit, Output);

    if (Input.flush_())
      Output << std::flush;
  }
}

void basic_StreamTagger::tag(const LexicalUnit &LexicalUnit_,
                             std::wostream &Output) const {
#if ENABLE_DEBUG

  for (std::vector<Analysis>::const_iterator Analysis_ =
           LexicalUnit_.TheAnalyses.begin();
       Analysis_ != LexicalUnit_.TheAnalyses.end(); ++Analysis_) {
    std::wcerr << L"score(\"" << *Analysis_ << L"\") ==\n  "
               << score_DEBUG(*Analysis_) << L" ==\n  " << std::fixed
               << std::setprecision(std::numeric_limits<long double>::digits10)
               << score(*Analysis_) << L"\n";
  }

#endif // ENABLE_DEBUG

  Optional<Analysis> TheAnalysis;

  if (!LexicalUnit_.TheAnalyses.empty()) {
    TheAnalysis = *LexicalUnit_.TheAnalyses.begin();
    for (std::vector<Analysis>::const_iterator Analysis_ =
             LexicalUnit_.TheAnalyses.begin() + 1;
         // Call .end() each iteration to save memory.
         Analysis_ != LexicalUnit_.TheAnalyses.end(); ++Analysis_) {
      if (score(*Analysis_) > score(*TheAnalysis))
        TheAnalysis = *Analysis_;
    }
  }

  outputLexicalUnit(LexicalUnit_, TheAnalysis, Output);
}
}
