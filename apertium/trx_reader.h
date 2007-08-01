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
#ifndef _TRXREADER_
#define _TRXREADER_

#include <apertium/transfer_data.h>
#include <lttoolbox/ltstr.h>

#include <libxml/xmlreader.h>
#include <map>
#include <string>

using namespace std;

class TRXReader
{
private:
  struct LemmaTags
  {
    wstring lemma;
    wstring tags;
  };

  xmlTextReaderPtr reader;  

  int type;
  wstring name;

  multimap<wstring, LemmaTags, Ltstr> cat_items;
  TransferData td;

  wstring attrib(wstring const &name);

  void parseError(wstring const &message);
  void copy(TRXReader const &o);
  void destroy();
  void clearTagIndex();
  
  void step();
  void procTransfer();
  void procDefCats();
  void procDefAttrs();
  void procDefVars();
  void procDefLists();
  void procDefMacros();
  void procRules();

  void insertCatItem(wstring const &name, wstring const &lemma, 
		     wstring const &tags);
  void insertAttrItem(wstring const &name, wstring const &tags);
  void createVar(wstring const &name, wstring const &initial_value);
  void insertListItem(wstring const &name, wstring const &value);
  void createMacro(wstring const &name, int const val);

  int insertLemma(int const base, wstring const &lemma);
  int insertTags(int const base, wstring const &tags);
  
public:
  static wstring const ANY_TAG;
  static wstring const ANY_CHAR;


  TRXReader();
  ~TRXReader();
  TRXReader(TRXReader const &o);
  TRXReader & operator =(TRXReader const &o);

  void read(string const &filename);
  void write(string const &filename);
};

#endif
