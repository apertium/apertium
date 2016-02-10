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

#ifndef WCHAR_T_EXCEPTION_TYPE_H
#define WCHAR_T_EXCEPTION_TYPE_H

#include "basic_exception_type.h"

#include <cstddef>
#include <sstream>
#include <string>

namespace Apertium {
class wchar_t_ExceptionType : public basic_ExceptionType {
public:
  friend void swap(wchar_t_ExceptionType &a, wchar_t_ExceptionType &b);
  wchar_t_ExceptionType(const wchar_t *wchar_t_what_);
  wchar_t_ExceptionType(const std::wstring &wchar_t_what_);
  wchar_t_ExceptionType(const std::wstringstream &wchar_t_what_);
  wchar_t_ExceptionType(const wchar_t_ExceptionType &wchar_t_ExceptionType_);
  wchar_t_ExceptionType &
  operator=(wchar_t_ExceptionType wchar_t_ExceptionType_);
  virtual ~wchar_t_ExceptionType() throw();
  const char *what() const throw();

private:
  static std::size_t size(const wchar_t *wchar_t_what_);
  void constructor(const wchar_t *wchar_t_what_);
  char *what_;
};
}

#endif // WCHAR_T_EXCEPTION_TYPE_H
