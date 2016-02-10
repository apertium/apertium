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

#ifndef STREAM_5_3_1_TAGGER_H
#define STREAM_5_3_1_TAGGER_H

#include "apertium_config.h"

#include "analysis.h"
#include "basic_5_3_1_tagger.h"
#include "basic_stream_tagger.h"

#include <istream>

#if ENABLE_DEBUG

#include <string>

#endif // ENABLE_DEBUG

namespace Apertium {
class Stream_5_3_1_Tagger : private basic_5_3_1_Tagger,
                            public basic_StreamTagger {
public:
  Stream_5_3_1_Tagger(const Flags &Flags_);
  void deserialise(std::istream &Serialised_basic_Tagger);

private:
  long double score(const Analysis &Analysis_) const;
  long double tokenCount_T(const Analysis &Analysis_) const;

#if ENABLE_DEBUG

  std::wstring score_DEBUG(const Analysis &Analysis_) const;

#endif // ENABLE_DEBUG

};
}

#endif // STREAM_5_3_1_TAGGER_H
