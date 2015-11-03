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
#include <apertium/transfer_token.h>
#include <apertium/string_utils.h>

using namespace Apertium;

void
TransferToken::copy(TransferToken const &o)
{
  type = o.type;
  content = o.content;
}

void
TransferToken::destroy()
{
}

TransferToken::TransferToken() :
type(tt_eof)
{
}

TransferToken::TransferToken(wstring const &content,
			     TransferTokenType type)
{
  this->content = content;
  this->type = type;
}

TransferToken::~TransferToken()
{
  destroy();
}

TransferToken::TransferToken(TransferToken const &o)
{
  copy(o);
}

TransferToken &
TransferToken::operator =(TransferToken const &o)
{
  if(this != &o)
  {
    destroy();
    copy(o);
  }
  return *this;
}

TransferTokenType
TransferToken::getType()
{
  return type;
}

wstring & 
TransferToken::getContent()
{
  return content;
}

void 
TransferToken::setType(TransferTokenType type)
{
  this->type = type;
}

void 
TransferToken::setContent(wstring const &content)
{
  this->content = content;
}

