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
#include <apertium/unlocked_cstdio.h>

#ifdef WIN32
#include <io.h>
#include <fcntl.h>
#endif

using namespace Apertium;
using namespace std;

TMXBuilder::TMXBuilder(wstring const &l1, wstring const &l2)
{
  lang1 = l1;
  lang2 = l2;

  // by default: percent = 0.95, lowLimit = 10
  percent = 0.95;
  lowLimit = 10;
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
      if(current_tu == L"")
      {
        return L"";
      }
      else
      {
        return current_tu;
      }
    }
    switch(symbol)
    {
      case L'\\':
        symbol = fgetwc_unlocked(input);
        if(feof(input))
        {
          if(current_tu == L"")
          {
            return L"";
          }
          else
          {
            return current_tu;
          }
        }
        // continued down
      default:
        current_tu += static_cast<wchar_t>(symbol);
        break;
      
      case L'[':
        tmp = restOfBlank(input);
        if(tmp.substr(0,2) == L"[ ")
        {
          current_tu.append(L" ");
        }  
        current_tu.append(L"<ph/>");
        if(tmp.substr(tmp.size()-2, 2) == L" ]")
        {
          current_tu.append(L" ");
        }   
        break;
      
      case L'.':
        current_tu += L'.';
        symbol = fgetwc_unlocked(input);

        if(symbol != L'[' && !iswspace(symbol))
        {
          if(!feof(input))          
          {
            ungetwc(symbol, input);
          }
        }
        else
        {
          if(!feof(input))
          {
            ungetwc(symbol, input);
          }

          return current_tu;
/*          size_t idx = current_tu.size()-1;
          while(current_tu[idx] == L'.')
          {
            idx--;
          }
          return current_tu.substr(0, idx+1);*/
        }
        break;
      
      case L'?':
      case L'!':
        current_tu += static_cast<wchar_t>(symbol);
        return current_tu;
    }
  }
  
  return current_tu;
}

wstring
TMXBuilder::xmlize(wstring const &str)
{
  wstring result = L"";
  
  for(size_t i = 0, limit = str.size(); i < limit; i++)
  {
    switch(str[i])
    {
      case L'<':
        if(i + 5 <= limit && str.substr(i,5)==L"<ph/>")
        {
          result.append(L"<ph/>");
          i += 4;
          break;
        }
        else
        {
          result.append(L"&lt;");
        }
        break;
        
      case L'>':
        result.append(L"&gt;");
        break;
        
      case L'&':
        result.append(L"&amp;");
        break;
      
      default:
        result += str[i];
        break;
    }
  }
  
  // remove leading <ph/>'s
  
  bool cambio = true;
  while(cambio == true)
  {
    cambio = false;
    while(result.size() >= 5 && result.substr(0,5) == L"<ph/>")
    {
      result = result.substr(5);
      cambio = true;
    }
    while(result.size() > 0 && !iswalnum(result[0]) && !iswpunct(result[0]))
    {
      result = result.substr(1);
      cambio = true;
    }
  }
  // remove trailing <ph/>'s
  
  cambio = true;
  while(cambio == true)
  {
    cambio = false;
    while(result.size() > 5 && result.substr(result.size()-5) == L"<ph/>")
    {
      result = result.substr(0, result.size()-5);
      cambio = true;
    }
    while(result.size() > 0 && !iswalnum(result[result.size()-1]) && !iswpunct(result[result.size()-1]))
    {
      result = result.substr(0, result.size()-1);
      cambio = true;
    }
  }
  
  return result;
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
#ifdef WIN32
  _setmode(_fileno(output), _O_U8TEXT);
#endif

  // Generate the TMX in UTF-8
  
  fprintf(output, "<?xml version=\"1.0\"?>\n");
  fprintf(output, "<tmx version=\"version 1.1\">\n");
  fprintf(output, "<header creationtool=\"Apertium TMX Builder\">\n");
  fprintf(output, "</header>");
  fprintf(output, "<body>");
  
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

#ifdef WIN32
  _setmode(_fileno(f1), _O_U8TEXT);
  _setmode(_fileno(f2), _O_U8TEXT);
#endif   

  set<wstring, Ltstr> storage;
  
  storage.insert(L"|#|");
  wstring tu1 = L""; 
  wstring tu2 = L""; 
  
  do
  {
    tu1 = xmlize(tu1);
    tu2 = xmlize(tu2);
    if(tu1 != L"" && tu2 != L"" &&
       storage.find(tu1 + L"|#|" + tu2) == storage.end())
    { 
      tu1 = removeLastPeriod(tu1);
      tu2 = removeLastPeriod(tu2);
      storage.insert(tu1 + L"|#|" + tu2);
      fprintf(output, "<tu>\n  <tuv xml:lang=\"%s\"><seg>%s</seg></tuv>\n", UtfConverter::toUtf8(lang1).c_str(), UtfConverter::toUtf8(tu1).c_str());
      fprintf(output, "  <tuv xml:lang=\"%s\"><seg>%s</seg></tuv>\n</tu>\n", UtfConverter::toUtf8(lang2).c_str(), UtfConverter::toUtf8(tu2).c_str());  
    }
    tu1 = nextTU(f1);    
    tu2 = nextTU(f2);
    int contador = 0;
    
    fpos_t pos1, pos2;
    
    fgetpos(f1, &pos1);
    fgetpos(f2, &pos2);    
    while(!similar(tu1, tu2) && contador <= 4)
    {
      contador++;
      if(tu1.size() < tu1.size())
      {
        tu1 += nextTU(f1);
      }
      else
      {
        tu2 += nextTU(f2);
      }
    }
    
    if(contador == 4 && !similar(tu1, tu2))
    {
      fsetpos(f1, &pos1);
      fsetpos(f2, &pos2);
      tu1 == L"";
      tu2 == L"";
    }
  }
  while((tu1 != L"" || !feof(f1)) && (tu2 != L"" || !feof(f2)));

  fprintf(output, "</body>\n</tmx>\n");

  if(output != stdin)
  {
    fclose(output);
  }
  fclose(f1);
  fclose(f2);
}

bool 
TMXBuilder::similar(wstring const &str1, wstring const &str2) const
{
  if(abs(int(str1.size())-int(str2.size())) <= lowLimit)
  {
    return true;
  }
  else
  {
    if(str1.size() < str2.size())
    {
      return double(str1.size())/double(str2.size()) >= percent;
    }
    else
    {
      return double(str2.size())/double(str1.size()) >= percent;
    }
  }  
}

void
TMXBuilder::setPercent(double const p)
{
  percent = p;
}

void
TMXBuilder::setLowLimit(int const l)
{
  lowLimit = l;
}

wstring
TMXBuilder::removeLastPeriod(wstring const &str)
{
  unsigned int index = str.size() - 1;
  
  if(index > 1)
  {
    if(str[index] == L'.' && iswalnum(str[index-1]))
    {
      return str.substr(0, index);
    }
  }
  
  return str;
}
