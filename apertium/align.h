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

#ifndef ALIGN_H
#define ALIGN_H

#include <string>
#include <utility>
#include <vector>

namespace Apertium {
class align {
public:
  static void
  align_(const std::vector<std::pair<std::string, std::string> > &string_);

private:
  static std::string::size_type
  col(const std::vector<std::pair<std::string, std::string> > &string_);
};
}

#endif // ALIGN_H
