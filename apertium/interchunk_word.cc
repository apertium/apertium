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

InterchunkWord::InterchunkWord(UString const &chunk)
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
InterchunkWord::init(UString const &chunk)
{
  size_t b_end = 0;
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
    else if(chunk[i] == ']')
    {
      if(chunk[i-1] == ']')
      {
        b_end = i+1;
      }
    }
  }
  
  if(b_end > 0)
  {
    this->wblank = chunk.substr(0, b_end);
    this->chunk = chunk.substr(b_end);
  }
  else
  {
    this->chunk = chunk;
  }
  this->queue.clear();
}

UString
InterchunkWord::chunkPart(ApertiumRE const &part)
{
  UString result = part.match(chunk);
  if(result.size() == 0)
  {
    result = part.match(queue);
    if(result.size() != queue.size())
    {
      return ""_u;
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

UString
InterchunkWord::getWblank()
{
  return wblank;
}

bool
InterchunkWord::setChunkPart(ApertiumRE const &part, UString const &value)
{
  return part.replace(chunk, value);
}
