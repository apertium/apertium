/*
 * Copyright (C) 2006 Universitat d'Alacant / Universidad de Alicante
 * author: Felipe Sánchez-Martínez
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 */
#ifndef __STRINGUTILS_H_
#define __STRINGUTILS_H_

#include <string>
#include <cstring>
#include <vector>
#include <lttoolbox/ustring.h>

using namespace std;

namespace Apertium
{
  bool operator==(string const &s1, string const &s2);
  bool operator==(string const &s1, char const *s2);
  bool operator==(char const *s1, string const &s2);
  bool operator!=(string const &s1, string const &s2);
  bool operator!=(string const &s1, char const *s2);
  bool operator!=(char const *s1, string const &s2);
}

class StringUtils {
  public:

  static UString trim(UString const &str);

  static vector<UString> split_UString(UString const &input, UString const &delimiter);

  static UString vector2UString(vector<UString> const &v);

  //Replace each ocurrence of the string 'olds' by the string 'news' in string 'source'
  static UString substitute(const UString &source, const UString &olds, const UString &news);

  static UString itoa(int n);

  static string itoa_string(int n);

  static UString ftoa(double f);

  static UString tolower(UString const &s);

  static UString toupper(UString const &s);

  static UString getcase(const UString& s);

  static UString copycase(const UString& source, const UString& target);
};

class u16iter
{
private:
  const UChar* buf;
  size_t len;
  size_t i;
  UChar32 c;
public:
  u16iter(const UString& s);
  u16iter(const u16iter& it);

  u16iter& operator++();
  u16iter begin();
  u16iter end();
  inline UChar32 operator*() const { return c; }
  bool operator!=(const u16iter& other) const;
  bool operator==(const u16iter& other) const;
};

std::wostream & operator<< (std::wostream & ostr, std::string const & str);

#endif
