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
#include <apertium/transfer_word_list.h>
#include <apertium/string_utils.h>

using namespace Apertium;
void
TransferWordList::copy(TransferWordList const &o)
{
  casefull_set = o.casefull_set;
  caseless_set = o.caseless_set;
}

void
TransferWordList::destroy()
{
}

TransferWordList::TransferWordList()
{
}

TransferWordList::~TransferWordList()
{
  destroy();
}

TransferWordList::TransferWordList(TransferWordList const &o)
{
  copy(o);
}

TransferWordList &
TransferWordList::operator =(TransferWordList const &o)
{
  if(this != &o)
  {
    destroy();
    copy(o);
  }
  return *this;
}

bool
TransferWordList::search(string const &cad, bool caseless)
{
  if(caseless)
  {
    return caseless_set.find(cad) != caseless_set.end();
  }
  else
  {
    return casefull_set.find(cad) != casefull_set.end();
  }
}

void
TransferWordList::addWord(string const &cad)
{
  casefull_set.insert(cad);
  caseless_set.insert(cad);
}
