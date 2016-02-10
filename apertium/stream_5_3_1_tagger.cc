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

#include "stream_5_3_1_tagger.h"

#include "apertium_config.h"

#include "analysis.h"
#include "deserialiser.h"
#include "lexical_unit.h"
#include "stream.h"
#include "streamed_type.h"

#include <cstddef>
#include <istream>
#include <map>
#include <ostream>

#if ENABLE_DEBUG

#include <sstream>
#include <string>

#endif // ENABLE_DEBUG

namespace Apertium {
Stream_5_3_1_Tagger::Stream_5_3_1_Tagger(const Flags &Flags_)
    : basic_5_3_1_Tagger(), basic_StreamTagger(Flags_) {}

void Stream_5_3_1_Tagger::deserialise(std::istream &Serialised_basic_Tagger) {
  Model = Deserialiser<std::map<Analysis, std::size_t> >::deserialise(
      Serialised_basic_Tagger);
}

long double Stream_5_3_1_Tagger::score(const Analysis &Analysis_) const {
  return tokenCount_T(Analysis_);
}

long double Stream_5_3_1_Tagger::tokenCount_T(const Analysis &Analysis_) const {
  if (Model.find(Analysis_) == Model.end())
    return 1;

  return 1 + Model.find(Analysis_)->second;
}

#if ENABLE_DEBUG

std::wstring Stream_5_3_1_Tagger::score_DEBUG(const Analysis &Analysis_) const {
  std::wstringstream score_DEBUG_;
  score_DEBUG_ << tokenCount_T(Analysis_);
  return score_DEBUG_.str();
}

#endif // ENABLE_DEBUG

}
