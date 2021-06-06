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
  UString s = "";
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
  UString s = "";
  for(unsigned i=0; i<v.size(); i++)
  {
    if (i>0)
      s+=L' ';
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
  return XMLParseUtil::stows(itoa_string(n));
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
  char str[256];
  sprintf(str, "%f",f);
  return XMLParseUtil::stows(str);
}

UString
StringUtils::tolower(UString const &s)
{
  UString l=s;
  for(unsigned i=0; i<s.length(); i++)
  {
    l[i] = (wchar_t) towlower(s[i]);
  }
  return l;
}

UString
StringUtils::toupper(UString const &s) {
  UString l=s;
  for(unsigned i=0; i<s.length(); i++)
  {
    l[i]  = (wchar_t) towupper(s[i]);
  }

  return l;
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

#include "string_to_wostream.h"
