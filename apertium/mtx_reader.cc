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

#include <cstdlib>
#include <iostream>
#include <sstream>
#include <iterator>

// XML parsing function guideline
// When control is pass to you, you might need to stepToTag
// When delegating or returning control, step beyond yourself

namespace Apertium {
MTXReader::MTXReader(VM &spec) : spec(spec), cur_feat(NULL) {}

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

/*
bool MTXReader::emitPushTokAddrFromAttr(bool is_tagged)
{
  std::string addr = attrib("tokaddr");
  if (addr.length() == 0) {
    emitOpcode(VM::PUSHTOKADDR);
    return false;
  }
  bool pushed_initial = false;
  bool is_rel = true;
  char op;
  int offset;
  std::istringstream addr_ss(addr);
  while (!addr_ss.eof()) {
    addr_ss >> op;
    switch (op) {
      case 'C':
        if (is_tagged) {
          emitOpcode(VM::CLAMPTAGGEDTOKADDR);
        } else {
          emitOpcode(VM::CLAMPTOKADDR);
        }
        break;
      case 'X':
        emitOpcode(VM::DUP);
        if (is_tagged) {
          emitOpcode(VM::ISVALIDTAGGEDTOKADDR);
        } else {
          emitOpcode(VM::ISVALIDTOKADDR);
        }
        emitOpcode(VM::DIEIFFALSE);
        break;
      case '.':
        is_rel = false;
        break;
      case ' ':
        is_rel = true;
        break;
      default:
        addr_ss >> offset;
        if (!pushed_initial) {
          if (is_rel) {
            emitOpcode(VM::PUSHTOKADDR);
          } else if (0 <= offset) {
            emitOpcode(VM::PUSHZERO);
          } else {
            if (is_tagged) {
              emitOpcode(VM::SENTLENTAGGEDTOK);
            } else {
              emitOpcode(VM::SENTLENTOK);
            }
          }
          pushed_initial = true;
        }
        emitOpcode(VM::PUSHINT);
        emitInt(offset);
        emitOpcode(VM::ADD);
        break;
    }
  }
  return true;
}

void MTXReader::emitPushTokAddrFromAttr()
{
  emitPushTokAddrFromAttr(false);
}

void MTXReader::emitPushAddrFromAttr()
{
  bool has_tok_addr = emitPushTokAddrFromAttr(true);
  std::string addr = attrib("wrdaddr");
  if (addr.length() == 0) {
    if (has_tok_addr) {
      emitOpcode(VM::PUSHINT);
      emitInt(0);
    } else {
      emitOpcode(VM::PUSHWRDADDR);
    }
    return;
  }
  bool pushed_initial = false;
  bool is_rel = true;
  char op;
  int offset;
  std::istringstream addr_ss(addr);
  while (!addr_ss.eof()) {
    addr_ss >> op;
    switch (op) {
      case 'A':
        emitOpcode(VM::ADJADDR);
        break;
      case 'C':
        emitOpcode(VM::CLAMPADDR);
        break;
      case 'X':
        emitOpcode(VM::DUP2);
        emitOpcode(VM::ISVALIDADDR);
        emitOpcode(VM::DIEIFFALSE);
        break;
      case '.':
        is_rel = false;
        break;
      case ' ':
        is_rel = true;
        break;
      default:
        addr_ss.seekg(-1, ios_base::cur);
        addr_ss >> offset;
        if (addr_ss.fail()) {
          parseError(L"Expected A, C, X, R, space or a number.");
        }
        if (!pushed_initial) {
          if (is_rel) {
            if (has_tok_addr) {
              parseError(L"Can't combine token addressing with relative "
                         L"wordoid addressing");
            }
            emitOpcode(VM::PUSHWRDADDR);
          } else if (0 <= offset) {
            emitOpcode(VM::PUSHZERO);
          } else {
            emitOpcode(VM::DUP);
            emitOpcode(VM::TOKLENWRD);
          }
          pushed_initial = true;
        }
        emitOpcode(VM::PUSHINT);
        emitInt(offset);
        break;
    }
  }
}
*/

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
  global_slot_counter = 0;
  stepToNextTag();
  while (type != XML_READER_TYPE_END_ELEMENT) {
    if (name == L"def-set") {
      procSetDef();
    } else if (name == L"def-str") {
      procStrDef();
    } else if (name == L"def-global") {
      procDefGlobal();
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
  if (name == L"sentlen") {
    emitOpcode(VM::SENTLENTOK);
    stepPastSelfClosingTag(L"sentlen");
  } else if (name == L"pathlen") {
    emitOpcode(VM::SENTLENWRD);
    stepPastSelfClosingTag(L"pathlen");
  } else if (name == L"tokaddr") {
    emitOpcode(VM::PUSHTOKADDR);
    stepPastSelfClosingTag(L"tokaddr");
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
  return true;
}

bool
MTXReader::procStrArrExpr(bool allow_fail)
{
  stepToTag();
  if (!tryProcVar(VM::STRARRVAL) && !tryProcSlice(&MTXReader::procStrArrExpr)) {
    if (name == L"ex-tags") {
      stepToNextTag();
      procWordoidExpr();
      assert(type == XML_READER_TYPE_END_ELEMENT);
      emitOpcode(VM::EXTAGS);
    } else if (name == L"for-each") {
      procForEach(Optional<VM::StackValueType>(VM::STRVAL));
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
    stepToNextTag();
    (this->*proc_inner)(false);
    bool exists;
    emitOpcode(VM::SUBSCRIPT);
    int idx = getInt("idx", exists);
    if (!exists) {
      emitInt(idx);
    }
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

    slot_names_it = global_slot_names.find(var_name);
    if (slot_names_it != global_slot_names.end()) {
      if (global_slot_types[slot_names_it->second] != svt) {
        parseError(L"Global variable " + var_name + L" has the wrong type");
      }
      emitOpcode(VM::GETGVAR);
      emitUInt(slot_names_it->second);
      stepPastSelfClosingTag(L"var");
      return true;
    }
    parseError(L"Variable " + var_name + L" has not been set.");
  }
  return false;
}

bool
MTXReader::procStrExpr(bool allow_fail)
{
  std::wcerr << "strexpr: " << name << "\n";
  if (!tryProcVar(VM::STRVAL) && !tryProcSlice(&MTXReader::procStrExpr)
                              && !tryProcSubscript(&MTXReader::procStrArrExpr)) {
    if (name == L"ex-surf") {
      stepToNextTag();
      procIntExpr();
      emitOpcode(VM::EXTOKSURF);
    } else if (name == L"ex-lemma") {
      stepToNextTag();
      procWordoidExpr();
      emitOpcode(VM::EXWRDLEMMA);
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
  if (name == L"and") {
    procCommBoolOp(VM::AND);
  } else if (name == L"or") {
    procCommBoolOp(VM::OR);
  } else if (name == L"not") {
    stepToNextTag();
    procBoolExpr();
    assert(type == XML_READER_TYPE_END_ELEMENT);
    emitOpcode(VM::NOT);
    step();
  } else if (name == L"lt") {
    procBinCompareOp(VM::LT);
  } else if (name == L"lte") {
    procBinCompareOp(VM::LTE);
  } else if (name == L"gt") {
    procBinCompareOp(VM::GT);
  } else if (name == L"gte") {
    procBinCompareOp(VM::GTE);
  } else if (name == L"streq") {
    stepToNextTag();
    procStrExpr();
    emitStrImmOp(VM::STREQ);
  } else if (name == L"strin") {
    stepToNextTag();
    procStrExpr();
    emitSetImmOp(VM::STRIN);
  } else if (name == L"sethas") {
    stepToNextTag();
    procStrExpr();
    emitSetImmOp(VM::SETHAS);
  } else if (name == L"sethasany") {
    stepToNextTag();
    procStrArrExpr();
    emitSetImmOp(VM::SETHASANY);
  } else if (name == L"sethasall") {
    stepToNextTag();
    procStrArrExpr();
    emitSetImmOp(VM::SETHASALL);
  } else {
    if (allow_fail) {
      return false;
    }
    parseError(L"Expected a boolean expression.");
  }
  stepToTag();
  assert(type == XML_READER_TYPE_END_ELEMENT);
  stepToTag();
  return true;
}

void
MTXReader::procAddrExpr()
{
  stepToTag();
  std::wcerr << "AYZ: " << name;
  /* Self-closing tags */
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

bool
MTXReader::procWordoidArrExpr(bool allow_fail)
{
  if (!tryProcVar(VM::WRDARRVAL) && !tryProcSlice(&MTXReader::procWordoidArrExpr)) {
    if (name == L"ex-wordoids") {
      stepToNextTag();
      procIntExpr();
      emitOpcode(VM::EXWRDARR);
      assert(name == L"ex-wordoids" && type == XML_READER_TYPE_END_ELEMENT);
    } else if (name == L"for-each") {
      procForEach(Optional<VM::StackValueType>(VM::WRDVAL));
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
  std::wcerr << "start wordoid\n";
  stepToTag();
  if (!tryProcVar(VM::WRDVAL)
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
  step();
  procBoolExpr();
  assert(type == XML_READER_TYPE_END_ELEMENT);
  step();
  emitOpcode(VM::DIEIFFALSE);
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
MTXReader::getStrRef(bool& exists)
{
  return getConstRef(L"name", "val", L"string", str_names, &MTXReader::pushStrConst, exists);
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
MTXReader::procForEach(Optional<VM::StackValueType> svt)
{
  std::wcerr << "Proc for-each\n";
  std::wstring var_name = attrib(L"as");
  if (var_name == L"") {
    parseError(L"'as' attribute required for for-each.");
  }
  std::wcerr << "Set var: " << var_name << "\n";
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

  if (!svt) {
    procVoidExpr();
  } else if (*svt == VM::STRVAL) {
    procStrExpr();
  } else if (*svt == VM::WRDVAL) {
    procWordoidExpr();
  } else {
    assert(false);
  }
  assert(type == XML_READER_TYPE_END_ELEMENT);

  size_t end_for_each_addr = cur_feat->size();
  emitOpcode(VM::ENDFOREACH);
  emitInt(end_for_each_addr + 2 - for_each_begin); // offset
  pokeBytecode(begin_offset_placeholder, (VM::Bytecode){.intbyte=end_for_each_addr + 2 - for_each_body_begin});
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
    procForEach(Optional<VM::StackValueType>());
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
MTXReader::procDefGlobal()
{
  slot_counter = 0;
  spec.global_defns.push_back(VM::FeatureDefn());
  cur_feat = &spec.global_defns.back();
  std::wstring var_name = attrib(L"as");
  if (var_name == L"") {
    parseError(L"'as' attribute required for for-each.");
  }
  global_slot_names[var_name] = global_slot_counter;
  stepToNextTag();

  bool has_expr = false;
  if (procIntExpr(true)) {
    global_slot_types.push_back(VM::INTVAL);
    has_expr = true;
  }
  if (!has_expr && procBoolExpr(true)) {
    global_slot_types.push_back(VM::BVAL);
    has_expr = true;
  }
  if (!has_expr && procStrExpr(true)) {
    global_slot_types.push_back(VM::STRVAL);
    has_expr = true;
  }
  if (!has_expr && procStrArrExpr(true)) {
    global_slot_types.push_back(VM::STRARRVAL);
    has_expr = true;
  }
  if (!has_expr && procWordoidArrExpr(true)) {
    global_slot_types.push_back(VM::WRDARRVAL);
    has_expr = true;
  }
  if (!has_expr && procWordoidExpr(true)) {
    global_slot_types.push_back(VM::WRDVAL);
    has_expr = true;
  }
  if (!has_expr) {
    parseError(L"Expected a non-void expression.");
  }
  assert(name == L"def-global" && type == XML_READER_TYPE_END_ELEMENT);
  stepToNextTag();

  global_slot_counter++;
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
  if (name == L"feats") {
    procFeats();
  }
  assert(name == L"metatag" && type == XML_READER_TYPE_END_ELEMENT);
}
}
