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
#ifndef _TRANSFERTOKEN_
#define _TRANSFERTOKEN_

#include <string>

using namespace std;

enum TransferTokenType
{
  tt_eof,
  tt_word,
  tt_blank
};


class TransferToken
{
private:
  TransferTokenType type;
  wstring content;

  void copy(TransferToken const &o);
  void destroy();
public:
  TransferToken();
  TransferToken(wstring const &content, TransferTokenType type);
  ~TransferToken();
  TransferToken(TransferToken const &o);
  TransferToken & operator =(TransferToken const &o);
  TransferTokenType getType();
  wstring & getContent();
  void setType(TransferTokenType type);
  void setContent(wstring const &content);
};

#endif
