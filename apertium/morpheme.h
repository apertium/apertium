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

#ifndef MORPHEME_H
#define MORPHEME_H

#include "tag.h"

#include <string>
#include <vector>

namespace Apertium {
class Morpheme {
public:
  friend bool operator==(const Morpheme &a, const Morpheme &b);
  friend bool operator<(const Morpheme &a, const Morpheme &b);
  operator std::wstring() const;
  std::wstring TheLemma;
  std::vector<Tag> TheTags;
};
}

#endif // MORPHEME_H
