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
#ifndef _TRANSFERDATA_
#define _TRANSFERDATA_

#include <lttoolbox/alphabet.h>
#include <lttoolbox/ltstr.h>
#include <lttoolbox/transducer.h>

#include <map>
#include <set>

using namespace std;

class TransferData
{
private:
  void copy(TransferData const &o);
  void destroy();
  
  map<wstring, wstring, Ltstr> attr_items;
  map<wstring, int, Ltstr> macros;
  map<wstring, set<wstring, Ltstr>, Ltstr> lists;
  map<wstring, wstring, Ltstr> variables;
  
  Alphabet alphabet;
  Transducer transducer;
  map<int, int> finals;

  void writeRegexps(FILE *output);
 public:
  TransferData();
  ~TransferData();
  TransferData(TransferData const &o);
  TransferData & operator =(TransferData const &o);
  
  Alphabet & getAlphabet();
  Transducer & getTransducer();
  map<int, int> & getFinals();
  map<wstring, wstring, Ltstr> & getAttrItems();  

  map<wstring, int, Ltstr> & getMacros();
  map<wstring, set<wstring, Ltstr>, Ltstr> & getLists();
  map<wstring, wstring, Ltstr> & getVariables();
  
  void write(FILE *output);
};

#endif
