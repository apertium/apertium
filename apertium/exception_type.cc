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

#include "exception_type.h"

#include <sstream>
#include <string>

namespace {
// ToDo: ABI break to change member what_ back to std::string
std::string res;
}

namespace Apertium {
ExceptionType::ExceptionType(const char *const what_)
  : what_(to_ustring(what_)) {}

ExceptionType::ExceptionType(const std::string &what_)
  : what_(to_ustring(what_.c_str())) {}

ExceptionType::ExceptionType(const std::stringstream &what_)
  : what_(to_ustring(what_.str().c_str())) {}

ExceptionType::ExceptionType(const UChar *const what_)
  : what_(what_) {}

ExceptionType::ExceptionType(const UString &what_)
  : what_(what_) {}

ExceptionType::~ExceptionType() throw() {}

const char *ExceptionType::what() const throw()
{
  res.clear();
  utf8::utf16to8(what_.begin(), what_.end(), std::back_inserter(res));
  return res.c_str();
}
}
