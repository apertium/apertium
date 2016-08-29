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
#include <apertium/mtx_reader.h>
#include <lttoolbox/xml_parse_util.h>
#include <lttoolbox/compression.h>
#include <apertium/string_utils.h>
#include <apertium/utils.h>
#include <apertium/tsx_reader.h>
#include <apertium/perceptron_spec.h>

#include <cstdlib>
#include <iostream>
#include <sstream>
#include <iterator>

// XML parsing function guideline
// When control is pass to you, you might need to stepToTag
// When delegating or returning control, step beyond yourself

namespace Apertium {
MTXReader::MTXReader(VM &spec) :
    spec(spec), in_global_defn(false),
    template_slot_counter(0), cur_feat(NULL) {}

size_t MTXReader::pushSetConst(std::string &val)
{
  size_t set_idx = spec.set_consts.size();
  stringstream val_ss(val);
  spec.set_consts.push_back(set<std::string>(
    istream_iterator<std::string>(val_ss),
    istream_iterator<std::string>()
  ));
  return set_idx;
}

size_t MTXReader::pushStrConst(std::string &val)
{
  size_t str_idx = spec.str_consts.size();
  spec.str_consts.push_back(val);
  return str_idx;
}

void MTXReader::emitBytecode(VM::Bytecode bc)
{
  cur_feat->push_back(bc.intbyte);
}

void MTXReader::pokeBytecode(size_t addr, VM::Bytecode bc)
{
  (*cur_feat)[addr] = bc.intbyte;
}

void MTXReader::emitOpcode(VM::Opcode op)
{
  emitBytecode((VM::Bytecode){.op = op});
}

void MTXReader::emitInt(int val)
{
  assert(-128 <= val && val < 128);
  emitBytecode((VM::Bytecode){.intbyte = (signed char)val});
}

void MTXReader::emitUInt(int val)
{
  assert(0 <= val && val < 256);
  emitBytecode((VM::Bytecode){.uintbyte = (unsigned char)val});
}

void MTXReader::procCoarseTags()
{
  std::string tsx_fn = attrib("tag");
  bool is_abs = ((tsx_fn.size() >= 1 && tsx_fn[0] == '/') ||
                 (tsx_fn.size() >= 2 && tsx_fn[1] == ':'));
  if (!is_abs) {
    size_t last_slash_pos = path.rfind("/");
    if (last_slash_pos != string::npos) {
      tsx_fn = path.substr(0, last_slash_pos + 1) + tsx_fn;
    }
  }
  TSXReader tsx_reader;
  tsx_reader.read(tsx_fn);
  spec.coarse_tags = Optional<TaggerDataPercepCoarseTags>(
      tsx_reader.getTaggerData());
  stepPastSelfClosingTag(L"coarse-tags");
}

void MTXReader::procSetDef()
{
  std::wstring name = attrib(L"name");
  stepToNextTag();
  size_t set_idx = spec.set_consts.size();
  spec.set_consts.push_back(VMSet());
  VMSet &vm_set = spec.set_consts.back();
  while (type != XML_READER_TYPE_END_ELEMENT) {
    if (name == L"set-member") {
      std::string tag = attrib("tag");
      std::string str = attrib("str");
      vm_set.insert(tag != "" ? tag : str);
    } else {
      parseError(L"Expected set-member");
    }
    stepToNextTag();
  }
  set_names[name] = set_idx;
  assert(name == L"def-set");
  stepToNextTag();
}

void MTXReader::procStrDef()
{
  std::wstring name = attrib(L"name");
  std::string tag = attrib("tag");
  std::string str = attrib("str");
  str_names[name] = pushStrConst(tag != "" ? tag : str);
  stepPastSelfClosingTag(L"def-str");
}

void
MTXReader::procDefns()
{
  stepToNextTag();
  while (type != XML_READER_TYPE_END_ELEMENT) {
    if (name == L"def-set") {
      procSetDef();
    } else if (name == L"def-str") {
      procStrDef();
    } else if (name == L"def-macro") {
      procDefMacro();
    } else if (name == L"#text" || name == L"#comment") {
      // skip
    } else {
      unexpectedTag();
    }
  }
  assert(name == L"defns");
  stepToNextTag();
}

void
MTXReader::procGlobalPred()
{
  cur_feat = &spec.global_pred;
  stepToNextTag();
  procBoolExpr();
  assert(name == L"global-pred" && type == XML_READER_TYPE_END_ELEMENT);
  stepToNextTag();
}

void
MTXReader::emitSetImmOp(VM::Opcode op)
{
  emitOpcode(op);
  getAndEmitSetRef();
}

void
MTXReader::emitStrImmOp(VM::Opcode op)
{
  emitOpcode(op);
  getAndEmitStrRef();
}

void
MTXReader::procBinCompareOp(VM::Opcode op)
{
  procIntExpr();
  procIntExpr();
  emitOpcode(op);
}

void
MTXReader::procCommBoolOp(VM::Opcode op)
{
  int operand_count = 0;
  while (type != XML_READER_TYPE_END_ELEMENT) {
    procBoolExpr();
    operand_count++;
  }
  while (--operand_count > 0) {
    emitOpcode(op);
  }
}

bool
MTXReader::procIntExpr(bool allow_fail)
{
  /* Self-closing tags */
  if (!tryProcArg(INTEXPR, true)
      && !tryProcVar(VM::INTVAL)) {
    if (name == L"sentlen") {
      emitOpcode(VM::SENTLENTOK);
      stepPastSelfClosingTag(L"sentlen");
    } else if (name == L"pathlen") {
      emitOpcode(VM::SENTLENWRD);
      stepPastSelfClosingTag(L"pathlen");
    } else if (name == L"tokaddr") {
      emitOpcode(VM::PUSHTOKADDR);
      stepPastSelfClosingTag(L"tokaddr");
    } else if (name == L"wrdidx") {
      emitOpcode(VM::PUSHWRDADDR);
      stepPastSelfClosingTag(L"wrdidx");
    } else if (name == L"int") {
      emitOpcode(VM::PUSHINT);
      getAndEmitInt();
      stepPastSelfClosingTag(L"int");
    /* Other tags */
    } else if (name == L"add") {
      stepToNextTag();
      procIntExpr();
      procIntExpr();
      assert(name == L"add" && type == XML_READER_TYPE_END_ELEMENT);
      emitOpcode(VM::ADD);
      stepToNextTag();
    } else if (name == L"toklen") {
      procIntExpr();
      assert(name == L"toklen" && type == XML_READER_TYPE_END_ELEMENT);
      emitOpcode(VM::TOKLENWRD);
      stepToNextTag();
    } else if (name == L"strlen") {
      procStrExpr();
      assert(name == L"strlen" && type == XML_READER_TYPE_END_ELEMENT);
      emitOpcode(VM::STRLEN);
      stepToNextTag();
    } else if (name == L"arrlen") {
      procStrArrExpr();
      assert(name == L"arrlen" && type == XML_READER_TYPE_END_ELEMENT);
      procBinCompareOp(VM::ARRLEN);
      stepToNextTag();
    } else {
      if (allow_fail) {
        return false;
      }
      parseError(L"Expected an integer expression.");
    }
  }
  return true;
}

bool
MTXReader::procStrArrExpr(bool allow_fail)
{
  stepToTag();
  if (!tryProcArg(STRARREXPR, true)
      && !tryProcVar(VM::STRARRVAL)
      && !tryProcSlice(&MTXReader::procStrArrExpr)) {
    if (name == L"ex-tags") {
      stepToNextTag();
      procWordoidExpr();
      assert(type == XML_READER_TYPE_END_ELEMENT);
      emitOpcode(VM::EXTAGS);
    } else if (name == L"ex-ambgset") {
      stepToNextTag();
      procIntExpr();
      emitOpcode(VM::EXAMBGSET);
    } else if (name == L"for-each") {
      procForEach(STREXPR);
    } else {
      if (allow_fail) {
        return false;
      }
      parseError(L"Expected a string list expression.");
    }
    stepToNextTag();
  }
  return true;
}

bool MTXReader::tryProcSubscript(bool (MTXReader::*proc_inner)(bool))
{
  if (name == L"subscript") {
    int idx = getInt("idx");
    stepToNextTag();
    (this->*proc_inner)(false);
    emitOpcode(VM::SUBSCRIPT);
    emitUInt(idx);
    assert(name == L"subscript" && type == XML_READER_TYPE_END_ELEMENT);
    stepToNextTag();
    return true;
  }
  return false;
}

bool MTXReader::tryProcSlice(bool (MTXReader::*proc_inner)(bool))
{
  if (name == L"slice") {
    stepToNextTag();
    (this->*proc_inner)(false);
    bool exists;
    emitOpcode(VM::SLICE);
    int start_lit = getInt("start", exists);
    if (exists) {
      emitInt(start_lit);
    } else {
      emitInt(0);
    }
    int end_lit = getInt("end", exists);
    if (exists) {
      emitInt(end_lit);
    } else {
      emitInt(0);
    }
    assert(name == L"slice" && type == XML_READER_TYPE_END_ELEMENT);
    stepToNextTag();
    return true;
  }
  return false;
}

bool MTXReader::tryProcArg(ExprType expr_type, bool allow_fail)
{
  if (name == L"var") {
    std::wstring var_name = attrib(L"name");
    if (in_global_defn) {
      VarNVMap::const_iterator arg_name_it = template_arg_names.find(var_name);
      if (arg_name_it != template_arg_names.end()) {
        cur_replacements->push_back(make_pair(arg_name_it->second, expr_type));
        stepPastSelfClosingTag(L"var");
        return true;
      }
      if (!allow_fail) {
        parseError(L"No such argument " + var_name);
      }
    }
  }
  return false;
}

bool MTXReader::tryProcVar(VM::StackValueType svt)
{
  if (name == L"var") {
    std::wstring var_name = attrib(L"name");

    VarNVMap::const_iterator slot_names_it = slot_names.find(var_name);
    if (slot_names_it != slot_names.end()) {
      if (slot_types[slot_names_it->second] != svt) {
        parseError(L"Variable " + var_name + L" has the wrong type");
      }
      emitOpcode(VM::GETVAR);
      emitUInt(slot_names_it->second);
      stepPastSelfClosingTag(L"var");
      return true;
    }

    parseError(L"Variable " + var_name + L" has not been set.");
  } else if (!in_global_defn && name == L"macro") {
    // Get template data
    std::wstring var_name = attrib(L"name");
    VarNVMap::const_iterator template_name_it = template_slot_names.find(var_name);
    if (template_name_it == template_slot_names.end()) {
      parseError(L"No such macro " + var_name);
    }
    size_t templ_idx = template_name_it->second;
    if (template_slot_types[templ_idx] != svt) {
      parseError(L"Macro " + var_name + L" returns the wrong type");
    }
    std::pair<VM::FeatureDefn, TemplateReplacements> &templ_defn = template_defns[templ_idx];
    // Get arg values
    stepToNextTag();
    std::vector<VM::FeatureDefn> arg_values;
    VM::FeatureDefn *saved_feat = cur_feat;
    TemplateReplacements::const_iterator templ_repl_it = templ_defn.second.begin();
    for (; templ_repl_it != templ_defn.second.end(); templ_repl_it++) {
      arg_values.push_back(VM::FeatureDefn());
      cur_feat = &arg_values.back();
      procTypeExpr(templ_repl_it->second);
    }
    cur_feat = saved_feat;
    // Check for previous instanciation...
    InstanciationMap::const_iterator templ_instcia_it =
        template_instanciations.find(make_pair(templ_idx, arg_values));
    if (templ_instcia_it == template_instanciations.end()) {
      // and instanciate if doesn't exist...
      // by ordering argument defns correctly
      vector<unsigned int> indices(templ_defn.second.size(), 0);
      for (size_t i = 0; i < templ_defn.second.size(); i++) {
        indices[i] = i;
      }
      SortByComparer<std::pair<size_t, ExprType>, int> sbc(templ_defn.second);
      std::sort(indices.begin(), indices.end(), sbc);
      // And then copying the template defn bits and the argument defn bits into the global defn
      VM::FeatureDefn::const_iterator src_start_it = templ_defn.first.begin();
      VM::FeatureDefn::const_iterator src_end_it = templ_defn.first.end();
      VM::FeatureDefn::const_iterator src_cur_it = src_start_it;
      spec.global_defns.push_back(VM::FeatureDefn());
      VM::FeatureDefn &global_defn = spec.global_defns.back();
      std::back_insert_iterator<VM::FeatureDefn> dest_it(global_defn);
      for (size_t i = 0; i < indices.size(); i++) {
        size_t repl_i = indices[i];
        size_t offset = templ_defn.second[repl_i].first;
        VM::FeatureDefn &arg_src = arg_values[repl_i];
        std::copy(src_cur_it, src_cur_it + offset, dest_it);
        src_cur_it = src_start_it + offset;
        std::copy(arg_src.begin(), arg_src.end(), dest_it);
      }
      std::copy(src_cur_it, src_end_it, dest_it);
      templ_instcia_it = template_instanciations.insert(
          make_pair(
            make_pair(templ_idx, arg_values),
            spec.global_defns.size() - 1)).first;
    }
    // Finally emit a reference to the relevant global
    emitOpcode(VM::GETGVAR);
    emitUInt(templ_instcia_it->second);
    // Step past end
    assert(name == L"macro" && type == XML_READER_TYPE_END_ELEMENT);
    stepToNextTag();
    return true;
  }
  return false;
}

bool
MTXReader::procStrExpr(bool allow_fail)
{
  if (!tryProcArg(STREXPR, true)
      && !tryProcVar(VM::STRVAL)
      && !tryProcSlice(&MTXReader::procStrExpr)
      && !tryProcSubscript(&MTXReader::procStrArrExpr)) {
    if (name == L"ex-surf") {
      stepToNextTag();
      procIntExpr();
      emitOpcode(VM::EXTOKSURF);
    } else if (name == L"ex-lemma") {
      stepToNextTag();
      procWordoidExpr();
      emitOpcode(VM::EXWRDLEMMA);
    } else if (name == L"ex-coarse") {
      stepToNextTag();
      procWordoidExpr();
      emitOpcode(VM::EXWRDCOARSETAG);
    } else if (name == L"join") {
      bool has_attr;
      size_t str_idx = getStrRef(has_attr);
      if (!has_attr) {
        str_idx = 255;
      }
      stepToNextTag();
      procStrArrExpr();
      emitOpcode(VM::JOIN);
      emitUInt(str_idx);
    } else {
      if (allow_fail) {
        return false;
      }
      parseError(L"Expected a string expression.");
    }
    assert(type == XML_READER_TYPE_END_ELEMENT);
    stepToNextTag();
  }
  return true;
}

bool
MTXReader::procBoolExpr(bool allow_fail)
{
  if (!tryProcArg(BEXPR, true)
      && !tryProcVar(VM::BVAL)) {
    if (name == L"and") {
      stepToNextTag();
      procCommBoolOp(VM::AND);
      assert(name == L"and" && type == XML_READER_TYPE_END_ELEMENT);
      stepToNextTag();
    } else if (name == L"or") {
      stepToNextTag();
      procCommBoolOp(VM::OR);
      assert(name == L"or" && type == XML_READER_TYPE_END_ELEMENT);
      stepToNextTag();
    } else if (name == L"not") {
      stepToNextTag();
      procBoolExpr();
      emitOpcode(VM::NOT);
      assert(name == L"not" && type == XML_READER_TYPE_END_ELEMENT);
      stepToNextTag();
    } else if (name == L"eq") {
      stepToNextTag();
      procBinCompareOp(VM::EQ);
      assert(name == L"eq" && type == XML_READER_TYPE_END_ELEMENT);
      stepToNextTag();
    } else if (name == L"neq") {
      stepToNextTag();
      procBinCompareOp(VM::NEQ);
      assert(name == L"neq" && type == XML_READER_TYPE_END_ELEMENT);
      stepToNextTag();
    } else if (name == L"lt") {
      stepToNextTag();
      procBinCompareOp(VM::LT);
      assert(name == L"lt" && type == XML_READER_TYPE_END_ELEMENT);
      stepToNextTag();
    } else if (name == L"lte") {
      stepToNextTag();
      procBinCompareOp(VM::LTE);
      assert(name == L"lte" && type == XML_READER_TYPE_END_ELEMENT);
      stepToNextTag();
    } else if (name == L"gt") {
      stepToNextTag();
      procBinCompareOp(VM::GT);
      assert(name == L"gt" && type == XML_READER_TYPE_END_ELEMENT);
      stepToNextTag();
    } else if (name == L"gte") {
      stepToNextTag();
      procBinCompareOp(VM::GTE);
      assert(name == L"gte" && type == XML_READER_TYPE_END_ELEMENT);
      stepToNextTag();
    } else if (name == L"streq") {
      size_t str_ref = getStrRef();
      stepToNextTag();
      procStrExpr();
      emitOpcode(VM::STREQ);
      emitUInt(str_ref);
      assert(name == L"streq" && type == XML_READER_TYPE_END_ELEMENT);
      stepToNextTag();
    } else if (name == L"strin") {
      size_t set_ref = getSetRef();
      stepToNextTag();
      procStrExpr();
      emitOpcode(VM::STRIN);
      emitUInt(set_ref);
      assert(name == L"strin" && type == XML_READER_TYPE_END_ELEMENT);
      stepToNextTag();
    /* Identical to strin?
    } else if (name == L"sethas") {
      stepToNextTag();
      procStrExpr();
      emitSetImmOp(VM::SETHAS);
    */
    } else if (name == L"sethasany") {
      size_t set_ref = getSetRef();
      stepToNextTag();
      procStrArrExpr();
      emitOpcode(VM::SETHASANY);
      emitUInt(set_ref);
      assert(name == L"sethasany" && type == XML_READER_TYPE_END_ELEMENT);
      stepToNextTag();
    } else if (name == L"sethasall") {
      size_t set_ref = getSetRef();
      stepToNextTag();
      procStrArrExpr();
      emitOpcode(VM::SETHASALL);
      emitUInt(set_ref);
      assert(name == L"sethasall" && type == XML_READER_TYPE_END_ELEMENT);
      stepToNextTag();
    } else {
      if (allow_fail) {
        return false;
      }
      parseError(L"Expected a boolean expression.");
    }
  }
  return true;
}

void
MTXReader::procAddrExpr()
{
  stepToTag();
  /* Self-closing tags */
  if (!tryProcArg(ADDREXPR)) {
    if (name == L"wrdaddr") {
      emitOpcode(VM::PUSHADDR);
      stepPastSelfClosingTag(L"wrdaddr");
    /* Others */
    } else if (name == L"addr-of-ints") {
      stepToNextTag();
      procIntExpr();
      procIntExpr();
      assert(name == L"addr-of-ints" && type == XML_READER_TYPE_END_ELEMENT);
      stepToNextTag();
    } else if (name == L"add") {
      stepToNextTag();
      procAddrExpr();
      procAddrExpr();
      assert(name == L"add" && type == XML_READER_TYPE_END_ELEMENT);
      emitOpcode(VM::ADD2);
      stepToNextTag();
    } else if (name == L"adjust") {
      stepToNextTag();
      procAddrExpr();
      assert(name == L"adjust" && type == XML_READER_TYPE_END_ELEMENT);
      emitOpcode(VM::ADJADDR);
      stepToNextTag();
    } else if (name == L"clamp") {
      stepToNextTag();
      procAddrExpr();
      assert(name == L"clamp" && type == XML_READER_TYPE_END_ELEMENT);
      emitOpcode(VM::CLAMPADDR);
      stepToNextTag();
    } else {
      parseError(L"Expected an address expression.");
    }
  }
}

bool
MTXReader::procWordoidArrExpr(bool allow_fail)
{
  if (!tryProcArg(WRDARREXPR, true)
      && !tryProcVar(VM::WRDARRVAL)
      && !tryProcSlice(&MTXReader::procWordoidArrExpr)) {
    if (name == L"ex-wordoids") {
      stepToNextTag();
      procIntExpr();
      emitOpcode(VM::EXWRDARR);
      assert(name == L"ex-wordoids" && type == XML_READER_TYPE_END_ELEMENT);
    } else if (name == L"for-each") {
      procForEach(WRDEXPR);
    } else {
      if (allow_fail) {
        return false;
      }
      parseError(L"Expected a wordoid array expression.");
    }
    stepToNextTag();
  }
  return true;
}

bool
MTXReader::procWordoidExpr(bool allow_fail)
{
  stepToTag();
  if (!tryProcArg(WRDEXPR, true)
      && !tryProcVar(VM::WRDVAL)
      && !tryProcSubscript(&MTXReader::procWordoidArrExpr)) {
    if (name == L"ex-wordoid") {
      stepToNextTag();
      procAddrExpr();
      emitOpcode(VM::GETWRD);
    } else {
      if (allow_fail) {
        return false;
      }
      parseError(L"Expected a wordoid expression.");
    }
    assert(type == XML_READER_TYPE_END_ELEMENT);
    stepToNextTag();
  }
  return true;
}

void
MTXReader::procPred()
{
  stepToNextTag();
  procBoolExpr();
  assert(name == L"pred" && type == XML_READER_TYPE_END_ELEMENT);
  emitOpcode(VM::DIEIFFALSE);
  stepToNextTag();
}

size_t
MTXReader::getConstRef(
    const std::wstring &ref_attr,
    const std::string &lit_attr,
    const std::wstring &what,
    VarNVMap &const_map,
    size_t (MTXReader::*push_new)(std::string&),
    bool& exists)
{
  std::wstring const_name = attrib(ref_attr);
  if (!const_name.empty()) {
    exists = true;
    VarNVMap::iterator sit = const_map.find(const_name);
    if (sit == const_map.end()) {
      parseError(L"No " + what + L" named " + const_name);
    }
    return sit->second;
  }
  std::string const_lit = attrib(lit_attr);
  if (!const_lit.empty()) {
    exists = true;
    return (this->*push_new)(const_lit);
  }
  exists = false;
  return 0;
}

size_t
MTXReader::getSetRef(bool& exists)
{
  return getConstRef(L"name", "val", L"set", set_names, &MTXReader::pushSetConst, exists);
}

size_t
MTXReader::getSetRef()
{
  bool has_attr;
  size_t set_ref = getSetRef(has_attr);
  if (!has_attr) {
    parseError(L"Set required");
  }
  return set_ref;
}

size_t
MTXReader::getStrRef(bool& exists)
{
  return getConstRef(L"name", "val", L"string", str_names, &MTXReader::pushStrConst, exists);
}

size_t
MTXReader::getStrRef()
{
  bool has_attr;
  size_t str_ref = getStrRef(has_attr);
  if (!has_attr) {
    parseError(L"String required");
  }
  return str_ref;
}

int
MTXReader::getInt(std::string attr_name, bool& exists)
{
  std::string int_lit = attrib(attr_name);
  if (!int_lit.empty()) {
    exists = true;
    int int_out;
    stringstream int_ss(int_lit);
    int_ss >> int_out;
    return int_out;
  }
  exists = false;
  return 0;
}

int
MTXReader::getInt(bool& exists)
{
  return getInt("val", exists);
}

int
MTXReader::getInt(std::string attr_name)
{
  bool has_attr;
  int i = getInt(attr_name, has_attr);
  if (!has_attr) {
    parseError(L"String required");
  }
  return i;
}

int
MTXReader::getInt()
{
  return getInt("val");
}

template<typename GetT, typename EmitT>
void
MTXReader::emitAttr(
    std::wstring what, GetT (MTXReader::*getter)(bool&), void (MTXReader::*emitter)(EmitT))
{
  bool has_attr = false;
  GetT val = (this->*getter)(has_attr);
  if (!has_attr) {
    parseError(what + L" required");
  }
  (this->*emitter)(val);
}

void
MTXReader::getAndEmitStrRef()
{
  emitAttr(L"String", &MTXReader::getStrRef, &MTXReader::emitUInt);
}

void
MTXReader::getAndEmitSetRef()
{
  emitAttr(L"Set", &MTXReader::getSetRef, &MTXReader::emitUInt);
}

void
MTXReader::getAndEmitInt()
{
  emitAttr(L"Integer", &MTXReader::getInt, &MTXReader::emitInt);
}

void
MTXReader::procInst()
{
  // XXX: There's no way to tell the difference between an empty and absent
  // attribute with the current lttoolbox xml code
  std::string op = attrib("opcode");
  std::transform(op.begin(), op.end(), op.begin(), ::toupper);
  emitOpcode(VM::opcode_values[op]);
  int val;
  bool has_set_ref;
  val = getSetRef(has_set_ref);
  bool has_str_ref;
  val = getStrRef(has_str_ref);
  bool has_int_lit;
  val = getInt(has_int_lit);
  int num_operands = has_set_ref + has_str_ref + has_int_lit;
  if (num_operands > 1) {
    parseError(L"Opcodes can have at most one operand.");
  } else if (num_operands == 1) {
    if (has_int_lit) {
      emitInt(val);
    } else {
      emitUInt(val);
    }
  }
}

void
MTXReader::procOut()
{
  bool has_expr = false;
  stepToNextTag();
  if (procStrExpr(true)) {
    emitOpcode(VM::FCATSTR);
    has_expr = true;
  }
  if (!has_expr && procBoolExpr(true)) {
    emitOpcode(VM::FCATBOOL);
    has_expr = true;
  }
  if (!has_expr && procIntExpr(true)) {
    emitOpcode(VM::FCATINT);
    has_expr = true;
  }
  if (!has_expr) {
    parseError(L"Expected a string, bool or int expression.");
  }
  stepToTag();
  assert(name == L"out" && type == XML_READER_TYPE_END_ELEMENT);
  stepToNextTag();
}

void
MTXReader::procOutMany()
{
  stepToNextTag();
  procStrArrExpr();
  emitOpcode(VM::FCATSTRARR);
  assert(name == L"out-many" && type == XML_READER_TYPE_END_ELEMENT);
  stepToNextTag();
}

void
MTXReader::printTmplDefn(const TemplateDefn &tmpl_defn)
{
  PerceptronSpec::printFeature(std::wcerr, tmpl_defn.first);
  if (tmpl_defn.second.size() > 0) {
    std::wcerr << "Replacements:\n";
    TemplateReplacements::const_iterator it = tmpl_defn.second.begin();
    for (; it != tmpl_defn.second.end(); it++) {
      std::wcerr << "Index: " << it->first << " ";
      printTypeExpr(it->second);
      std::wcerr << "\n";
    }
  }
}

void
MTXReader::printStackValueType(VM::StackValueType svt)
{
  switch (svt) {
    case VM::INTVAL:
      std::wcerr << "INT";
      break;
    case VM::BVAL:
      std::wcerr << "BOOL";
      break;
    case VM::STRVAL:
      std::wcerr << "STR";
      break;
    case VM::STRARRVAL:
      std::wcerr << "STRARR";
      break;
    case VM::WRDVAL:
      std::wcerr << "WRD";
      break;
    case VM::WRDARRVAL:
      std::wcerr << "WRDARR";
      break;
    default:
      assert(false);
  }
}

void
MTXReader::printTypeExpr(ExprType expr_type)
{
  switch (expr_type) {
    case VOIDEXPR:
      std::wcerr << "VOID";
      break;
    case INTEXPR:
      std::wcerr << "INT";
      break;
    case BEXPR:
      std::wcerr << "BOOL";
      break;
    case STREXPR:
      std::wcerr << "STR";
      procStrExpr();
      break;
    case STRARREXPR:
      std::wcerr << "STRARR";
      break;
    case WRDEXPR:
      std::wcerr << "WRD";
      break;
    case WRDARREXPR:
      std::wcerr << "WRDARR";
      break;
    case ADDREXPR:
      std::wcerr << "ADDR";
      break;
    default:
      assert(false);
  }
}

void
MTXReader::procTypeExpr(ExprType expr_type)
{
  switch (expr_type) {
    case VOIDEXPR:
      procVoidExpr();
      break;
    case INTEXPR:
      procIntExpr();
      break;
    case BEXPR:
      procBoolExpr();
      break;
    case STREXPR:
      procStrExpr();
      break;
    case STRARREXPR:
      procStrArrExpr();
      break;
    case WRDEXPR:
      procWordoidExpr();
      break;
    case WRDARREXPR:
      procWordoidArrExpr();
      break;
    case ADDREXPR:
      procAddrExpr();
      break;
    default:
      assert(false);
  }
}

void
MTXReader::procForEach(ExprType expr_type)
{
  std::wstring var_name = attrib(L"as");
  if (var_name == L"") {
    parseError(L"'as' attribute required for for-each.");
  }
  size_t slot_idx = slot_counter++;
  slot_names[var_name] = slot_idx;
  bool has_expr = false;
  stepToNextTag();
  if (procStrArrExpr(true)) {
    slot_types.push_back(VM::STRVAL);
    has_expr = true;
  }
  if (!has_expr && procWordoidArrExpr(true)) {
    slot_types.push_back(VM::WRDVAL);
    has_expr = true;
  }
  if (!has_expr) {
    parseError(L"Expected a string array or wordoid array expression.");
  }

  emitOpcode(VM::FOREACHINIT);
  size_t for_each_begin = cur_feat->size();
  emitOpcode(VM::FOREACH);
  emitUInt(slot_idx);
  size_t begin_offset_placeholder = cur_feat->size();
  emitUInt(0); // offset placeholder
  size_t for_each_body_begin = cur_feat->size();

  switch (expr_type) {
    case VOIDEXPR:
      procVoidExpr();
      break;
    case STREXPR:
      procStrExpr();
      break;
    case WRDEXPR:
      procWordoidExpr();
      break;
    default:
      assert(false);
      break;
  }
  assert(type == XML_READER_TYPE_END_ELEMENT);

  size_t end_for_each_addr = cur_feat->size();
  emitOpcode(VM::ENDFOREACH);
  emitInt(end_for_each_addr + 2 - for_each_begin); // offset
  pokeBytecode(begin_offset_placeholder, (VM::Bytecode){
    .uintbyte=(unsigned char)(end_for_each_addr + 2 - for_each_body_begin)});
}

bool
MTXReader::procVoidExpr(bool allow_fail)
{
  stepToTag();
  if (name == L"pred") {
    procPred();
  } else if (name == L"out") {
    procOut();
  } else if (name == L"out-many") {
    procOutMany();
  } else if (name == L"for-each") {
    procForEach(VOIDEXPR);
  } else if (name == L"inst") {
    procInst();
  } else {
    if (allow_fail) {
      return false;
    }
    parseError(L"Expected a void expression.");
  }
  return true;
}

void
MTXReader::procDefMacro()
{
  in_global_defn = true;
  slot_counter = 0;
  template_defns.push_back(make_pair(VM::FeatureDefn(), TemplateReplacements()));
  cur_feat = &template_defns.back().first;
  cur_replacements = &template_defns.back().second;

  std::wstring var_name = attrib(L"as");
  if (var_name == L"") {
    parseError(L"'as' attribute required for def-macro.");
  }
  template_slot_names[var_name] = template_slot_counter;

  template_arg_names.clear();
  std::wstring args = attrib(L"args");
  std::wistringstream args_ss(args);
  size_t arg_i = 0;
  for (; !args_ss.eof(); arg_i++) {
    wstring arg_name;
    args_ss >> arg_name;
    if (arg_name == L"") {
      break;
    }
    template_arg_names[arg_name] = arg_i;
  }
  stepToNextTag();

  bool has_expr = false;
  if (procIntExpr(true)) {
    template_slot_types.push_back(VM::INTVAL);
    has_expr = true;
  }
  if (!has_expr && procBoolExpr(true)) {
    template_slot_types.push_back(VM::BVAL);
    has_expr = true;
  }
  if (!has_expr && procStrExpr(true)) {
    template_slot_types.push_back(VM::STRVAL);
    has_expr = true;
  }
  if (!has_expr && procStrArrExpr(true)) {
    template_slot_types.push_back(VM::STRARRVAL);
    has_expr = true;
  }
  if (!has_expr && procWordoidArrExpr(true)) {
    template_slot_types.push_back(VM::WRDARRVAL);
    has_expr = true;
  }
  if (!has_expr && procWordoidExpr(true)) {
    template_slot_types.push_back(VM::WRDVAL);
    has_expr = true;
  }
  if (!has_expr) {
    parseError(L"Expected a non-void expression.");
  }
  assert(name == L"def-macro" && type == XML_READER_TYPE_END_ELEMENT);
  stepToNextTag();

  template_slot_counter++;
  in_global_defn = false;
}

void
MTXReader::procFeat()
{
  slot_counter = 0;
  spec.features.push_back(VM::FeatureDefn());
  cur_feat = &spec.features.back();
  stepToNextTag();
  while (type != XML_READER_TYPE_END_ELEMENT) {
    procVoidExpr();
  }
  assert(name == L"feat");
  stepToNextTag();
}

void
MTXReader::procFeats()
{
  stepToNextTag();
  while (type != XML_READER_TYPE_END_ELEMENT) {
    if (name == L"feat") {
      procFeat();
    } else {
      unexpectedTag();
    }
  }
  assert(name == L"feats");
  stepToNextTag();
}

void
MTXReader::printTmplDefns()
{
  std::vector<TemplateDefn>::const_iterator it = template_defns.begin();
  for (; it != template_defns.end(); it++) {
    std::wcerr << " Macro " << it - template_defns.begin() << "\n";
    printTmplDefn(*it);
  }
}

void
MTXReader::parse()
{
  xmlTextReaderSetParserProp(reader, XML_PARSER_SUBST_ENTITIES, true);
  stepToNextTag();
  if (type == XML_READER_TYPE_DOCUMENT_TYPE) {
    stepToNextTag();
  }
  if (name != L"metatag") {
    parseError(L"expected <metatag> tag");
  }
  stepToNextTag();
  if (name == L"coarse-tags") {
    procCoarseTags();
  }
  if (name == L"beam-width") {
    size_t val;
    std::istringstream val_ss(attrib("val"));
    val_ss >> val;
    spec.beam_width = val;
  } else {
    spec.beam_width = 4;
  }
  if (name == L"defns") {
    procDefns();
  }
  if (name == L"global-pred") {
    procGlobalPred();
  }
  if (name == L"feats") {
    procFeats();
  }
  assert(name == L"metatag" && type == XML_READER_TYPE_END_ELEMENT);
}
}
