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

#include "analysis.h"

#include "exception.h"
#include "morpheme.h"

#include <string>
#include <vector>

namespace Apertium {
std::wostream &operator<<(std::wostream &Stream_, const Analysis &Analysis_) {
  Stream_ << static_cast<std::wstring>(Analysis_);
  return Stream_;
}

bool operator==(const Analysis &a, const Analysis &b) {
  return a.TheMorphemes == b.TheMorphemes;
}

bool operator<(const Analysis &a, const Analysis &b) {
  return a.TheMorphemes < b.TheMorphemes;
}

Analysis::operator std::wstring() const {
  if (TheMorphemes.empty())
    throw Exception::Analysis::TheMorphemes_empty(
        "can't convert Analysis comprising empty Morpheme std::vector to "
        "std::wstring");

  std::vector<Morpheme>::const_iterator Morpheme_ = TheMorphemes.begin();
  std::wstring wstring_ = *Morpheme_;
  ++Morpheme_;

  // Call .end() each iteration to save memory.
  for (; Morpheme_ != TheMorphemes.end(); ++Morpheme_) {
    wstring_ += L"+" + static_cast<std::wstring>(*Morpheme_);
  }

  return wstring_;
}
}
