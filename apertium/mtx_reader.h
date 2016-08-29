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
#ifndef _MTXREADER_
#define _MTXREADER_

#include <apertium/constant_manager.h>
#include <apertium/tagger_data_perceptron.h>
#include <apertium/perceptron_spec.h>
#include <apertium/ttag.h>
#include <apertium/xml_reader.h>
#include <lttoolbox/pattern_list.h>
#include <lttoolbox/ltstr.h>

#include <libxml/xmlreader.h>
#include <map>
#include <set>
#include <string>
#include <vector>

namespace Apertium {
class MTXReader : public XMLReader
{
  typedef PerceptronSpec VM;
public:
  MTXReader(VM &spec);
  VM &spec;

protected:
  virtual void parse();

private:
  typedef std::map<std::wstring, size_t> ConstNVMap;

  size_t pushSetConst(std::string &val);
  size_t pushStrConst(std::string &val);
  size_t getConstRef(const std::wstring &ref_attr, const std::string &lit_attr,
                     const std::wstring &what, ConstNVMap &const_map,
                     size_t (MTXReader::*push_new)(std::string&), bool& exists);
  size_t getSetRef(bool& exists);
  size_t getStrRef(bool& exists);
  void emitBytecode(VM::Bytecode bc);
  void emitOpcode(VM::Opcode op);
  void emitInt(int val);
  void emitUInt(int val);
  bool emitPushTokAddrFromAttr(bool is_tagged);
  void emitPushTokAddrFromAttr();
  void emitPushAddrFromAttr();
  int getInt(bool& exists);

  void procSetDef();
  void procStrDef();
  void procDefns();
  void procBinOp(void (MTXReader::*procLeft)(void),
                 void (MTXReader::*procRight)(void), VM::Opcode op);
  void emitSetImmOp(VM::Opcode op);
  void emitStrImmOp(VM::Opcode op);
  void procBinCompareOp(VM::Opcode op);
  void procCommBoolOp(VM::Opcode op);
  void procIntExpr();
  void procStrArrExpr();
  void procStrExpr();
  void procBoolExpr();
  void procPred();
  template<typename GetT, typename EmitT> void emitAttr(
      std::wstring what, GetT (MTXReader::*getter)(bool&),
      void (MTXReader::*emitter)(EmitT));
  void emitStrRef();
  void emitSetRef();
  void emitInt();
  void procInst();
  void procFeat();
  void procFeats();
  void copy(MTXReader const &o);
  MTXReader(MTXReader const &o);

  static ConstNVMap set_names;
  static ConstNVMap str_names;
  VM::Feature *cur_feat;
};
}

#endif
