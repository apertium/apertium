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

/* This class compiles Meta-Tagger XML description into a PerceptronSpec. Tree
 * of expressions are lowered to stack instructions. Named variables/references
 * are lowered to integer indices. Literals are lowered put into constant
 * vectors and lowered to integer indices. Some higher level features such as
 * macros are handled entirely by this class and not part of the runtime at all. */

namespace Apertium {
class MTXReader : public XMLReader
{
  enum ExprType {
    VOIDEXPR, INTEXPR, BEXPR, STREXPR, STRARREXPR, WRDEXPR, WRDARREXPR, ADDREXPR
  };

  typedef PerceptronSpec VM;
  typedef std::map<std::wstring, size_t> VarNVMap;
  typedef std::vector<std::pair<size_t, ExprType> > TemplateReplacements;
  typedef std::map<std::pair<size_t, std::vector<VM::FeatureDefn> >, size_t> InstanciationMap;
  typedef std::pair<VM::FeatureDefn, TemplateReplacements> TemplateDefn;
public:
  void printTmplDefns();
  MTXReader(VM &spec);
  VM &spec;

protected:
  virtual void parse();

private:
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
  int getInt(std::string attr_name, bool& exists);
  int getInt(bool& exists);
  int getInt(std::string attr_name);
  int getInt();

  void procCoarseTags();
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
  void procDefMacro();
  bool tryProcSubscript(bool (MTXReader::*proc_inner)(bool));
  bool tryProcSlice(bool (MTXReader::*proc_inner)(bool));
  bool tryProcArg(ExprType type, bool allow_fail=false);
  bool tryProcVar(VM::StackValueType);
  void procAddrExpr();
  bool procWordoidArrExpr(bool allow_fail=false);
  bool procWordoidExpr(bool allow_fail=false);
  bool procIntExpr(bool allow_fail=false);
  bool procStrArrExpr(bool allow_fail=false);
  bool procStrExpr(bool allow_fail=false);
  bool procBoolExpr(bool allow_fail=false);
  void printTmplDefn(const TemplateDefn &tmpl_defn);
  void printStackValueType(VM::StackValueType svt);
  void printTypeExpr(ExprType expr_type);
  void procTypeExpr(ExprType type);
  void procOut();
  void procOutMany();
  void procForEach(ExprType type);
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

  bool in_global_defn;
  size_t slot_counter;
  size_t template_slot_counter;
  VarNVMap set_names;
  VarNVMap str_names;
  VarNVMap slot_names;
  std::vector<VM::StackValueType> slot_types;
  /*
  VarNVMap global_slot_names;
  std::vector<VM::StackValueType> global_slot_types;
  */
  VarNVMap template_slot_names;
  std::vector<VM::StackValueType> template_slot_types;
  VarNVMap template_arg_names;
  std::vector<TemplateDefn> template_defns;
  InstanciationMap template_instanciations;
  std::stack<size_t> loop_stack;
  VM::FeatureDefn *cur_feat;
  TemplateReplacements *cur_replacements;
};
}

#endif
