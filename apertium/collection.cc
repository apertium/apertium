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
#include <lttoolbox/compression.h>
#include <apertium/collection.h>
#include <apertium/string_utils.h>
#include <apertium/serialiser.h>
#include <apertium/deserialiser.h>

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
  return (*this)[t];
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

void
Collection::serialise(std::ostream &serialised) const
{
  Serialiser<size_t>::serialise(element.size(), serialised);
  for (size_t i = 0; i < element.size(); i++) {
    Serialiser<set<int> >::serialise(*element[i], serialised);
  }
}

void
Collection::deserialise(std::istream &serialised)
{
  size_t size = Deserialiser<size_t>::deserialise(serialised);
  for (size_t i = 0; i < size; i++) {
    set<int> deserialised_set =
      Deserialiser<set<int> >::deserialise(serialised);
    add(deserialised_set);
  }
}
