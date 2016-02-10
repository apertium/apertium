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

#include "wchar_t_exception_type.h"

#include "exception.h"

#include <algorithm>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <cwchar>
#include <sstream>
#include <string>

namespace Apertium {
void swap(wchar_t_ExceptionType &a, wchar_t_ExceptionType &b) {
  using std::swap;

  swap(a.what_, b.what_);
}

wchar_t_ExceptionType::wchar_t_ExceptionType(const wchar_t *wchar_t_what_)
    : what_(new char[size(wchar_t_what_)]) {
  constructor(wchar_t_what_);
}

wchar_t_ExceptionType::wchar_t_ExceptionType(const std::wstring &wchar_t_what_)
    : what_(new char[size(wchar_t_what_.c_str())]) {
  constructor(wchar_t_what_.c_str());
}

wchar_t_ExceptionType::wchar_t_ExceptionType(
    const std::wstringstream &wchar_t_what_)
    : what_(new char[size(wchar_t_what_.str().c_str())]) {
  constructor(wchar_t_what_.str().c_str());
}

wchar_t_ExceptionType::wchar_t_ExceptionType(
    const wchar_t_ExceptionType &wchar_t_ExceptionType_)
    : what_(new char[std::strlen(wchar_t_ExceptionType_.what_) + 1]) {
  std::strcpy(what_, wchar_t_ExceptionType_.what_);
}

wchar_t_ExceptionType &wchar_t_ExceptionType::
operator=(wchar_t_ExceptionType wchar_t_ExceptionType_) {
  swap(*this, wchar_t_ExceptionType_);
  return *this;
}

wchar_t_ExceptionType::~wchar_t_ExceptionType() throw() { delete[] what_; }

const char *wchar_t_ExceptionType::what() const throw() { return what_; }

std::size_t wchar_t_ExceptionType::size(const wchar_t *wchar_t_what_) {
  std::mbstate_t ps = {0};
  errno = 0;
  std::size_t size_ = std::wcsrtombs(NULL, &wchar_t_what_, 0, &ps);

  if (errno == EILSEQ)
    throw Exception::wchar_t_ExceptionType::EILSEQ_(
        "can't convert const wchar_t *wchar_t_what_ to char * : unexpected "
        "wide character");

  return size_ + 1;
}

void wchar_t_ExceptionType::constructor(const wchar_t *wchar_t_what_) {
  std::mbstate_t ps = {0};
  errno = 0;
  std::wcsrtombs(what_, &wchar_t_what_, size(wchar_t_what_), &ps);

  if (errno == EILSEQ)
    throw Exception::wchar_t_ExceptionType::EILSEQ_(
        "can't convert const wchar_t *const wchar_t_what_ to char *what_: "
        "unexpected wide character");
}
}
