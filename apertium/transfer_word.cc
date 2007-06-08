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

#include <apertium/transfer_word.h>
#include <iostream>

void
TransferWord::copy(TransferWord const &o)
{
  s_str = o.s_str;
  t_str = o.t_str;
  queue_length = o.queue_length;
}

void
TransferWord::destroy()
{
}

string
TransferWord::access(string const &str, string const &part)
{
  string result;
  pcrecpp::RE(part, 
              pcrecpp::RE_Options().set_caseless(true)
                                   .set_extended(true)
                                   .set_utf8(true)).PartialMatch(str, &result);
  return result;
}

TransferWord::TransferWord()
{
}

TransferWord::TransferWord(string const &src, string const &tgt, int queue)
{
  init(src, tgt);
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
TransferWord::init(string const &src, string const &tgt)
{
  s_str = src;
  t_str = tgt;
}

string
TransferWord::source(string const &part, bool with_queue)
{
  if(with_queue)
  {
    return access(s_str, part);
  }
  else
  {
    return access(s_str.substr(0, s_str.size() - queue_length), part);
  }
}

string
TransferWord::target(string const &part, bool with_queue)
{
  if(with_queue)
  {
    return access(t_str, part);
  }
  else
  {
    return access(t_str.substr(0, t_str.size() - queue_length), part);
  }
}

void
TransferWord::assign(string &str, string const &part, string const &value)
{
  pcrecpp::RE(part, 
              pcrecpp::RE_Options().set_caseless(true)
                                   .set_extended(true)
                                   .set_utf8(true)).Replace(value, &str);
}

void
TransferWord::setSource(string const &part, string const &value, 
			bool with_queue)
{
  if(with_queue)
  {
    assign(s_str, part, value);
  }
  else
  {
    string mystring = s_str.substr(0, s_str.size() - queue_length);
    assign(mystring, part, value);
    s_str = mystring + s_str.substr(s_str.size() - queue_length);
  }
}

void
TransferWord::setTarget(string const &part, string const &value, 
			bool with_queue)
{
  if(with_queue)
  {
    assign(t_str, part, value);
  }
  else
  {
    string mystring = t_str.substr(0, t_str.size() - queue_length);
    assign(mystring, part, value);
    t_str = mystring + t_str.substr(t_str.size() - queue_length);
  }
}
