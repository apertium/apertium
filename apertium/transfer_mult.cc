/*
 * Copyright (C) 2005--2015 Universitat d'Alacant / Universidad de Alicante
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
#include <apertium/transfer_mult.h>
#include <apertium/trx_reader.h>
#include <lttoolbox/compression.h>
#include <lttoolbox/xml_parse_util.h>
#include <apertium/utf_converter.h>
#include <apertium/string_utils.h>

#include <cctype>
#include <iostream>
#include <stack>

#ifdef _WIN32
#include <utf8_fwrap.h>
#endif

using namespace std;

void
TransferMult::destroy()
{
  if(me)
  {
    delete me;
    me = NULL;
  }
}

TransferMult::TransferMult() :
word(0),
blank(0),
output(0),
any_char(0),
any_tag(0),
nwords(0)
{
  me = NULL;
  isRule = false;
  defaultAttrs = lu;
  numwords = 0;
}

TransferMult::~TransferMult()
{
  destroy();
}

string
TransferMult::tolower(string const &str) const
{
  string result = str;
  for(unsigned int i = 0, limit = str.size(); i != limit; i++)
  {
    result[i] = ::tolower(result[i]);
  }

  return result;
}

void
TransferMult::readData(FILE *in)
{
  alphabet.read(in);
  any_char = alphabet(TRXReader::ANY_CHAR);
  any_tag = alphabet(TRXReader::ANY_TAG);

  Transducer t;
  t.read(in, alphabet.size());

  map<int, int> finals;

  // finals
  for(int i = 0, limit = Compression::multibyte_read(in); i != limit; i++)
  {
    int key = Compression::multibyte_read(in);
    finals[key] = Compression::multibyte_read(in);
  }

  me = new MatchExe(t, finals);

  // attr_items
  bool recompile_attrs = Compression::string_read(in) != string(pcre_version());
  for(int i = 0, limit = Compression::multibyte_read(in); i != limit; i++)
  {
    string const cad_k = UtfConverter::toUtf8(Compression::wstring_read(in));
    attr_items[cad_k].read(in);
    wstring fallback = Compression::wstring_read(in);
    if(recompile_attrs) {
      attr_items[cad_k].compile(UtfConverter::toUtf8(fallback));
    }
  }

  // variables
  for(int i = 0, limit = Compression::multibyte_read(in); i != limit; i++)
  {
    string const cad_k = UtfConverter::toUtf8(Compression::wstring_read(in));
    variables[cad_k] = UtfConverter::toUtf8(Compression::wstring_read(in));
  }

  // macros
  for(int i = 0, limit = Compression::multibyte_read(in); i != limit; i++)
  {
    string const cad_k = UtfConverter::toUtf8(Compression::wstring_read(in));
    macros[cad_k] = Compression::multibyte_read(in);
  }

  // lists
  for(int i = 0, limit = Compression::multibyte_read(in); i != limit; i++)
  {
    string const cad_k = UtfConverter::toUtf8(Compression::wstring_read(in));

    for(int j = 0, limit2 = Compression::multibyte_read(in); j != limit2; j++)
    {
      wstring const cad_v = Compression::wstring_read(in);
      lists[cad_k].insert(UtfConverter::toUtf8(cad_v));
      listslow[cad_k].insert(UtfConverter::toUtf8(StringUtils::tolower(cad_v)));
    }
  }
}

void
TransferMult::readBil(string const &fstfile)
{
  FILE *in = fopen(fstfile.c_str(), "r");
  if(!in)
  {
    wcerr << "Error: Could not open file '" << fstfile << "'." << endl;
    exit(EXIT_FAILURE);
  }
  fstp.load(in);
  fstp.initBiltrans();
  fclose(in);
}

void
TransferMult::read(string const &datafile, string const &fstfile)
{
  // datafile
  FILE *in = fopen(datafile.c_str(), "r");
  if(!in)
  {
    wcerr << "Error: Could not open file '" << datafile << "'." << endl;
    exit(EXIT_FAILURE);
  }
  readData(in);
  fclose(in);

  readBil(fstfile);
}

TransferToken &
TransferMult::readToken(FILE *in)
{
  if(!input_buffer.isEmpty())
  {
    return input_buffer.next();
  }

  wstring content = L"";
  while(true)
  {
    int val = fgetwc_unlocked(in);
    if(feof(in))
    {
      return input_buffer.add(TransferToken(content, tt_eof));
    }
    if(val == L'\\')
    {
      content += L'\\';
      content += wchar_t(fgetwc_unlocked(in));
    }
    else if(val == L'[')
    {
      content += L'[';
      while(true)
      {
	int val2 = fgetwc_unlocked(in);
	if(val2 == L'\\')
	{
	  content += L'\\';
	  content += wchar_t(fgetwc_unlocked(in));
	}
	else if(val2 == L']')
	{
	  content += L']';
	  break;
	}
	else
	{
	  content += wchar_t(val2);
	}
      }
    }
    else if(val == L'$')
    {
      return input_buffer.add(TransferToken(content, tt_word));
    }
    else if(val == L'^')
    {
      return input_buffer.add(TransferToken(content, tt_blank));
    }
    else
    {
      content += wchar_t(val);
    }
  }
}

void
TransferMult::transfer(FILE *in, FILE *out)
{
  int last = 0;

  output = out;
  ms.init(me->getInitial());

  while(true)
  {
    if(ms.size() == 0)
    {
      if(isRule)
      {
	applyRule();
	isRule = false;
	input_buffer.setPos(last);
      }
      else
      {
	if(tmpword.size() != 0)
	{
	  pair<wstring, int> tr = fstp.biltransWithQueue(*tmpword[0], false);
	  if(tr.first.size() != 0)
	  {
	    vector<wstring> multiword = acceptions(tr.first);
	    if(multiword.size() > 1)
	    {
	      fputws_unlocked(L"[{]", output);
	    }
	    for(unsigned int i = 0, limit = multiword.size(); i != limit; i++)
	    {
	      if(i > 0)
	      {
	        fputws_unlocked(L"[|]", output);
	      }
	      fputwc_unlocked(L'^', output);
	      fputws_unlocked(multiword[i].c_str(), output);
	      fputwc_unlocked(L'$', output);
	    }
	    if(multiword.size() > 1)
	    {
	      fputws_unlocked(L".[][}]", output);
            }
	  }
	  tmpword.clear();
	  isRule = false;
	  input_buffer.setPos(last);
	  input_buffer.next();
	  last = input_buffer.getPos();
	  ms.init(me->getInitial());
	}
	else if(tmpblank.size() != 0)
	{
	  fputws_unlocked(tmpblank[0]->c_str(), output);
	  tmpblank.clear();
	  last = input_buffer.getPos();
	  ms.init(me->getInitial());
	}
      }
    }
    int val = ms.classifyFinals(me->getFinals());
    if(val != -1)
    {
      isRule = true;
      numwords = tmpword.size();
      last = input_buffer.getPos();
    }

    TransferToken &current = readToken(in);

    switch(current.getType())
    {
      case tt_word:
	applyWord(current.getContent());
        tmpword.push_back(&current.getContent());
	break;

      case tt_blank:
	ms.step(L' ');
	tmpblank.push_back(&current.getContent());
	break;

      case tt_eof:
	if(tmpword.size() != 0)
	{
	  tmpblank.push_back(&current.getContent());
	  ms.clear();
	}
	else
	{
	  fputws_unlocked(current.getContent().c_str(), output);
	  return;
	}
	break;

      default:
	wcerr << L"Error: Unknown input token." << endl;
	return;
    }
  }
}

bool
TransferMult::isDefaultWord(wstring const &str)
{
  return str.find(L" D<");
}

vector<wstring>
TransferMult::acceptions(wstring str)
{
  vector<wstring> result;
  int low = 0;

  // removing '@'
  if(str[0] == L'@')
  {
    str = str.substr(1);
  }

  for(unsigned int i = 0, limit = str.size(); i != limit; i++)
  {
     if(str[i] == L'\\')
     {
       i++;
     }
     else if(str[i] == L'/')
     {
       wstring new_word = str.substr(low, i-low);

       if(result.size() > 1 && isDefaultWord(new_word))
       {
	 result.push_back(result[0]);
	 result[0] = new_word;
       }
       else
       {
         result.push_back(new_word);
       }
       low = i + 1;
     }
  }

  wstring otherword = str.substr(low);
  if(result.size() > 0 && isDefaultWord(otherword))
  {
    result.push_back(result[0]);
    result[0] = otherword;
  }
  else
  {
    result.push_back(otherword);
  }

  // eliminar las acepciones sin sentido marcado
  if(result.size() >= 2)
  {
    vector<wstring> result2;
    for(unsigned int i = 0, limit = result.size(); i != limit; i++)
    {
      if(result[i].find(L"__") != wstring::npos)
      {
        result2.push_back(result[i]);
      }
    }
    if(result2.size() >= 2)
    {
      return result2;
    }
  }

  return result;
}

void
TransferMult::writeMultiple(list<vector<wstring> >::iterator itwords,
                            list<wstring>::iterator itblanks,
                            list<vector<wstring> >::const_iterator limitwords,
                            wstring acum , bool multiple)
{
  if(itwords == limitwords)
  {
    if(multiple)
    {
      output_string.append(L"[|]");
    }
    output_string.append(acum);
  }
  else
  {
    vector<wstring> &refword = *itwords;

    itwords++;

    if(itwords == limitwords)
    {
      for(unsigned int i = 0, limit = refword.size(); i != limit; i++)
      {
        writeMultiple(itwords, itblanks, limitwords,
                      acum + L"^" + refword[i] + L"$", multiple || (i > 0));
      }
    }
    else
    {
      wstring &refblank = *itblanks;
      itblanks++;

      for(unsigned int i = 0, limit = refword.size(); i != limit; i++)
      {
        writeMultiple(itwords, itblanks, limitwords,
                      acum + L"^" + refword[i] + L"$" + refblank,
                      multiple || (i > 0));
      }
    }
  }
}

void
TransferMult::applyRule()
{
  list<wstring> blanks;
  list<vector<wstring> > words;

  pair<wstring, int> tr = fstp.biltransWithQueue(*tmpword[0], false);
  words.push_back(acceptions(tr.first));

  for(unsigned int i = 1; i != numwords; i++)
  {
    blanks.push_back(*tmpblank[i-1]);
    pair<wstring, int> tr = fstp.biltransWithQueue(*tmpword[i], false);
    words.push_back(acceptions(tr.first));
  }

  output_string = L"";
  writeMultiple(words.begin(), blanks.begin(), words.end());

  if(output_string.find(L"[|]") != wstring::npos)
  {
    fputws_unlocked(L"[{]", output);
    fputws_unlocked(output_string.c_str(), output);
    fputws_unlocked(L".[][}]", output);
  }
  else
  {
    fputws_unlocked(output_string.c_str(), output);
  }

  ms.init(me->getInitial());

  tmpblank.clear();
  tmpword.clear();
  numwords = 0;
}

void
TransferMult::applyWord(wstring const &word_str)
{
  ms.step(L'^');
  for(unsigned int i = 0, limit = word_str.size(); i < limit; i++)
  {
    switch(word_str[i])
    {
      case L'\\':
        i++;
	ms.step(towlower(word_str[i]), any_char);
	break;

      case L'<':
	for(unsigned int j = i+1; j != limit; j++)
	{
	  if(word_str[j] == L'>')
	  {
	    int symbol = alphabet(word_str.substr(i, j-i+1));
	    if(symbol)
	    {
	      ms.step(symbol, any_tag);
	    }
	    else
	    {
	      ms.step(any_tag);
	    }
	    i = j;
	    break;
	  }
	}
	break;

      default:
	ms.step(towlower(word_str[i]), any_char);
	break;
    }
  }
  ms.step(L'$');
}
