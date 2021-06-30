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
#ifndef _TRANSFERDATA_
#define _TRANSFERDATA_

#include <lttoolbox/alphabet.h>
#include <lttoolbox/transducer.h>

#include <map>
#include <set>

using namespace std;

class TransferData
{
private:
  void copy(TransferData const &o);
  void destroy();

  map<UString, UString> attr_items;
  map<UString, int> macros;
  map<UString, set<UString>> lists;
  map<UString, UString> variables;
  set<int> final_symbols;

  Alphabet alphabet;
  Transducer transducer;

  void writeRegexps(FILE *output);
 public:
  TransferData();
  ~TransferData();
  TransferData(TransferData const &o);
  TransferData & operator =(TransferData const &o);

  Alphabet & getAlphabet();
  Transducer & getTransducer();
  map<UString, UString> & getAttrItems();

  map<int, int> seen_rules;

  map<UString, int> & getMacros();
  map<UString, set<UString>> & getLists();
  map<UString, UString> & getVariables();

  /**
   * Encode the rule count in an arc label/symbol (later extracted by
   * write()), recording that it's been used in final_symbols, and
   * return the resulting alphabet symbol.
   *
   * The symbol should be unique, since all other alphabet uses in
   * trx_reader are tags (which have *negative* ints as symbols).
   */
  int countToFinalSymbol(const int count);

  void write(FILE *output);
};

#endif
