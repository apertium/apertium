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
#ifndef _TRXREADER_
#define _TRXREADER_

#include <apertium/transfer_data.h>
#include <apertium/xml_reader.h>

#include <libxml/xmlreader.h>
#include <map>
#include <string>

using namespace std;

class TRXReader : public XMLReader
{
private:
  struct LemmaTags
  {
    UString lemma;
    UString tags;
  };

  multimap<UString, LemmaTags> cat_items;
  TransferData td;

  void destroy();
  void clearTagIndex();

  void procTransfer();
  void procDefCats();
  void procDefAttrs();
  void procDefVars();
  void procDefLists();
  void procDefMacros();
  void procRules();

  void checkClip();

  void insertCatItem(UString const &name, UString const &lemma,
		     UString const &tags);
  void createVar(UString const &name, UString const &initial_value);
  void insertListItem(UString const &name, UString const &value);
  void createMacro(UString const &name, int const val);

  int insertLemma(int const base, UString const &lemma);
  int insertTags(int const base, UString const &tags);

protected:
  virtual void parse();

public:
  static UString const ANY_TAG;
  static UString const ANY_CHAR;


  TRXReader();

  void write(string const &filename);
};

#endif
