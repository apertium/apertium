/*
 * Copyright (C) 2005 Universitat d'Alacant / Universidad de Alicante
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
#include <apertium/tmx_builder.h>
#include <apertium/utf_converter.h>
#include <lttoolbox/compression.h>

#include <cstdlib>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

TMXBuilder::TMXBuilder()
{
}

TMXBuilder::~TMXBuilder()
{
}

wstring 
TMXBuilder::restOfBlank(FILE *input)
{
  wstring result = L"[";
  
  while(true)
  {
    wint_t val = fgetwc(input);
    if(feof(input))
    {
      return L"";
    }
    switch(val)
    {
      case L'\\':
        result += L'\\';
        val = fgetwc(input);
        if(feof(input))
        {
          return L"";
        }
        result += static_cast<wchar_t>(val);
        break;
      
      case L']':
        result += L']';
        return result;
        
      default:
        result += static_cast<wchar_t>(val);
        break;
    }
  }
  
  return L"";
}

wstring 
TMXBuilder::nextBlank(FILE *input)
{
  wstring result = L"";
  
  while(true)
  {
    wint_t val = fgetwc(input);
    if(feof(input))
    {
      return L"";
    }
    switch(val)
    {
      case L'\\':
        fgetwc(input);
        break;
      case L'[':
        
        result = restOfBlank(input);
        return result;
    }
  }  
}

bool
TMXBuilder::compatible(FILE *f1, FILE *f2, bool lazy)
{
  wstring s1 = nextBlank(f1), s2 = nextBlank(f2);
  if(!lazy)
  {  
    while(s1 != L"")
    {
      if(s1 != s2)
      {
        return false;
      }
      s1 = nextBlank(f1);
      s2 = nextBlank(f2);
    }
  }    
  else
  {
    while(s1 != L"")
    {
      if(s1.size() < s2.size()*(1-0.05) || s1.size() > s2.size()*(1+0.05))
      {
        return false;
      }
      s1 = nextBlank(f1);
      s2 = nextBlank(f2);
    }    
  }
  return true;
}

bool
TMXBuilder::check(string const &file1, string const &file2, bool lazy)
{
  FILE *f1 = fopen(file1.c_str(), "r");
  FILE *f2 = fopen(file2.c_str(), "r");
  if(!f1 && !f2)
  {
    wcerr << L"Error: Cannot access files '" << UtfConverter::fromUtf8(file1);
    wcerr << L"' and '" << UtfConverter::fromUtf8(file2) << "'" << endl;
    return false;
  }
  else if(!f1)
  {
    wcerr << L"Error: Cannot access file '";
    wcerr << UtfConverter::fromUtf8(file2);
    wcerr << "'" << endl;
    return false;
  }
  else if(!f2)
  {
    wcerr << L"Error: Cannot access file '";
    wcerr << UtfConverter::fromUtf8(file2);
    wcerr << "'" << endl;
  }
     
  bool retval = compatible(f1, f2, lazy);

  fclose(f1);
  fclose(f2);
  return retval;
}
