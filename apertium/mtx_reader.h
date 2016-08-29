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
  typedef std::map<std::wstring, size_t> VarNVMap;

  size_t pushSetConst(std::string &val);
  size_t pushStrConst(std::string &val);
  size_t getConstRef(const std::wstring &ref_attr, const std::string &lit_attr,
                     const std::wstring &what, VarNVMap &const_map,
                     size_t (MTXReader::*push_new)(std::string&), bool& exists);
  size_t getSetRef(bool& exists);
  size_t getSetRef();
  size_t getStrRef(bool& exists);
  size_t getStrRef();
  void emitBytecode(VM::Bytecode bc);
  void emitOpcode(VM::Opcode op);
  void pokeBytecode(size_t addr, VM::Bytecode bc);
  void emitInt(int val);
  void emitUInt(int val);
  bool emitPushTokAddrFromAttr(bool is_tagged);
  void emitPushTokAddrFromAttr();
  void emitPushAddrFromAttr();
  int getInt(std::string attr_name, bool& exists);
  int getInt(bool& exists);
  int getInt(std::string attr_name);
  int getInt();

  void procSetDef();
  void procStrDef();
  void procDefns();
  void procGlobalPred();
  void procBinOp(void (MTXReader::*procLeft)(void),
                 void (MTXReader::*procRight)(void), VM::Opcode op);
  void emitSetImmOp(VM::Opcode op);
  void emitStrImmOp(VM::Opcode op);
  void procBinCompareOp(VM::Opcode op);
  void procCommBoolOp(VM::Opcode op);
  bool procVoidExpr(bool allow_fail=false);
  void procDefGlobal();
  bool tryProcSubscript(bool (MTXReader::*proc_inner)(bool));
  bool tryProcSlice(bool (MTXReader::*proc_inner)(bool));
  bool tryProcVar(VM::StackValueType);
  void procAddrExpr();
  bool procWordoidArrExpr(bool allow_fail=false);
  bool procWordoidExpr(bool allow_fail=false);
  bool procIntExpr(bool allow_fail=false);
  bool procStrArrExpr(bool allow_fail=false);
  bool procStrExpr(bool allow_fail=false);
  bool procBoolExpr(bool allow_fail=false);
  void procOut();
  void procOutMany();
  void procForEach(Optional<VM::StackValueType> svt);
  void procPred();
  template<typename GetT, typename EmitT> void emitAttr(
      std::wstring what, GetT (MTXReader::*getter)(bool&),
      void (MTXReader::*emitter)(EmitT));
  void getAndEmitStrRef();
  void getAndEmitSetRef();
  void getAndEmitInt();
  void procInst();
  void procFeat();
  void procFeats();
  void copy(MTXReader const &o);
  MTXReader(MTXReader const &o);

  size_t slot_counter;
  size_t global_slot_counter;
  VarNVMap set_names;
  VarNVMap str_names;
  VarNVMap slot_names;
  std::vector<VM::StackValueType> slot_types;
  VarNVMap global_slot_names;
  std::vector<VM::StackValueType> global_slot_types;
  std::stack<size_t> loop_stack;
  VM::FeatureDefn *cur_feat;
};
}

#endif
