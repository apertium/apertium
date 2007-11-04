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
#include <apertium/string_utils.h>
#include <lttoolbox/ltstr.h>
#include <lttoolbox/compression.h>

#include <cstdlib>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <apertium/string_utils.h>

using namespace Apertium;
TMXBuilder::TMXBuilder(wstring const &l1, wstring const &l2)
{
  lang1 = l1;
  lang2 = l2;
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
    while(!feof(f1) && !feof(f2))
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
    while(!feof(f1) && !feof(f2))
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

wstring
TMXBuilder::nextTU(FILE *input)
{
  wstring current_tu = L"";
  wstring tmp;
  
  while(true)
  {
    wint_t symbol = fgetwc_unlocked(input);
    if(feof(input))
    {
      return L"";
    }
    switch(symbol)
    {
      case L'\\':
        symbol = fgetwc_unlocked(input);
        if(feof(input))
        {
          return L"";
        }
        // continued down
      default:
        current_tu += static_cast<wchar_t>(symbol);
        break;
      
      case L'[':
        tmp = restOfBlank(input);
        current_tu += L' ';
        break;
      
      case L'.':
        symbol = fgetwc_unlocked(input);
        if(symbol != L'[' && !iswspace(symbol))
        {
          ungetwc(symbol, input);
          current_tu += L'.';
        }
        else
        {
          ungetwc(symbol, input);
          size_t idx = current_tu.size()-1;
          while(current_tu[idx] == L'.')
          {
            idx--;
          }
          return current_tu.substr(0, idx+1);
        }
        break;
      
      case L'?':
      case L'!':
        current_tu += static_cast<wchar_t>(symbol);
        return current_tu;
    }
  }
  return L"";
}

void 
TMXBuilder::generate(string const &file1, string const &file2, 
                     string const &outfile) const
{
  FILE *output = stdout;
  
  if(outfile != "")
  {
    output = fopen(outfile.c_str(), "w");
    if(!output)
    {
      wcerr << L"Error: file '" << UtfConverter::fromUtf8(outfile);
      wcerr << L"' cannot be opened for writing" << endl;
      exit(EXIT_FAILURE);
    }
  }

  FILE *f1 = fopen(file1.c_str(), "r");
  if(!f1)
  {
    wcerr << L"Error: file '" << UtfConverter::fromUtf8(file1);
    wcerr << L"' cannot be opened for reading" << endl;
    exit(EXIT_FAILURE);
  }

  FILE *f2 = fopen(file2.c_str(), "r");
  if(!f2)
  {
    wcerr << L"Error: file '" << UtfConverter::fromUtf8(file2);
    wcerr << L"' cannot be opened for reading" << endl;
    exit(EXIT_FAILURE);
  }

  set<wstring, Ltstr> storage;
  
  storage.insert(L"|#|");
  wstring tu1 = StringUtils::trim(nextTU(f1));
  wstring tu2 = StringUtils::trim(nextTU(f2));
  
  while(!feof(f1) && !feof(f2))
  {
    if(storage.find(tu1 + L"|#|" + tu2) == storage.end())
    { 
      storage.insert(tu1 + L"|#|" + tu2);
      fwprintf(output, L"<tu>\n  <tuv xml:lang=\"%ls\">%ls</tuv>\n", lang1.c_str(), tu1.c_str());
      fwprintf(output, L"  <tuv xml:lang=\"%ls\">%ls</tuv>\n</tu>\n", lang2.c_str(), tu2.c_str());  
    }
    tu1 = StringUtils::trim(nextTU(f1));
    tu2 = StringUtils::trim(nextTU(f2));
  }

  if(output != stdin)
  {
    fclose(output);
  }
  fclose(f1);
  fclose(f2);
}
