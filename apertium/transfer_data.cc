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

#include <apertium/transfer_data.h>
#include <lttoolbox/compression.h>
#include <apertium/utf_converter.h>
#include <apertium/apertium_re.h>
#include <pcre.h>
#include <iostream>
#include <apertium/string_utils.h>

using namespace Apertium;
using namespace std;

void
TransferData::copy(TransferData const &o)
{
  alphabet = o.alphabet;
  transducer = o.transducer;
  finals = o.finals;
  attr_items = o.attr_items;
  macros = o.macros;
  lists = o.lists;
  variables = o.variables;
}

void
TransferData::destroy()
{
}

TransferData::TransferData()
{
  // adding fixed attr_items
  attr_items[L"lem"] = L"(([^<]|\"\\<\")+)";
  attr_items[L"lemq"] = L"\\#[- _][^<]+"; 
  attr_items[L"lemh"] = L"(([^<#]|\"\\<\"|\"\\#\")+)";
  attr_items[L"whole"] = L"(.+)";
  attr_items[L"tags"] = L"((<[^>]+>)+)";
  attr_items[L"chname"] = L"({([^/]+)\\/)"; // includes delimiters { and / !!!
  attr_items[L"chcontent"] = L"(\\{.+)";    
  attr_items[L"content"] = L"(\\{.+)";
}

TransferData::~TransferData()
{
  destroy();
}

TransferData::TransferData(TransferData const &o)
{
  copy(o);
}

TransferData &
TransferData::operator =(TransferData const &o)
{
  if(this != &o)
  {
    destroy();
    copy(o);
  }
  return *this;
}

Alphabet &
TransferData::getAlphabet()
{
  return alphabet;
}

Transducer & 
TransferData::getTransducer()
{
  return transducer;
}

map<int, int> & 
TransferData::getFinals()
{
  return finals;
}

map<wstring, wstring, Ltstr> &
TransferData::getAttrItems()
{
  return attr_items;
}

map<wstring, int, Ltstr> &
TransferData::getMacros()
{
  return macros;
}

map<wstring, set<wstring, Ltstr>, Ltstr> &
TransferData::getLists()
{
  return lists;
}

map<wstring, wstring, Ltstr> &
TransferData::getVariables()
{
  return variables;
}

void 
TransferData::write(FILE *output)
{
  alphabet.write(output);
  transducer.write(output, alphabet.size());  

  // finals

  Compression::multibyte_write(finals.size(), output);  
  for(map<int, int>::const_iterator it = finals.begin(), limit = finals.end();
      it != limit; it++)
  {
    Compression::multibyte_write(it->first, output);
    Compression::multibyte_write(it->second, output);
  }

  // attr_items
  
  // precompiled regexps
  writeRegexps(output);

  // variables
  Compression::multibyte_write(variables.size(), output);
  for(map<wstring, wstring, Ltstr>::const_iterator it = variables.begin(), limit = variables.end();
      it != limit; it++)
  {
    Compression::wstring_write(it->first, output);
    Compression::wstring_write(it->second, output);
  }

  // macros
  Compression::multibyte_write(macros.size(), output);
  for(map<wstring, int, Ltstr>::const_iterator it = macros.begin(), limit = macros.end();
      it != limit; it++)
  {
    Compression::wstring_write(it->first, output);
    Compression::multibyte_write(it->second, output);
  }

  // lists
  Compression::multibyte_write(lists.size(), output);
  for(map<wstring, set<wstring, Ltstr>, Ltstr>::const_iterator it = lists.begin(), limit = lists.end();
      it != limit; it++)
  {
    Compression::wstring_write(it->first, output);
    Compression::multibyte_write(it->second.size(), output);
   
    for(set<wstring, Ltstr>::const_iterator it2 = it->second.begin(), limit2 = it->second.end();
	it2 != limit2; it2++)
    {
      Compression::wstring_write(*it2, output);
    }
  }

}

void
TransferData::writeRegexps(FILE *output)
{
  Compression::multibyte_write(attr_items.size(), output);
  
  map<wstring, wstring, Ltstr>::iterator it, limit;
  for(it = attr_items.begin(), limit = attr_items.end(); it != limit; it++)
  {
    Compression::wstring_write(it->first, output);
    ApertiumRE my_re;
    //wcerr << it->second << endl;
    my_re.compile(UtfConverter::toUtf8(it->second));
    my_re.write(output);
  }   
}
