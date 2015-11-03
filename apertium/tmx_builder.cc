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
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */
#include <apertium/tmx_builder.h>
#include <apertium/utf_converter.h>
#include <apertium/string_utils.h>
#include <apertium/tmx_aligner_tool.h>
#include <lttoolbox/ltstr.h>
#include <lttoolbox/compression.h>


#include <cmath>
#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <apertium/string_utils.h>
#include "apertium_config.h"
#include <apertium/unlocked_cstdio.h>

#ifdef _MSC_VER
#include <io.h>
#include <fcntl.h>
#endif

using namespace Apertium;
using namespace std;

TMXBuilder::TMXBuilder(wstring const &l1, wstring const &l2):
low_limit(0)
{
  lang1 = l1;
  lang2 = l2;

  // default values of the parameters
    
  max_edit = 50;
  diagonal_width = 10;
  window_size = 100;
  step = 75;
  percent=0.85;
  edit_distance_percent=0.30;
 
  freference = NULL;
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
  FILE *f1 = fopen(file1.c_str(), "rb");
  FILE *f2 = fopen(file2.c_str(), "rb");
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
    fclose(f2);
    return false;
  }
  else if(!f2)
  {
    wcerr << L"Error: Cannot access file '";
    wcerr << UtfConverter::fromUtf8(file2);
    wcerr << "'" << endl;
    fclose(f1);
    return false;
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
  
  // remove trailing punctuation

  
  for(unsigned int i = result.size()-1; result.size() > 0 && i > 0; i--)
  {
    if(!isRemovablePunct(result[i]))
    {
      result = result.substr(0, i+1);
      break;
    }
  }

  while(result.size() > 0 && isRemovablePunct(result[result.size()-1]))
  {
    result = result.substr(0,result.size()-1);
  }

  return result;
} 

void 
TMXBuilder::generate(string const &file1, string const &file2, 
                     string const &outfile)
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
#ifdef _MSC_VER
  _setmode(_fileno(output), _O_U8TEXT);
#endif

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

#ifdef _MSC_VER
  _setmode(_fileno(f1), _O_U8TEXT);
  _setmode(_fileno(f2), _O_U8TEXT);
#endif   
 
  generateTMX(f1, f2, output);
}

vector<wstring>
TMXBuilder::reverseList(vector<wstring> const &v)
{
  vector<wstring> retval(v.size());
 
  for(int j = v.size() - 1, i = 0; j >=0; j--, i++)
  {
    retval[i] = v[j];
  }
  
  return retval;
}

vector<wstring>
TMXBuilder::sentenceList(FILE *file)
{
  vector<wstring> retval;
  
  while(true)
  {
    wstring f = nextTU(file);
    if(feof(file))
    {
      break;
    }   
    retval.push_back(f);
  }
  
  return retval;
}  

vector<wstring>
TMXBuilder::extractFragment(vector<wstring> const &text, unsigned int base, unsigned int width)
{
  vector<wstring> result;
  
  for(unsigned int i = base; i < (base + width) && i < text.size(); i++)
  {
    result.push_back(text[i]);
  }
  
  return result;
}

int
TMXBuilder::argmin(int nw, int n, int w)
{
  if(nw <= n)
  {
    if(nw <= w)
    {
      return 1;
    }
    else
    {
      return 3;
    }
  }
  else if(n <= w)
  {
    return 2;
  }
  else
  {
    return 3;
  }
}  

void
TMXBuilder::generateTMX(FILE *f1, FILE *f2, FILE *output)
{
  fprintf(output, "<?xml version=\"1.0\"?>\n");
  fprintf(output, "<tmx version=\"1.4\">\n");
  fprintf(output, "<header creationtool=\"Apertium TMX Builder\"\n");
  fprintf(output, "        creationtoolversion=\"%s\"\n", PACKAGE_VERSION);
  fprintf(output, "        segtype=\"sentence\"\n");
  fprintf(output, "        srclang=\"%s\"\n", UtfConverter::toUtf8(lang1).c_str());
  fprintf(output, "        adminlang=\"%s\"\n", UtfConverter::toUtf8(lang2).c_str());
  fprintf(output, "        datatype=\"plaintext\"\n");
  fprintf(output, "        o-tmf=\"none\">\n");
  fprintf(output, "</header>\n");
  fprintf(output, "<body>\n");
  outputTU(f1, f2, output);
  fprintf(output, "</body>\n</tmx>\n");

}

void
TMXBuilder::printTable(int *table, unsigned int nrows, unsigned int ncols)
{
  for(unsigned int i = 0; i < nrows; i++)
  {
    for(unsigned int j = 0; j < ncols; j++)
    {
      if(j != 0)
      {
        wcerr << L" ";
      }
      wcerr << setw(10) << table[i*ncols + j];
    }
    wcerr << endl;
  }
}


void
TMXBuilder::printTUCond(FILE *output, wstring const &tu1, wstring const &tu2, bool secure_zone)
{
  if(secure_zone && similar(tu1, tu2))
  {
    printTU(output, tu1, tu2);
  }  
}

void
TMXBuilder::splitAndMove(FILE *f1, string const &filename)
{
  FILE *stream = fopen(filename.c_str(), "w");
  vector<wstring> fichero_por_cadenas = sentenceList(f1);
  for(size_t i = 0; i < fichero_por_cadenas.size(); i++)
  {
    fputws_unlocked(fichero_por_cadenas[i].c_str(), stream);
    fputws_unlocked(L"\n", stream);
  }
  fclose(stream);
}

void
TMXBuilder::outputTU(FILE *f1, FILE *f2, FILE *output)
{
  string left = tmpnam(NULL);
  string right = tmpnam(NULL);
  string out = tmpnam(NULL);

  splitAndMove(f1, left);
  fclose(f1);

  splitAndMove(f2, right);
  fclose(f2);

  TMXAligner::DictionaryItems dict;
  AlignParameters ap;
  
  ap.justSentenceIds = false;
  ap.utfCharCountingMode = false;
  ap.realignType=AlignParameters::NoRealign;

  TMXAligner::alignerToolWithFilenames(dict, left, right, ap, out);

  FILE *stream = fopen(out.c_str(), "r");
  int conta = 0;
  wstring partes[2];
  while(true)
  {
    wchar_t val = fgetwc(stream);
    if(feof(stream))
    {
      break;
    }
  
    if(val == L'\t')
    {
      conta++;
    }
    else if(val == L'\n')
    {
      if(partes[0] != L"" && partes[1] != L"")
      {
        printTU(output, partes[0], partes[1]);
      }
      partes[0] = L"";
      partes[1] = L"";
      conta = 0;
    }
    if(conta < 2)
    {
      partes[conta] += val;
    }
  }
  
  unlink(left.c_str());
  unlink(right.c_str());
  unlink(out.c_str());

  /*


  int base_i = 0, base_j = 0;

  vector<wstring> lista1 = reverseList(sentenceList(f1)),
                  lista2 = reverseList(sentenceList(f2)), lista3;

  if(freference != NULL)
  {
    lista3 = reverseList(sentenceList(freference));
  } 

  while(true)
  { 
    vector<wstring> l1 = extractFragment(lista1, base_i, window_size);
    vector<wstring> l2 = extractFragment(lista2, base_j, window_size) , l3;

    if(lista3.size() != 0)
    {
      l3 = extractFragment(lista3, base_j, window_size);
    }

    int *table;
    if(lista3.size() == 0)
    {
      table = levenshteinTable(l1, l2, diagonal_width, max_edit);
    }
    else
    {
      table = levenshteinTable(l1, l3, diagonal_width, max_edit);
    }

    unsigned int const nrows = l1.size() + 1;
    unsigned int const ncols = l2.size() + 1;
    unsigned int i = nrows - 1;
    unsigned int j = ncols - 1;
  
  
    //    printTable(table, nrows, ncols);
  
    bool newBase = false;
  

    while(true)
    {
      int v = argmin(table[(i-1)*ncols + j-1], // i-1, j-1
                     table[(i-1)*ncols + j],  // i-j, j
                     table[i*ncols + j-1]); // i, j-1
      switch(v)
      {
        case 1:
          i--;
          j--;
	  
          if(l3.size() == 0)
	  {
            if((newBase || l1.size() < step) && similar(l1[i], l2[j]))
  	    {
  	      printTU(output, l1[i], l2[j]);
	    }
	  }
	  else
	  {
            if((newBase || l1.size() < step) && similar(l1[i], l3[j]))
  	    {
  	      printTU(output, l1[i], l2[j]);
	    }
	  }	    
          break;
      
        case 2: 
          i--;
          if(i > 2 && argmin(table[(i-1)*ncols + j-1],
			     table[(i-1)*ncols + j],  
			     table[i*ncols + j-1]) == 3 && 
	              argmin(table[(i-1)*ncols + j-2],
			     table[(i-1)*ncols + j-1],  
			     table[i*ncols + j-2]) != 1)
	    {
	      if(l3.size() == 0)
	      {
		if((newBase || l1.size() < step) && similar(l1[i], l2[j]))
		  {
		      printTU(output, l1[i], l2[j]);
		  }
		}
	      else
		{
		  if((newBase || l1.size() < step) && similar(l1[i], l3[j]))
		    {
		      printTU(output, l1[i], l2[j]);
		    }
		}	    
	    } 

	  //          wcerr << L"[" << i << L" " << j << L"]" << endl;
         break;
    
        case 3:
          j--;
          if(j > 2 && argmin(table[(i-1)*ncols + j-1],
			     table[(i-1)*ncols + j],  
			     table[i*ncols + j-1]) == 1 && 
	              argmin(table[(i-1)*ncols + j-2],
			     table[(i-1)*ncols + j-1],  
			     table[i*ncols + j-2]) != 3)
	    {
	      if(l3.size() == 0)
	      {
		if((newBase || l1.size() < step) && similar(l1[i], l2[j]))
		  {
		      printTU(output, l1[i], l2[j]);
		  }
		}
	      else
		{
		  if((newBase || l1.size() < step) && similar(l1[i], l3[j]))
		    {
		      printTU(output, l1[i], l2[j]);
		    }
		}	    
	    } 


          break;
    
        default:
          // error
          break;
      }
  
      if(i == step  && !newBase)
      {
         base_i += i;
         base_j += j;
         newBase = true;
      }
      
      if(i == 0 || j == 0)
      {
        break;
      }
    }
  
    delete[] table;  
    
    if(l1.size() < window_size)
    {
      break;
    }
    }*/
}

int 
TMXBuilder::weight(wstring const &s)
{
  return s.size()*2;  // just the size of the string
}

int * 
TMXBuilder::levenshteinTable(vector<wstring> &l1, vector<wstring> &l2, 
			     unsigned int diagonal_width, unsigned int max_edit)
{  
  unsigned int const nrows = l1.size() + 1;
  unsigned int const ncols = l2.size() + 1;
  
  int *table = new int[nrows * ncols];
  
  table[0] = 0;
  
  for(unsigned int i = 1; i < nrows; i++)
  {
    table[i*ncols] = table[(i-1)*ncols] + weight(l1[i-1]);
  }
  
  for(unsigned int j = 1; j < ncols; j++)
  {
    table[j] = table[j-1] + weight(l2[j-1]);
  }
  
  for(unsigned int i = 1; i < nrows; i++)
  {
    for(unsigned int j = 1; j < ncols; j++)
    {
      int ed = 0;
      
      if(i > (j + diagonal_width))
      {
        ed = table[i*ncols]+table[j];
      }
      else if(j > (i + diagonal_width))
      {
        ed = table[i*ncols]+table[j];
      }
      else
      {
        ed = editDistance(l1[i-1], l2[j-1], max_edit);
      }
      
      table[i*ncols+j] = min3(table[(i-1)*ncols + j-1] + ed,
                              table[(i-1)*ncols + j] + weight(l2[j-1]),
                              table[i*ncols + j-1] + weight(l1[i-1]));
    }
  }
  
  return table;
}

wstring
TMXBuilder::filter(wstring const &tu)
{
  bool has_text = false;  
  unsigned int count_blank = 0;

  for(unsigned int i = 0, limit = tu.size(); i != limit; i++)
  {
    if(iswalpha(tu[i]))
    {
      has_text = true;
    }      
    else if(has_text && iswspace(tu[i]))
    {
      count_blank++;
    }
  }  

  if(!has_text || count_blank <= 2 || tu.size() == 0)
  {
    return L"";
  }

  return xmlize(tu);  
}

void
TMXBuilder::printTU(FILE *output, wstring const &tu1, wstring const &tu2) const
{
  wstring tu1_filtered = filter(tu1);
  wstring tu2_filtered = filter(tu2);

  if(tu1_filtered != L"" && tu2_filtered != L"")
  {

    fprintf(output, "<tu>\n  <tuv xml:lang=\"%s\"><seg>%s</seg></tuv>\n", 
                    UtfConverter::toUtf8(lang1).c_str(), 
                    UtfConverter::toUtf8(tu1_filtered).c_str());
                  
    fprintf(output, "  <tuv xml:lang=\"%s\"><seg>%s</seg></tuv>\n</tu>\n",
                    UtfConverter::toUtf8(lang2).c_str(), 
                    UtfConverter::toUtf8(tu2_filtered).c_str());  
  }
} 

int
TMXBuilder::min3(int i1, int i2, int i3)
{
  if(i1 <= i2)
  {
    if(i1 <= i3)
    {
      return i1;
    }
    else
    {
      return i3;
    }
  }
  else if(i2 <= i3)
  {
    return i2;
  }
  else
  {
    return i3;
  }
}

int
TMXBuilder::min2(int i1, int i2)
{
  if(i1 <= i2)
  {
    return i1;
  }
  else
  {
    return i2;
  }
}

int
TMXBuilder::editDistance(wstring const &s1, wstring const &s2, unsigned int max_edit)
{
  int const nrows = min2(s1.size() + 1, max_edit);
  int const ncols = min2(s2.size() + 1, max_edit);
  
  int *table = new int[nrows*ncols];
 
  table[0] = 0;
  
  for(int i = 1; i < nrows; i++)
  {
    table[i*ncols] = i;
  }

  for(int j = 1; j < nrows; j++)
  {
    table[j] = j;
  }
    
  for(int i = 1; i < nrows; i++)
  {
    for(int j = 1; j < ncols; j++)
    {
      int coste = 0;
      if(s1[i-1] != s2[j-1])
      {
        coste = 1;
      }
      
      table[i*ncols+j] = min3(table[(i-1)*ncols+(j-1)]+coste,
                              table[(i-1)*ncols+j] + 2,
                              table[i*ncols+(j-1)] + 2);
    }
  }
  int result = table[(nrows*ncols)-1];
  delete[] table;
  return result;
}

void
TMXBuilder::setMaxEdit(int me)
{
  max_edit = me;
}

void
TMXBuilder::setDiagonalWidth(int dw)
{
  diagonal_width = dw;
}

void
TMXBuilder::setWindowSize(int ws)
{
  window_size = ws;
}

void
TMXBuilder::setStep(int s)
{
  step = s;
}

void
TMXBuilder::setPercent(double p)
{
  percent = p;
}

void
TMXBuilder::setLowLimit(int l)
{
  low_limit = l;
}

void
TMXBuilder::setEditDistancePercent(double e)
{
  edit_distance_percent = e;
}

bool
TMXBuilder::isRemovablePunct(wchar_t const &c)
{
  return c == L'.';
}

bool
TMXBuilder::similar(wstring const &s1, wstring const &s2)
{
  unsigned int l1 = s1.size();
  unsigned int l2 = s2.size(); 

  if((l1 <= low_limit) && (l2 <= low_limit))
  {
    return true;
  }
  else
  {
    int maxlength = max(l1, l2);
    int minlength = min(l1, l2);
    int ed = editDistance(s1, s2, maxlength);

    if(double(ed) < edit_distance_percent*double(maxlength))
    { 
      return double(minlength)/double(maxlength) > percent;
    }
    else
    {
      return false;
    }
  }
}

void
TMXBuilder::setTranslation(string const &filename)
{
  freference = fopen(filename.c_str(), "r");
  if(!freference)
  {
    wcerr << L"Error: file '" << UtfConverter::fromUtf8(filename);
    wcerr << L"' cannot be opened for reading" << endl;
    freference = NULL;
  }

#ifdef _MSC_VER
  if(freference != NULL)
  {
    _setmode(_fileno(freference), _O_U8TEXT);
  }
#endif     
}
