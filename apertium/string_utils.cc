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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */

#include <apertium/string_utils.h>
#include <lttoolbox/xml_parse_util.h>
#include <iostream>

//Delete white spaces from the end and the begining of the string
wstring 
StringUtils::trim(wstring const &str) 
{ 
  if(str == L"")
  {
    return L"";
  }

  int begin = 0, end = str.size() - 1;

  while(begin < end && str[begin] != L' ')
  {
    begin++;
  }

  while(end > begin && str[end] != L' ')
  {
    end--;
  }

  if(str[end] != L' ')
  {
    end++;
  }
 
  return str.substr(begin, end-begin);
}

vector<wstring>
StringUtils::split_wstring(wstring const &input, wstring const &delimiter) 
{
  unsigned pos;
  int new_pos;
  vector<wstring> result;
  wstring s = L"";
  pos=0;

  while(pos<input.size())
  {
    new_pos=input.find(delimiter, pos);
    if(new_pos<0)
      new_pos=input.size();
    s=input.substr(pos, new_pos-pos);
    if (s.length()==0) {
      wcerr<<L"Warning in StringUtils::split_wstring: After splitting there is an empty string\n";
      wcerr<<L"Skipping this empty string\n";
    } else
      result.push_back(s);
    pos=new_pos+delimiter.size();
  }

  return result;
}

wstring 
StringUtils::vector2wstring(vector<wstring> const &v)
{
  wstring s = L"";
  for(unsigned i=0; i<v.size(); i++)
  {
    if (i>0)
      s+=L' ';
    s.append(v[i]);
  }
  return s;
}

wstring 
StringUtils::substitute(wstring const &source, wstring const &olds, wstring const &news) {
  wstring s = source;

  unsigned p=s.find(olds , 0);
  while (p!=wstring::npos)
  {
    s.replace(p, olds.length(), news);
    p+=news.length();
    p=s.find(olds,p);
  }

  return s;
}

wstring
StringUtils::itoa(int n)
{
  char str[256];
  sprintf(str, "%d",n);
  return XMLParseUtil::stows(str);
}

wstring
StringUtils::ftoa(double f)
{
  char str[256];
  sprintf(str, "%f",f);
  return XMLParseUtil::stows(str);
}

wstring
StringUtils::tolower(wstring const &s)
{
  wstring l=s;
  for(unsigned i=0; i<s.length(); i++)
  {
    l[i] = (wchar_t) towlower(s[i]);
  }
  return l;
}

wstring
StringUtils::toupper(wstring const &s) {
  wstring l=s;
  for(unsigned i=0; i<s.length(); i++)
  {
    l[i]  = (wchar_t) towupper(s[i]);
  }

  return l;
}
