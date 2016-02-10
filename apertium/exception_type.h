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

#ifndef EXCEPTION_TYPE_H
#define EXCEPTION_TYPE_H

#include "basic_exception_type.h"

#include <sstream>
#include <string>

namespace Apertium {
class ExceptionType : public basic_ExceptionType {
public:
  ExceptionType(const char *const what_);
  ExceptionType(const std::string &what_);
  ExceptionType(const std::stringstream &what_);
  virtual ~ExceptionType() throw() = 0;
  const char *what() const throw();

protected:
  const std::string what_;
};
}

#endif // EXCEPTION_TYPE_H
