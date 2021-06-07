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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
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
#include "apertium_config.h"
#include <apertium/unlocked_cstdio.h>

#ifdef _MSC_VER
#include <io.h>
#include <fcntl.h>
#endif

using namespace Apertium;
using namespace std;

TMXBuilder::TMXBuilder(UString const &l1, UString const &l2):
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

UString
TMXBuilder::restOfBlank(InputFile& input)
{
  UString result = "["_u;

  while(true)
  {
    UChar32 val = input.get();
    if(input.eof())
    {
      return ""_u;
    }
    switch(val)
    {
      case '\\':
        result += '\\';
        val = input.get();
        if(input.eof())
        {
          return ""_u;
        }
        result += static_cast<wchar_t>(val);
        break;

      case ']':
        result += ']';
        return result;

      default:
        result += static_cast<wchar_t>(val);
        break;
    }
  }

  return ""_u;
}

UString
TMXBuilder::nextBlank(InputFile& input)
{
  UString result;

  while(true)
  {
    UChar32 val = input.get();
    if(input.eof()) {
      return ""_u;
    }
    switch(val)
    {
      case '\\':
        input.get();
        break;
      case '[':

        result = restOfBlank(input);
        return result;
    }
  }
}

bool
TMXBuilder::compatible(InputFile& f1, InputFile& f2, bool lazy)
{
  UString s1 = nextBlank(f1), s2 = nextBlank(f2);
  if(!lazy)
  {
    while(!f1.eof() && !f2.eof())
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
    while(!f1.eof() && !f2.eof())
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
  InputFile f1, f2;
  bool bf1 = f1.open(file1.c_str());
  bool bf2 = f2.open(file2.c_str());
  if(!bf1 && !bf2)
  {
    cerr << "Error: Cannot access files '" << file1;
    cerr << "' and '" << file2 << "'" << endl;
    return false;
  }
  else if(!bf1)
  {
    cerr << "Error: Cannot access file '" << file1 << "'" << endl;
    return false;
  }
  else if(!bf2)
  {
    cerr << "Error: Cannot access file '" << file2 << "'" << endl;
    return false;
  }

  bool retval = compatible(f1, f2, lazy);

  return retval;
}

UString
TMXBuilder::nextTU(InputFile& input)
{
  UString current_tu;
  UString tmp;

  while(true)
  {
    UChar32 symbol = input.get();
    if(input.eof()) {
      return current_tu;
    }
    switch(symbol)
    {
      case '\\':
        symbol = input.get();
        if(input.eof()) {
          return current_tu;
        }
        // continued down
      default:
        current_tu += symbol;
        break;

      case '[':
        tmp = restOfBlank(input);
        if(tmp.substr(0,2) == "[ "_u)
        {
          current_tu += ' ';
        }
        current_tu.append("<ph/>"_u);
        if(tmp.substr(tmp.size()-2, 2) == " ]"_u)
        {
          current_tu += ' ';
        }
        break;

      case '.':
        current_tu += '.';
        symbol = input.get();

        if(symbol != '[' && !iswspace(symbol))
        {
          if (!input.eof()) {
            input.unget(symbol);
          }
        }
        else
        {
          if (!input.eof()) {
            input.unget(symbol);
          }

          return current_tu;
/*          size_t idx = current_tu.size()-1;
          while(current_tu[idx] == '.')
          {
            idx--;
          }
          return current_tu.substr(0, idx+1);*/
        }
        break;

      case '?':
      case '!':
        current_tu += static_cast<wchar_t>(symbol);
        return current_tu;
    }
  }

  return current_tu;
}

UString
TMXBuilder::xmlize(UString const &str)
{
  UString result;

  for(size_t i = 0, limit = str.size(); i < limit; i++)
  {
    switch(str[i])
    {
      case '<':
        if(i + 5 <= limit && str.substr(i,5)=="<ph/>"_u)
        {
          result.append("<ph/>"_u);
          i += 4;
          break;
        }
        else
        {
          result.append("&lt;"_u);
        }
        break;

      case '>':
        result.append("&gt;"_u);
        break;

      case '&':
        result.append("&amp;"_u);
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
    while(result.size() >= 5 && result.substr(0,5) == "<ph/>"_u)
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
    while(result.size() > 5 && result.substr(result.size()-5) == "<ph/>"_u)
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
  UFILE* output = u_finit(stdout, NULL, NULL);

  if(!outfile.empty())
  {
    output = u_fopen(outfile.c_str(), "w", NULL, NULL);
    if(!output)
    {
      cerr << "Error: file '" << outfile;
      cerr << "' cannot be opened for writing" << endl;
      exit(EXIT_FAILURE);
    }
  }

  InputFile f1;
  f1.open_or_exit(file1.c_str());

  InputFile f2;
  f2.open_or_exit(file2.c_str());

  generateTMX(f1, f2, output);
}

vector<UString>
TMXBuilder::reverseList(vector<UString> const &v)
{
  vector<UString> retval(v.size());

  for(int j = v.size() - 1, i = 0; j >=0; j--, i++)
  {
    retval[i] = v[j];
  }

  return retval;
}

vector<UString>
TMXBuilder::sentenceList(InputFile& file)
{
  vector<UString> retval;

  while(true)
  {
    UString f = nextTU(file);
    if(file.eof()) {
      break;
    }
    retval.push_back(f);
  }

  return retval;
}

vector<UString>
TMXBuilder::extractFragment(vector<UString> const &text, unsigned int base, unsigned int width)
{
  vector<UString> result;

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
TMXBuilder::generateTMX(InputFile& f1, InputFile& f2, UFILE* output)
{
  u_fprintf(output, "<?xml version=\"1.0\"?>\n");
  u_fprintf(output, "<tmx version=\"1.4\">\n");
  u_fprintf(output, "<header creationtool=\"Apertium TMX Builder\"\n");
  u_fprintf(output, "        creationtoolversion=\"%s\"\n", PACKAGE_VERSION);
  u_fprintf(output, "        segtype=\"sentence\"\n");
  u_fprintf(output, "        srclang=\"%S\"\n", lang1.c_str());
  u_fprintf(output, "        adminlang=\"%S\"\n", lang2.c_str());
  u_fprintf(output, "        datatype=\"plaintext\"\n");
  u_fprintf(output, "        o-tmf=\"none\">\n");
  u_fprintf(output, "</header>\n");
  u_fprintf(output, "<body>\n");
  outputTU(f1, f2, output);
  u_fprintf(output, "</body>\n</tmx>\n");

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
        cerr << " ";
      }
      cerr << setw(10) << table[i*ncols + j];
    }
    cerr << endl;
  }
}


void
TMXBuilder::printTUCond(UFILE *output, UString const &tu1, UString const &tu2, bool secure_zone)
{
  if(secure_zone && similar(tu1, tu2))
  {
    printTU(output, tu1, tu2);
  }
}

void
TMXBuilder::splitAndMove(InputFile& f1, string const &filename)
{
  UFILE* stream = u_fopen(filename.c_str(), "w", NULL, NULL);
  vector<UString> fichero_por_cadenas = sentenceList(f1);
  for (auto& it : fichero_por_cadenas) {
    u_fprintf(stream, "%S\n", it.c_str());
  }
  u_fclose(stream);
}

void
TMXBuilder::outputTU(InputFile& f1, InputFile& f2, UFILE* output)
{
  string left = tmpnam(NULL);
  string right = tmpnam(NULL);
  string out = tmpnam(NULL);

  splitAndMove(f1, left);

  splitAndMove(f2, right);

  TMXAligner::DictionaryItems dict;
  AlignParameters ap;

  ap.justSentenceIds = false;
  ap.utfCharCountingMode = false;
  ap.realignType=AlignParameters::NoRealign;

  TMXAligner::alignerToolWithFilenames(dict, left, right, ap, out);

  InputFile stream;
  stream.open(out.c_str());
  int conta = 0;
  UString partes[2];
  while(!stream.eof())
  {
    UChar32 val = stream.get();

    if(val == '\t')
    {
      conta++;
    }
    else if(val == '\n')
    {
      if (!partes[0].empty() && !partes[1].empty()) {
        printTU(output, partes[0], partes[1]);
      }
      partes[0].clear();
      partes[1].clear();
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

  vector<UString> lista1 = reverseList(sentenceList(f1)),
                  lista2 = reverseList(sentenceList(f2)), lista3;

  if(freference != NULL)
  {
    lista3 = reverseList(sentenceList(freference));
  }

  while(true)
  {
    vector<UString> l1 = extractFragment(lista1, base_i, window_size);
    vector<UString> l2 = extractFragment(lista2, base_j, window_size) , l3;

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

	  //          cerr << "[" << i << " " << j << "]" << endl;
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
TMXBuilder::weight(UString const &s)
{
  return s.size()*2;  // just the size of the string
}

int *
TMXBuilder::levenshteinTable(vector<UString> &l1, vector<UString> &l2,
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

UString
TMXBuilder::filter(UString const &tu)
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
    return ""_u;
  }

  return xmlize(tu);
}

void
TMXBuilder::printTU(UFILE* output, UString const &tu1, UString const &tu2) const
{
  UString tu1_filtered = filter(tu1);
  UString tu2_filtered = filter(tu2);

  if (tu1_filtered.empty() && !tu2_filtered.empty()) {
    u_fprintf(output, "<tu>\n  <tuv xml:lang=\"%S\"><seg>%S</seg></tuv>\n",
              lang1.c_str(), tu1_filtered.c_str());

    u_fprintf(output, "  <tuv xml:lang=\"%S\"><seg>%S</seg></tuv>\n</tu>\n",
              lang2.c_str(), tu2_filtered.c_str());
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
TMXBuilder::editDistance(UString const &s1, UString const &s2, unsigned int max_edit)
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
  return c == '.';
}

bool
TMXBuilder::similar(UString const &s1, UString const &s2)
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
    cerr << "Error: file '" << filename;
    cerr << "' cannot be opened for reading" << endl;
    freference = NULL;
  }

#ifdef _MSC_VER
  if(freference != NULL)
  {
    _setmode(_fileno(freference), _O_U8TEXT);
  }
#endif
}
