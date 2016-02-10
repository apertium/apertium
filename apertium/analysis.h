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

#ifndef ANALYSIS_H
#define ANALYSIS_H

#include "morpheme.h"

#include <ostream>
#include <string>
#include <vector>

namespace Apertium {
class Analysis {
public:
  friend std::wostream &operator<<(std::wostream &Stream_,
                                   const Analysis &Analysis_);
  friend bool operator==(const Analysis &a, const Analysis &b);
  friend bool operator<(const Analysis &a, const Analysis &b);
  operator std::wstring() const;
  std::vector<Morpheme> TheMorphemes;
};
}

#endif // ANALYSIS_H
