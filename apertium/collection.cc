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
#include <lttoolbox/compression.h>
#include <apertium/collection.h>
#include <apertium/string_utils.h>

using namespace Apertium;

int
Collection::size()
{
  return element.size();
}

bool 
Collection::has_not(const set<int> &t)
{
  return index.find(t) == index.end();
}

const set<int> &
Collection::operator[](int n)
{
  return *element[n];
}

int &
Collection::operator[](const set<int> &t)
{
  if(has_not(t))
  {
    index[t] = index.size()-1;
    element.push_back(&(index.find(t)->first));
  }
  return index[t];
}

int &
Collection::add(const set<int> &t)
{
  index[t] = index.size()-1;
  element.push_back(&(index.find(t)->first));
  return index[t];
}

void
Collection::write(FILE *output)
{
  Compression::multibyte_write(element.size(), output);

  for(int i = 0, limit = element.size(); i != limit; i++)
  {
    Compression::multibyte_write(element[i]->size(), output);
    for(set<int>::const_iterator it = element[i]->begin(), 
	  limit2 = element[i]->end(); it != limit2; it++)
    {
      Compression::multibyte_write(*it, output);
    }
  }
}

void
Collection::read(FILE *input)
{
  int size = Compression::multibyte_read(input);

  for(; size != 0; size--)
  {
    set<int> myset;
    int set_size = Compression::multibyte_read(input);
    for(; set_size != 0; set_size--)
    {
      myset.insert(Compression::multibyte_read(input));
    }
    add(myset);
  }
}
