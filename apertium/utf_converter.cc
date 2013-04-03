/*
 * Copyright (C) 2013 Jimmy O'Regan
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
#include <apertium/utf_converter.h>
#include <iostream>
#include <cstdlib>
#include <apertium/string_utils.h>

using namespace Apertium;

using namespace std;

namespace UtfConverter
{
  wstring
  fromUtf8(string const &s)
  {
    wchar_t* out = (wchar_t *)malloc((sizeof(wchar_t)*s.size()+1));
    size_t ret = mbstowcs(out, s.c_str(), s.size());
    if(ret == -1)
    {
      free(out);
      return L"";
    }
    out[ret] = L'\0';
    wstring retval = out;
    free(out);
    return retval;
  }

  string
  toUtf8(wstring const &s)
  {
    char* out = (char *)malloc(sizeof(char)*((s.size()*4)+1));
    size_t ret = wcstombs(out, s.c_str(), s.size()*4);
    if(ret == -1)
    {
      free(out);
      return "";
    }
    out[ret] = '\0';
    string retval = out;
    free(out);
    return retval;
  }
}
