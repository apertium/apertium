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

#include "a.h"

#include "analysis.h"
#include "exception.h"

namespace Apertium {
bool operator==(const a &a_, const a &b_) {
  return a_.TheTags == b_.TheTags && a_.TheMorphemes == b_.TheMorphemes;
}

bool operator<(const a &a_, const a &b_) {
  if (a_.TheTags == b_.TheTags)
    return a_.TheMorphemes < b_.TheMorphemes;

  return a_.TheTags < b_.TheTags;
}

a::a() : TheTags(), TheMorphemes() {}

a::a(const Analysis &Analysis_) : TheTags(), TheMorphemes() {
  if (Analysis_.TheMorphemes.empty())
    throw Exception::Analysis::TheMorphemes_empty("can't convert const "
                                                  "Analysis & comprising empty "
                                                  "Morpheme std::vector to a");

  if (Analysis_.TheMorphemes.front().TheTags.empty())
    throw Exception::Morpheme::TheTags_empty("can't convert const Analysis & "
                                             "comprising Morpheme comprising "
                                             "empty Tag std::vector to a");

  TheTags = Analysis_.TheMorphemes.front().TheTags;
  TheMorphemes = std::vector<Morpheme>(Analysis_.TheMorphemes.begin() + 1,
                                       Analysis_.TheMorphemes.end());
}
}
