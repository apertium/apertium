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
#include <apertium/transfer_instr.h>
#include <apertium/string_utils.h>

using namespace Apertium;
void
TransferInstr::copy(TransferInstr const &o)
{
  type = o.type;
  content = o.content;
  pos = o.pos;
  pointer = o.pointer;
  condition = o.condition;
}

void
TransferInstr::destroy()
{
}

TransferInstr::TransferInstr(TransferInstrType t, string const &c, 
                             int const p, void *ptr, bool cond)
{
  type = t;
  content = c;
  pos = p;
  pointer = ptr;
  condition = cond;
}

TransferInstr::~TransferInstr()
{
  destroy();
}

TransferInstr::TransferInstr(TransferInstr const &o)
{
  copy(o);
}

TransferInstr &
TransferInstr::operator =(TransferInstr const &o)
{
  if(this != &o)
  {
    destroy();
    copy(o);
  }
  return *this;
}

TransferInstrType
TransferInstr::getType()
{
  return type;
}

string const &
TransferInstr::getContent()
{
  return content;
}

int
TransferInstr::getPos()
{
  return pos;
}

void *
TransferInstr::getPointer()
{
  return pointer;
}

bool
TransferInstr::getCondition()
{
  return condition;
}
