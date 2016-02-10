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

#ifndef BASIC_EXCEPTION_TYPE_H
#define BASIC_EXCEPTION_TYPE_H

#include <exception>

namespace Apertium {
class basic_ExceptionType : public std::exception {
public:
  virtual ~basic_ExceptionType() throw() = 0;
  virtual const char *what() const throw() = 0;
};
}

#endif // BASIC_EXCEPTION_TYPE_H
