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

#ifndef LINEBREAK_H
#define LINEBREAK_H

#include <string>

namespace Apertium {
class linebreak {
public:
  static std::string linebreak_(std::string string_,
                                std::string::size_type col,
                                const std::string::size_type &wrapmargin);

private:
  static void linebreak_(std::string &string_, std::string::size_type &col,
                         const std::string::size_type &wrapmargin,
                         std::string::size_type &i_,
                         const std::string::size_type &j_);
};
}

#endif // LINEBREAK_H
