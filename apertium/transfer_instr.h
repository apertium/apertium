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
#ifndef _TRANSFERINSTR_
#define _TRANSFERINSTR_

#include <libxml/tree.h>
#include <string>
#include <lttoolbox/ustring.h>

using namespace std;

enum TransferInstrType
{
  ti_clip_sl,
  ti_clip_tl,
  ti_clip_ref,
  ti_var,
  ti_lit_tag,
  ti_lit,
  ti_b,
  ti_get_case_from,
  ti_case_of_sl,
  ti_case_of_tl,
  ti_case_of_ref,
  ti_linkto_sl,
  ti_linkto_tl,
  ti_linkto_ref,
  ti_lu_count
};

class TransferInstr
{
private:
  TransferInstrType type;
  UString content;
  int pos;
  xmlNode* pointer;
  bool condition;
  UString strval;

  void copy(TransferInstr const &o);
  void destroy();
public:
  TransferInstr() :
  type(ti_clip_sl),
  pos(0),
  pointer(0),
  condition(false)
  {}
  TransferInstr(TransferInstrType t, UString const &c, int const p,
                xmlNode* ptr=NULL, bool cond = true, const UString& sv = ""_u);
  ~TransferInstr();
  TransferInstr(TransferInstr const &o);
  TransferInstr & operator =(TransferInstr const &o);


  TransferInstrType getType();
  UString const & getContent();
  int getPos();
  xmlNode* getPointer();
  bool getCondition();
  const UString& getStrval();
};

#endif
