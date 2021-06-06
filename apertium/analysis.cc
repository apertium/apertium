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
// along with this program; if not, see <https://www.gnu.org/licenses/>.

#include "analysis.h"

#include "exception.h"
#include "morpheme.h"

#include <string>
#include <vector>

namespace Apertium {
std::ostream &operator<<(std::ostream &Stream_, const Analysis &Analysis_) {
  //Stream_ << static_cast<UString>(Analysis_);
  // this line is giving a type error that I can't make sense of
  // wstring wcerr TODO L""
  return Stream_;
}

bool operator==(const Analysis &a, const Analysis &b) {
  return a.TheMorphemes == b.TheMorphemes;
}

bool operator<(const Analysis &a, const Analysis &b) {
  return a.TheMorphemes < b.TheMorphemes;
}

Analysis::operator UString() const {
  if (TheMorphemes.empty())
    throw Exception::Analysis::TheMorphemes_empty(
        "can't convert Analysis comprising empty Morpheme std::vector to "
        "UString");

  std::vector<Morpheme>::const_iterator Morpheme_ = TheMorphemes.begin();
  UString UString_ = *Morpheme_;
  ++Morpheme_;

  // Call .end() each iteration to save memory.
  for (; Morpheme_ != TheMorphemes.end(); ++Morpheme_) {
    UString_ += '+';
    UString_ += static_cast<UString>(*Morpheme_);
  }

  return UString_;
}
}
