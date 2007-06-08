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

#include <apertium/interchunk_word.h>
#include <iostream>

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
  this->chunk = chunk;
}

string
InterchunkWord::chunkPart(string const &part)
{
  string result;
  pcrecpp::RE(part,
              pcrecpp::RE_Options().set_caseless(true)
                                   .set_extended(true)
                                   .set_utf8(true)).PartialMatch(chunk, &result);
  return result;
}

void
InterchunkWord::setChunkPart(string const &part, string const &value)
{
  pcrecpp::RE(part,
              pcrecpp::RE_Options().set_caseless(true)
                                   .set_extended(true)
                                   .set_utf8(true)).Replace(value, &chunk);
}
