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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 */
#include <apertium/transfer_mult.h>
#include <apertium/trx_reader.h>
#include <lttoolbox/compression.h>
#include <lttoolbox/xml_parse_util.h>
#include <lttoolbox/string_utils.h>

#include <cctype>
#include <iostream>
#include <stack>

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
  Compression::string_read(in); // PCRE version placeholder
  for(int i = 0, limit = Compression::multibyte_read(in); i != limit; i++)
  {
    UString const cad_k = Compression::string_read(in);
    attr_items[cad_k].read(in);
    UString fallback = Compression::string_read(in);
    attr_items[cad_k].compile(fallback);
  }

  // variables
  for(int i = 0, limit = Compression::multibyte_read(in); i != limit; i++)
  {
    UString const cad_k = Compression::string_read(in);
    variables[cad_k] = Compression::string_read(in);
  }

  // macros
  for(int i = 0, limit = Compression::multibyte_read(in); i != limit; i++)
  {
    UString const cad_k = Compression::string_read(in);
    macros[cad_k] = Compression::multibyte_read(in);
  }

  // lists
  for(int i = 0, limit = Compression::multibyte_read(in); i != limit; i++)
  {
    UString const cad_k = Compression::string_read(in);

    for(int j = 0, limit2 = Compression::multibyte_read(in); j != limit2; j++)
    {
      UString const cad_v = Compression::string_read(in);
      lists[cad_k].insert(cad_v);
      listslow[cad_k].insert(StringUtils::tolower(cad_v));
    }
  }
}

void
TransferMult::readBil(string const &fstfile)
{
  FILE *in = fopen(fstfile.c_str(), "r");
  if(!in)
  {
    cerr << "Error: Could not open file '" << fstfile << "'." << endl;
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
    cerr << "Error: Could not open file '" << datafile << "'." << endl;
    exit(EXIT_FAILURE);
  }
  readData(in);
  fclose(in);

  readBil(fstfile);
}

TransferToken &
TransferMult::readToken(InputFile& in)
{
  if(!input_buffer.isEmpty())
  {
    return input_buffer.next();
  }

  UString content;
  while(true)
  {
    UChar32 val = in.get();
    if(in.eof())
    {
      return input_buffer.add(TransferToken(content, tt_eof));
    }
    if(val == '\\')
    {
      content += '\\';
      content += in.get();
    }
    else if(val == '[')
    {
      content += '[';
      while(true)
      {
        UChar32 val2 = in.get();
	if(val2 == '\\')
	{
	  content += '\\';
      content += in.get();
	}
	else if(val2 == ']')
	{
	  content += ']';
	  break;
	}
	else
	{
	  content += val2;
	}
      }
    }
    else if(val == '$')
    {
      return input_buffer.add(TransferToken(content, tt_word));
    }
    else if(val == '^')
    {
      return input_buffer.add(TransferToken(content, tt_blank));
    }
    else
    {
      content += val;
    }
  }
}

void
TransferMult::transfer(InputFile& in, UFILE* out)
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
	  pair<UString, int> tr = fstp.biltransWithQueue(*tmpword[0], false);
	  if(tr.first.size() != 0)
	  {
	    vector<UString> multiword = acceptions(tr.first);
	    if(multiword.size() > 1) {
          write("[{]"_u, output);
	    }
	    for(unsigned int i = 0, limit = multiword.size(); i != limit; i++)
	    {
	      if(i > 0)
	      {
	        write("[|]"_u, output);
	      }
          u_fprintf(output, "^%S$", multiword[i].c_str());
	    }
	    if(multiword.size() > 1)
	    {
	      write(".[][}]"_u, output);
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
	  write(*tmpblank[0], output);
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
	ms.step(' ');
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
	  write(current.getContent(), output);
	  return;
	}
	break;

      default:
	cerr << "Error: Unknown input token." << endl;
	return;
    }
  }
}

bool
TransferMult::isDefaultWord(UString const &str)
{
  return str.find(" D<"_u) != UString::npos;
}

vector<UString>
TransferMult::acceptions(UString str)
{
  vector<UString> result;
  int low = 0;

  // removing '@'
  if(str[0] == '@')
  {
    str = str.substr(1);
  }

  for(unsigned int i = 0, limit = str.size(); i != limit; i++)
  {
     if(str[i] == '\\')
     {
       i++;
     }
     else if(str[i] == '/')
     {
       UString new_word = str.substr(low, i-low);

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

  UString otherword = str.substr(low);
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
    vector<UString> result2;
    for(unsigned int i = 0, limit = result.size(); i != limit; i++)
    {
      if(result[i].find("__"_u) != UString::npos)
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
TransferMult::writeMultiple(list<vector<UString> >::iterator itwords,
                            list<UString>::iterator itblanks,
                            list<vector<UString> >::const_iterator limitwords,
                            UString acum , bool multiple)
{
  if(itwords == limitwords)
  {
    if(multiple)
    {
      output_string.append("[|]"_u);
    }
    output_string.append(acum);
  }
  else
  {
    vector<UString> &refword = *itwords;

    itwords++;

    if(itwords == limitwords)
    {
      for(unsigned int i = 0, limit = refword.size(); i != limit; i++)
      {
        UString temp = acum;
        temp += '^';
        temp += refword[i];
        temp += '$';
        writeMultiple(itwords, itblanks, limitwords, temp, multiple || (i > 0));
      }
    }
    else
    {
      UString &refblank = *itblanks;
      itblanks++;

      for(unsigned int i = 0, limit = refword.size(); i != limit; i++)
      {
        UString temp = acum;
        temp += '^';
        temp += refword[i];
        temp += '$';
        temp += refblank;
        writeMultiple(itwords, itblanks, limitwords,
                      temp,
                      multiple || (i > 0));
      }
    }
  }
}

void
TransferMult::applyRule()
{
  list<UString> blanks;
  list<vector<UString> > words;

  pair<UString, int> tr = fstp.biltransWithQueue(*tmpword[0], false);
  words.push_back(acceptions(tr.first));

  for(unsigned int i = 1; i != numwords; i++)
  {
    blanks.push_back(*tmpblank[i-1]);
    pair<UString, int> tr = fstp.biltransWithQueue(*tmpword[i], false);
    words.push_back(acceptions(tr.first));
  }

  output_string.clear();
  writeMultiple(words.begin(), blanks.begin(), words.end());

  if(output_string.find("[|]"_u) != UString::npos) {
    u_fprintf(output, "[{]%S.[][}]", output_string.c_str());
  } else {
    write(output_string, output);
  }

  ms.init(me->getInitial());

  tmpblank.clear();
  tmpword.clear();
  numwords = 0;
}

void
TransferMult::applyWord(UString const &word_str)
{
  ms.step('^');
  for(unsigned int i = 0, limit = word_str.size(); i < limit; i++)
  {
    switch(word_str[i])
    {
      case '\\':
        i++;
	ms.step(u_tolower(word_str[i]), any_char);
	break;

      case '<':
	for(unsigned int j = i+1; j != limit; j++)
	{
	  if(word_str[j] == '>')
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
	ms.step(u_tolower(word_str[i]), any_char);
	break;
    }
  }
  ms.step('$');
}
