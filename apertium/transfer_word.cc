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

#include <apertium/transfer_word.h>
#include <iostream>
#include <apertium/string_utils.h>

using namespace Apertium;
void
TransferWord::copy(TransferWord const &o)
{
  s_str = o.s_str;
  t_str = o.t_str;
  r_str = o.r_str;
  queue_length = o.queue_length;
}

void
TransferWord::destroy()
{
}

TransferWord::TransferWord() :
queue_length(0)
{
}

TransferWord::TransferWord(string const &src, string const &tgt, string const &ref, int queue)
{
  init(src, tgt, ref);
  queue_length = queue;
}

TransferWord::~TransferWord()
{
  destroy();
}

TransferWord::TransferWord(TransferWord const &o)
{
  copy(o);
}

TransferWord &
TransferWord::operator =(TransferWord const &o)
{
  if(this != &o)
  {
    destroy();
    copy(o);
  }
  return *this;
}

void
TransferWord::init(string const &src, string const &tgt, string const &ref)
{
  s_str = src;
  t_str = tgt;
  r_str = ref;
}

string
TransferWord::source(ApertiumRE const &part, bool with_queue)
{
  if(with_queue)
  {
    return part.match(s_str);
  }
  else
  {
    return part.match(s_str.substr(0, s_str.size() - queue_length));
  }
}

string
TransferWord::target(ApertiumRE const &part, bool with_queue)
{
  if(with_queue)
  {
    return part.match(t_str);
  }
  else
  {
    return part.match(t_str.substr(0, t_str.size() - queue_length));
  }
}

string
TransferWord::reference(ApertiumRE const &part, bool with_queue)
{
  if(with_queue)
  {
    return part.match(r_str);
  }
  else
  {
    return part.match(r_str.substr(0, r_str.size() - queue_length));
  }
}

bool
TransferWord::setSource(ApertiumRE const &part, string const &value,
			bool with_queue)
{
  if(with_queue)
  {
    return part.replace(s_str, value);
  }
  else
  {
    string mystring = s_str.substr(0, s_str.size() - queue_length);
    bool ret = part.replace(mystring, value);
    s_str = mystring + s_str.substr(s_str.size() - queue_length);
    return ret;
  }
}

bool
TransferWord::setTarget(ApertiumRE const &part, string const &value,
			bool with_queue)
{
  if(with_queue)
  {
    return part.replace(t_str, value);
  }
  else
  {
    string mystring = t_str.substr(0, t_str.size() - queue_length);
    bool ret = part.replace(mystring, value);
    t_str = mystring + t_str.substr(t_str.size() - queue_length);
    return ret;
  }
}

bool
TransferWord::setReference(ApertiumRE const &part, string const &value,
      bool with_queue)
{
  if(with_queue)
  {
    return part.replace(r_str, value);
  }
  else
  {
    string mystring = r_str.substr(0, r_str.size() - queue_length);
    bool ret = part.replace(mystring, value);
    r_str = mystring + r_str.substr(r_str.size() - queue_length);
    return ret;
  }
}
