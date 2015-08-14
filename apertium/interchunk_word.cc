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

#include <apertium/interchunk_word.h>
#include <iostream>
#include <apertium/string_utils.h>

using namespace Apertium;

void
InterchunkWord::copy(InterchunkWord const &o)
{
  this->chunk = o.chunk;
}

void
InterchunkWord::destroy()
{
}

InterchunkWord::InterchunkWord()
{
}

InterchunkWord::InterchunkWord(string const &chunk)
{
  init(chunk);
}

InterchunkWord::~InterchunkWord()
{
  destroy();
}

InterchunkWord::InterchunkWord(InterchunkWord const &o)
{
  copy(o);
}

InterchunkWord &
InterchunkWord::operator =(InterchunkWord const &o)
{
  if(this != &o)
  {
    destroy();
    copy(o);
  }
  return *this;
}

void
InterchunkWord::init(string const &chunk)
{
  for(size_t i = 0; i < chunk.size(); i++)
  {
    if(chunk[i] == '\\')
    {
      i++;
    }
    else if(chunk[i] == '{')
    {
      this->chunk = chunk.substr(0, i);
      this->queue = chunk.substr(i);
      return;
    }
  }
  this->chunk = chunk;
  this->queue = "";
}

string
InterchunkWord::chunkPart(ApertiumRE const &part)
{
  string result = part.match(chunk);
  if(result.size() == 0)
  {
    result = part.match(queue);
    if(result.size() != queue.size())
    {
      return "";
    }
    else
    {
      return result;
    }
  }
  else if(result.size() == chunk.size())
  {
    return part.match(chunk+queue);
  }
  else
  {
    return result;
  }
}

void
InterchunkWord::setChunkPart(ApertiumRE const &part, string const &value)
{
  part.replace(chunk, value);
}
