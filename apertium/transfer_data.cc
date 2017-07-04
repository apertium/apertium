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

#include <apertium/transfer_data.h>
#include <lttoolbox/compression.h>
#include <apertium/utf_converter.h>
#include <apertium/apertium_re.h>
#include <iostream>
#include <apertium/string_utils.h>

using namespace Apertium;
using namespace std;

void
TransferData::copy(TransferData const &o)
{
  alphabet = o.alphabet;
  transducer = o.transducer;
  final_symbols = o.final_symbols;
  seen_rules = o.seen_rules;
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
  attr_items[L"lem"] = L"^(([^<]|\"\\<\")+)";
  attr_items[L"lemq"] = L"\\#[- _][^<]+";
  attr_items[L"lemh"] = L"^(([^<#]|\"\\<\"|\"\\#\")+)";
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

int
TransferData::countToFinalSymbol(const int count) {
  const wstring count_sym = L"<RULE_NUMBER:" + to_wstring(count) + L">";
  alphabet.includeSymbol(count_sym);
  const int symbol = alphabet(count_sym);
  final_symbols.insert(symbol);
  return symbol;
}

void
TransferData::write(FILE *output)
{
  alphabet.write(output);

  transducer.minimize();
  set<int> old_finals = transducer.getFinals(); // copy for later removal
  map<int, int> finals_rules;                   // node id -> rule number
  map<int, multimap<int, int> >& transitions = transducer.getTransitions();
  // Find all arcs with "final_symbols" in the transitions, let their source node instead be final,
  // and extract the rule number from the arc. Record relation between source node and rule number
  // in finals_rules. It is now no longer safe to minimize -- but we already did that.
  const wstring rule_sym_pre = L"<RULE_NUMBER:"; // see countToFinalSymbol()
  for(map<int, multimap<int, int> >::const_iterator it = transitions.begin(),
        limit = transitions.end(); it != limit; ++it)
  {
    const int src = it->first;
    for(multimap<int, int>::const_iterator arc = it->second.begin(),
          arclimit = it->second.end(); arc != arclimit; ++arc)
    {
      const int symbol = arc->first;
      const int trg = arc->second;
      if(final_symbols.count(symbol) == 0) {
        continue;
      }
      if(!transducer.isFinal(trg)) {
        continue;
      }
      // Extract the rule number encoded by countToFinalSymbol():
      wstring s;
      alphabet.getSymbol(s, symbol);
      if(s.compare(0, rule_sym_pre.size(), rule_sym_pre) != 0) {
        continue;
      }
      const int rule_num = stoi(s.substr(rule_sym_pre.size()));
      transducer.setFinal(src);
      finals_rules[src] = rule_num;
    }
  }
  // Remove the old finals:
  for(set<int>::const_iterator it = old_finals.begin(), limit = old_finals.end();
      it != limit; ++it)
  {
    transducer.setFinal(*it, false);
  }

  transducer.write(output, alphabet.size());

  // finals_rules

  Compression::multibyte_write(finals_rules.size(), output);
  for(map<int, int>::const_iterator it = finals_rules.begin(), limit = finals_rules.end();
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
  Compression::string_write(string(pcre_version()), output);
  Compression::multibyte_write(attr_items.size(), output);

  map<wstring, wstring, Ltstr>::iterator it, limit;
  for(it = attr_items.begin(), limit = attr_items.end(); it != limit; it++)
  {
    Compression::wstring_write(it->first, output);
    ApertiumRE my_re;
    my_re.compile(UtfConverter::toUtf8(it->second));
    my_re.write(output);
    Compression::wstring_write(it->second, output);
  }
}
