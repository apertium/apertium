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

#include <apertium/string_utils.h>
#include <lttoolbox/xml_parse_util.h>
#include <iostream>
#include <cstring>
#include <unicode/utf16.h>
#include <unicode/uchar.h>

#ifdef _MSC_VER
#define snprintf _snprintf
#endif

//Delete white spaces from the end and the begining of the string
UString
StringUtils::trim(UString const &str)
{
  if(str.empty())
  {
    return ""_u;
  }

  int begin = 0, end = str.size() - 1;

  while(begin < end && iswspace(str[begin]))
  {
    begin++;
  }

  while(end > begin && iswspace(str[end]))
  {
    end--;
  }

  if(!iswspace(str[end]))
  {
    end++;
  }

  return str.substr(begin, end-begin);
}

vector<UString>
StringUtils::split_UString(UString const &input, UString const &delimiter)
{
  unsigned pos;
  int new_pos;
  vector<UString> result;
  UString s;
  pos=0;

  while(pos<input.size())
  {
    new_pos=input.find(delimiter, pos);
    if(new_pos<0)
      new_pos=input.size();
    s=input.substr(pos, new_pos-pos);
    if (s.length()==0) {
      cerr<<"Warning in StringUtils::split_UString: After splitting there is an empty string\n";
      cerr<<"Skipping this empty string\n";
    } else
      result.push_back(s);
    pos=new_pos+delimiter.size();
  }

  return result;
}

UString
StringUtils::vector2UString(vector<UString> const &v)
{
  UString s;
  for(unsigned i=0; i<v.size(); i++)
  {
    if (i>0)
      s+=' ';
    s.append(v[i]);
  }
  return s;
}

UString
StringUtils::substitute(UString const &source, UString const &olds, UString const &news) {
  UString s = source;

  unsigned int p=s.find(olds , 0);
  while (p!=static_cast<unsigned int>(UString::npos))
  {
    s.replace(p, olds.length(), news);
    p+=news.length();
    p=s.find(olds,p);
  }

  return s;
}

UString
StringUtils::itoa(int n)
{
  UChar str[256];
  u_snprintf(str, 256, "%d", n);
  return str;
}

string
StringUtils::itoa_string(int n)
{
  char str[256];
  snprintf(str, 256, "%d", n);
  return str;
}

UString
StringUtils::ftoa(double f)
{
  UChar str[256];
  u_snprintf(str, 256, "%f", f);
  return str;
}

UString
StringUtils::tolower(UString const &s)
{
  UString l;
  l.reserve(s.size());
  size_t i = 0;
  UChar32 c;
  while (i < s.length()) {
    U16_NEXT(s.c_str(), i, s.size(), c);
    l += u_tolower(c);
  }
  return l;
}

UString
StringUtils::toupper(UString const &s)
{
  UString l;
  l.reserve(s.size());
  size_t i = 0;
  UChar32 c;
  while (i < s.length()) {
    U16_NEXT(s.c_str(), i, s.size(), c);
    l += u_toupper(c);
  }
  return l;
}

UString
StringUtils::getcase(const UString& str)
{
  UString ret = "aa"_u;
  if (str.empty()) {
    return ret;
  }
  size_t i = 0;
  size_t l = str.size();
  UChar32 c;
  U16_NEXT(str.c_str(), i, l, c);
  if (u_isupper(c)) {
    ret[0] = 'A';
    if (i < l) {
      U16_BACK_1(str.c_str(), i, l); // decrements l
      U16_GET(str.c_str(), 0, l, str.size(), c);
      if (u_isupper(c)) {
        ret[1] = 'A';
      }
    }
  }
  return ret;
}

UString
StringUtils::copycase(const UString& source, const UString& target)
{
  if (source.empty() || target.empty()) {
    return target;
  }
  UString ret;
  ret.reserve(target.size());
  size_t i = 0;
  size_t l = source.size();
  UChar32 c;
  U16_NEXT(source.c_str(), i, l, c);
  bool firstupper = u_isupper(c);
  bool uppercase = false;
  if (firstupper) {
    if (i != l) {
      U16_BACK_1(source.c_str(), i, l); // decrements l
      U16_GET(source.c_str(), 0, l, source.size(), c);
      uppercase = u_isupper(c);
    }
  }
  i = 0;
  l = target.size();
  if (firstupper) {
    U16_NEXT(target.c_str(), i, l, c);
    ret += u_toupper(c);
  }
  if (uppercase) {
    while (i < l) {
      U16_NEXT(target.c_str(), i, l, c);
      ret += u_toupper(c);
    }
  } else {
    while (i < l) {
      U16_NEXT(target.c_str(), i, l, c);
      ret += u_tolower(c);
    }
  }
  return ret;
}

bool Apertium::operator==(string const &s1, string const &s2)
{
  return strcmp(s1.c_str(), s2.c_str()) == 0;
}

bool Apertium::operator==(string const &s1, char const *s2)
{
  return strcmp(s1.c_str(), s2) == 0;
}

bool Apertium::operator==(char const *s1, string const &s2)
{
  return strcmp(s1, s2.c_str()) == 0;
}

bool Apertium::operator!=(string const &s1, string const &s2)
{
  return strcmp(s1.c_str(), s2.c_str()) != 0;
}

bool Apertium::operator!=(string const &s1, char const *s2)
{
  return strcmp(s1.c_str(), s2) != 0;
}

bool Apertium::operator!=(char const *s1, string const &s2)
{
  return strcmp(s1, s2.c_str()) != 0;
}

u16iter::u16iter(const UString& s)
  : buf(s.c_str()), len(s.size()), i(0), c('\0')
{
  if (!s.empty()) {
    U16_GET(buf, 0, 0, len, c);
  }
}

u16iter::u16iter(const u16iter& it)
  : buf(it.buf), len(it.len), i(it.i), c(it.c)
{}

u16iter&
u16iter::operator++()
{
  if (i < len) {
    U16_FWD_1(buf, i, len);
    if (i < len) {
      U16_GET(buf, 0, i, len, c);
    } else {
      c = '\0';
    }
  }
  return *this;
}

u16iter
u16iter::begin()
{
  u16iter ret(""_u);
  ret.buf = buf;
  ret.len = len;
  ret.i = 0;
  U16_GET(buf, 0, 0, len, ret.c);
  return ret;
}

u16iter
u16iter::end()
{
  u16iter ret(""_u);
  ret.buf = buf;
  ret.len = len;
  ret.i = len;
  ret.c = '\0';
  return ret;
}

bool
u16iter::operator!=(const u16iter& other) const {
  return other.buf != buf || other.i != i;
}

bool
u16iter::operator==(const u16iter& other) const {
  return other.buf == buf && other.i == i;
}

#include "string_to_wostream.h"
