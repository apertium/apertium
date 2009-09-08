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
/** 
 *  Word class and MorphoStream class definitions
 *
 *  @author	Felipe Sánchez-Martínez 
 */

#include <apertium/morpho_stream.h>
#include <apertium/constant_manager.h>
#include <vector>
#include <apertium/string_utils.h>
#include "apertium_config.h"
#include <apertium/unlocked_cstdio.h>

using namespace Apertium;
MorphoStream::MorphoStream(FILE *ftxt, bool d, TaggerData *t)
{
  foundEOF = false;
  debug=d;
  td = t;
  me = td->getPatternList().newMatchExe();
  alphabet = td->getPatternList().getAlphabet();
  input = ftxt;
  ca_any_char = alphabet(PatternList::ANY_CHAR);
  ca_any_tag = alphabet(PatternList::ANY_TAG);
  
  ConstantManager &constants = td->getConstants();
  ca_kignorar = constants.getConstant(L"kIGNORAR");
  ca_kbarra = constants.getConstant(L"kBARRA");
  ca_kdollar = constants.getConstant(L"kDOLLAR");
  ca_kbegin = constants.getConstant(L"kBEGIN");
  ca_kmot = constants.getConstant(L"kMOT");
  ca_kmas = constants.getConstant(L"kMAS");
  ca_kunknown = constants.getConstant(L"kUNKNOWN");
  
  map<wstring, int, Ltstr> &tag_index = td->getTagIndex();
  ca_tag_keof = tag_index[L"TAG_kEOF"];  
  ca_tag_kundef = tag_index[L"TAG_kUNDEF"]; 

  end_of_file = false;
  null_flush = false;
}

MorphoStream::~MorphoStream() 
{
  delete me;
}

TaggerWord *
MorphoStream::get_next_word()
{
  if(vwords.size() != 0)
  {
    TaggerWord* word=vwords.front();
    vwords.erase(vwords.begin());
    
    if(word->isAmbiguous())
    {
      vector<wstring> &ref = td->getDiscardRules();
      for(unsigned int i = 0; i < ref.size(); i++)
      {
        word->discardOnAmbiguity(ref[i]);
      }
    }
//    cout << *word << endl;
    return word;
  }

  if(feof(input))
  {
    return NULL;
  }
  
  int ivwords = 0;
  vwords.push_back(new TaggerWord());

  while(true)
  {
    int symbol = fgetwc_unlocked(input);
    if(feof(input) || (null_flush && symbol == L'\0'))
    {
      end_of_file = true;
      vwords[ivwords]->add_tag(ca_tag_keof, L"", td->getPreferRules());
      return get_next_word();
    }
    if(symbol == L'^')
    {
      readRestOfWord(ivwords);
      return get_next_word();
    }
    else
    {
      wstring str = L"";
      if(symbol == L'\\')
      {
        symbol = fgetwc_unlocked(input);
        str += L'\\';
        str += static_cast<wchar_t>(symbol);
        symbol = L'\\';
      }
      else
      {
        str += static_cast<wchar_t>(symbol);
      }
      
      while(symbol != L'^')
      {
	symbol = fgetwc_unlocked(input);
	if(feof(input) || (null_flush && symbol == L'\0'))
	{
	  end_of_file = true;
	  vwords[ivwords]->add_ignored_string(str);
          vwords[ivwords]->add_tag(ca_tag_keof, L"", td->getPreferRules());
	  return get_next_word();
	}
	else if(symbol == L'\\')
	{
	  str += L'\\';
          symbol = fgetwc_unlocked(input);
	  if(feof(input) || (null_flush && symbol == L'\0'))
	  {
	    end_of_file = true;
	    vwords[ivwords]->add_ignored_string(str);
            vwords[ivwords]->add_tag(ca_tag_keof, L"", td->getPreferRules());
	    return get_next_word();
	  }
	  str += static_cast<wchar_t>(symbol);
	  symbol = L'\\';
	}
	else if(symbol == L'^')
	{
	  if(str.size() > 0)
	  {
	    vwords[ivwords]->add_ignored_string(str);
          }
	  readRestOfWord(ivwords);
	  return get_next_word();
	}
        else
	{
	  str += static_cast<wchar_t>(symbol);
	}
      }
    }
  }
}

void
MorphoStream::lrlmClassify(wstring const &str, int &ivwords)
{
  int floor = 0;
  int last_type = -1;
  int last_pos = 0;

  ms.init(me->getInitial());
  for(int i = 0, limit = str.size(); i != limit; i++)
  {
    if(str[i] != L'<')
    {
      if(str[i] == L'+')
      {
        int val = ms.classifyFinals(me->getFinals());
        if(val != -1)
        {
          last_pos = i-1;
          last_type = val;
        }
      }
      ms.step(towlower(str[i]), ca_any_char);
    }
    else
    {
      wstring tag = L"";
      for(int j = i+1; j != limit; j++)
      {
        if(str[j] == L'\\')
        {
 	  j++;
        }
        else if(str[j] == L'>')
        {
 	  tag = str.substr(i, j-i+1);
	  i = j;
          break;
        }
      }

      int symbol = alphabet(tag);
      if(symbol)
      {
        ms.step(symbol, ca_any_tag);
      }
      else
      {
        ms.step(ca_any_tag);
      }
    }

    if(ms.size() == 0)
    {
      if(last_pos != floor)
      {
        vwords[ivwords]->add_tag(last_type, 
                                 str.substr(floor, last_pos - floor + 1),
                                 td->getPreferRules());
	if(str[last_pos+1] == L'+' && last_pos+1 < limit )
	{	
	  floor = last_pos + 1;
	  last_pos = floor;
          vwords[ivwords]->set_plus_cut(true); 
          if (((int)vwords.size())<=((int)(ivwords+1)))
            vwords.push_back(new TaggerWord(true));
          ivwords++;
	  ms.init(me->getInitial());
	}
	i = floor++;
      }
      else
      {
        if (debug)
        {
	  wcerr<<L"Warning: There is not coarse tag for the fine tag '"<< str.substr(floor) <<L"'\n";
          wcerr<<L"         This is because of an incomplete tagset definition or a dictionary error\n";
	}
        vwords[ivwords]->add_tag(ca_tag_kundef, str.substr(floor) , td->getPreferRules());
	return;
      }
    }
    else if(i == limit - 1)
    {
      if(ms.classifyFinals(me->getFinals()) == -1)
      {
	if(last_pos != floor)
	{
	  vwords[ivwords]->add_tag(last_type, 
                                   str.substr(floor, last_pos - floor + 1),
                                   td->getPreferRules());
          if(str[last_pos+1] == L'+' && last_pos+1 < limit )
          {	
            floor = last_pos + 1;
	    last_pos = floor;
            vwords[ivwords]->set_plus_cut(true); 
            if (((int)vwords.size())<=((int)(ivwords+1)))
              vwords.push_back(new TaggerWord(true));
            ivwords++;
            ms.init(me->getInitial());
	  }
	  i = floor++;
        }
        else
        {
          if (debug)
          {
	    wcerr<<L"Warning: There is not coarse tag for the fine tag '"<< str.substr(floor) <<L"'\n";
            wcerr<<L"         This is because of an incomplete tagset definition or a dictionary error\n";
	  }
          vwords[ivwords]->add_tag(ca_tag_kundef, str.substr(floor) , td->getPreferRules());
	  return;
        }
      }
    }
  }
  
  int val = ms.classifyFinals(me->getFinals());
  if(val == -1)
  {
    val = ca_tag_kundef;
    if (debug)
    {
      wcerr<<L"Warning: There is not coarse tag for the fine tag '"<< str.substr(floor) <<L"'\n";
      wcerr<<L"         This is because of an incomplete tagset definition or a dictionary error\n";
    }

  }    
  vwords[ivwords]->add_tag(val, str.substr(floor), td->getPreferRules());
}

void
MorphoStream::readRestOfWord(int &ivwords)
{
  // first we have the superficial form
  wstring  str = L"";
  
  while(true)
  {
    int symbol = fgetwc_unlocked(input);
    if(feof(input) || (null_flush && symbol == L'\0'))
    {
      end_of_file = true;
      if(str.size() > 0)
      {
        vwords[ivwords]->add_ignored_string(str);
        wcerr<<L"Warning (internal): kIGNORE was returned while reading a word\n";
        wcerr<<L"Word being read: "<<vwords[ivwords]->get_superficial_form()<<L"\n";
        wcerr<<L"Debug: "<< str <<L"\n";
      }
      vwords[ivwords]->add_tag(ca_tag_keof, L"", td->getPreferRules());
      return;
    }
    else if(symbol == L'\\')
    {
      symbol = fgetwc_unlocked(input);
      str += L'\\';
      str += static_cast<wchar_t>(symbol);
    }
    else if(symbol == L'/')
    {
      vwords[ivwords]->set_superficial_form(str); 
      str = L"";
      break;
    }
    else if(symbol == L'$')
    {
      vwords[ivwords]->set_superficial_form(str);
      vwords[ivwords]->add_ignored_string(L"$");
      break;
    }
    else
    {
      str += static_cast<wchar_t>(symbol);
    }
  }

  // then we read the acceptions

  while(true)
  {
    int symbol = fgetwc_unlocked(input);
    if(feof(input) || (null_flush && symbol == L'\0'))
    {
      end_of_file = true;
      if(str.size() > 0)
      {
        vwords[ivwords]->add_ignored_string(str);
        wcerr<<L"Warning (internal): kIGNORE was returned while reading a word\n";
        wcerr<<L"Word being read: "<<vwords[ivwords]->get_superficial_form()<<L"\n";
        wcerr<<L"Debug: "<< str <<L"\n";
      }
      vwords[ivwords]->add_tag(ca_tag_keof, L"", td->getPreferRules());
      return;
    }
    else if(symbol == L'\\')
    {
      symbol = fgetwc_unlocked(input);
      str += L'\\';
      str += static_cast<wchar_t>(symbol);
      symbol = L'\\';  // to prevent exiting with '\$'
    }
    else if(symbol == L'/')
    {
      lrlmClassify(str, ivwords);
      str = L"";
      ivwords = 0;
      continue;
    }
    else if(symbol == L'$')
    {
      if(str[0] != L'*')// do nothing with unknown words 
      {
	lrlmClassify(str, ivwords);
      }
      return;
    }
    else
    {
      str += static_cast<wchar_t>(symbol);
    }    
  }
}

void
MorphoStream::setNullFlush(bool nf)
{
  null_flush = nf;
}

bool
MorphoStream::getEndOfFile(void)
{
  return end_of_file;
}

void
MorphoStream::setEndOfFile(bool eof)
{
  end_of_file = eof;
}
