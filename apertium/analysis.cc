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

#include <lttoolbox/i18n.h>

namespace Apertium {
std::ostream &operator<<(std::ostream &Stream_, const Analysis &Analysis_) {
  ::operator<<(Stream_, static_cast<UString>(Analysis_));
  //Stream_ << static_cast<UString>(Analysis_);
  // namespace issue
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
    throw Exception::Analysis::TheMorphemes_empty(I18n(APR_I18N_DATA, "apertium").format("APR80150"));

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

void
Analysis::read(InputFile& in)
{
  UChar32 c;
  do {
    TheMorphemes.push_back(Morpheme());
    TheMorphemes.back().read(in);
    c = in.get();
  } while (c == '+');
  if (in.eof() || c == '\0') {
    throw Exception::Stream::UnexpectedEndOfFile(I18n(APR_I18N_DATA, "apertium").format("APR80160"));
  }
  in.unget(c); // leave $ or / for caller
}
}
