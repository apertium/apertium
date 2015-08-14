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
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */
#ifndef _TRANSFERWORDLIST_
#define _TRANSFERWORDLIST_

#include <cstring>
#include <set>
#include <string>
#ifdef _MSC_VER
#define strcasecmp _stricmp
#endif

using namespace std;

struct ltstr
{
  bool operator()(string const &s1, string const &s2) const
  {
    return s1 < s2;
  }
};

struct ltstri
{
  bool operator()(string const &s1, string const &s2) const
  {
    return strcasecmp(s1.c_str(), s2.c_str()) < 0;
  }
};

class TransferWordList
{
private:
  set<string, ltstr> casefull_set;
  set<string, ltstri> caseless_set;

  void copy(TransferWordList const &o);
  void destroy();
public:
  TransferWordList();
  ~TransferWordList();
  TransferWordList(TransferWordList const &o);
  TransferWordList & operator =(TransferWordList const &o);

  bool search(string const &cad, bool caseless = false);
  void addWord(string const &cad);
};

#endif
