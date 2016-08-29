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

#include "morpheme.h"

#include "exception.h"
#include "tag.h"

#include <iostream>
#include <string>
#include <vector>

namespace Apertium {
bool operator==(const Morpheme &a, const Morpheme &b) {
  return a.TheLemma == b.TheLemma && a.TheTags == b.TheTags;
}

bool operator<(const Morpheme &a, const Morpheme &b) {
  if (a.TheLemma != b.TheLemma)
    return a.TheLemma < b.TheLemma;

  return a.TheTags < b.TheTags;
}

std::wostream& operator<<(std::wostream& out, const Morpheme &morph) {
  out << morph.TheLemma;
  const std::vector<Tag> &tags = morph.TheTags;
  std::vector<Tag>::const_iterator it = tags.begin();
  for (; it != tags.end(); it++) {
    out << L"<" << it->TheTag << L">";
  }
  return out;
}

Morpheme::operator std::wstring() const {
  if (TheTags.empty())
    throw Exception::Morpheme::TheTags_empty("can't convert Morpheme "
                                             "comprising empty Tag std::vector "
                                             "to std::wstring");

  if (TheLemma.empty())
    throw Exception::Morpheme::TheLemma_empty("can't convert Morpheme "
                                              "comprising empty TheLemma "
                                              "std::wstring to std::wstring");

  std::wstring wstring_ = TheLemma;

  for (std::vector<Tag>::const_iterator Tag_ = TheTags.begin();
       // Call .end() each iteration to save memory.
       Tag_ != TheTags.end(); ++Tag_) {
    wstring_ += static_cast<std::wstring>(*Tag_);
  }

  return wstring_;
}
}
