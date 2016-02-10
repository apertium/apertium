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

#ifndef A_H
#define A_H

#include "analysis.h"
#include "morpheme.h"
#include "tag.h"

#include <vector>

namespace Apertium {
class a {
public:
  friend bool operator==(const a &a_, const a &b_);
  friend bool operator<(const a &a_, const a &b_);
  a();
  a(const Analysis &Analysis_);
  std::vector<Tag> TheTags;
  std::vector<Morpheme> TheMorphemes;
};
}

#endif // A_H
