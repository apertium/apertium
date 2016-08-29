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
  cur_feat->push_back(bc);
}

void MTXReader::emitOpcode(VM::Opcode op)
{
  emitBytecode((VM::Bytecode){.op = op});
}

void MTXReader::emitInt(int val)
{
  assert(-128 <= val && val < 128);
  emitBytecode((VM::Bytecode){.intbyte = val});
}

void MTXReader::emitUInt(int val)
{
  assert(0 <= val && val < 256);
  emitBytecode((VM::Bytecode){.uintbyte = val});
}

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
    addr_ss.clear();
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
    addr_ss.clear();
  }
}

void MTXReader::procSetDef()
{
  std::wstring name = attrib(L"name");
  std::string val = attrib("val");
  set_names[name] = pushSetConst(val);
}

void MTXReader::procStrDef()
{
  std::wstring name = attrib(L"name");
  std::string val = attrib("val");
  str_names[name] = pushStrConst(val);
}


void
MTXReader::procDefns()
{
  while (type != XML_READER_TYPE_END_ELEMENT) {
    if (name == L"def-set") {
      procSetDef();
    } else if (name == L"def-str") {
      procStrDef();
    } else if (name == L"#text" || name == L"#comment") {
      // skip
    } else {
      unexpectedTag();
    }
    step();
  }
}

void
MTXReader::procBinOp(void (MTXReader::*proc_left)(void),
                     void (MTXReader::*proc_right)(void), VM::Opcode op)
{
  (this->*proc_left)();
  (this->*proc_right)();
  emitOpcode(op);
}

void
MTXReader::emitSetImmOp(VM::Opcode op)
{
  emitOpcode(op);
  emitSetRef();
}

void
MTXReader::emitStrImmOp(VM::Opcode op)
{
  emitOpcode(op);
  emitStrRef();
}

void
MTXReader::procBinCompareOp(VM::Opcode op)
{
  procBinOp(&MTXReader::procIntExpr, &MTXReader::procIntExpr, op);
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

void
MTXReader::procIntExpr()
{
  stepToTag();
  if (name == L"add") {
    emitInt();
    procIntExpr();
    emitOpcode(VM::ADI);
  } else if (name == L"sentlen") {
    emitOpcode(VM::SENTLENTOK);
  } else if (name == L"pathlen") {
    emitOpcode(VM::SENTLENWRD);
  } else if (name == L"toklen") {
    emitPushTokAddrFromAttr();
    emitOpcode(VM::TOKLENWRD);
  } else if (name == L"strlen") {
    procStrExpr();
    emitOpcode(VM::STRLEN);
  } else if (name == L"arrlen") {
    procStrArrExpr();
    procBinCompareOp(VM::ARRLEN);
  } else {
    parseError(L"Expected an integer expression.");
  }
  stepToTag();
  assert(type == XML_READER_TYPE_END_ELEMENT);
  stepToTag();
}

void
MTXReader::procStrArrExpr()
{
  stepToTag();
  if (name == L"gettags") {
    emitPushAddrFromAttr();
    emitOpcode(VM::GETTAGS);
  } else {
    parseError(L"Expected a string list expression.");
  }
  stepToTag();
  assert(type == XML_READER_TYPE_END_ELEMENT);
  stepToTag();
}

void
MTXReader::procStrExpr()
{
  stepToTag();
  if (name == L"slicebegin") {
    procStrExpr();
    emitOpcode(VM::SLICEBEGIN);
    emitInt();
  } else if (name == L"sliceend") {
    procStrExpr();
    emitOpcode(VM::SLICEEND);
    emitInt();
  } else if (name == L"gettoklf") {
    emitPushTokAddrFromAttr();
    emitOpcode(VM::GETTOKLF);
  } else if (name == L"getwrdlf") {
    emitPushAddrFromAttr();
    emitOpcode(VM::GETWRDLF);
  } else if (name == L"gettagsflat") {
    emitPushAddrFromAttr();
    emitOpcode(VM::GETTAGSFLAT);
  } else {
    parseError(L"Expected a string list expression.");
  }
  stepToTag();
  assert(type == XML_READER_TYPE_END_ELEMENT);
  stepToTag();
}

void
MTXReader::procBoolExpr()
{
  stepToTag();
  if (name == L"and") {
    procCommBoolOp(VM::AND);
  } else if (name == L"or") {
    procCommBoolOp(VM::OR);
  } else if (name == L"not") {
    procBoolExpr();
    emitOpcode(VM::NOT);
  } else if (name == L"lt") {
    procBinCompareOp(VM::LT);
  } else if (name == L"lte") {
    procBinCompareOp(VM::LTE);
  } else if (name == L"gt") {
    procBinCompareOp(VM::GT);
  } else if (name == L"gte") {
    procBinCompareOp(VM::GTE);
  } else if (name == L"streq") {
    procStrExpr();
    emitStrImmOp(VM::STREQ);
  } else if (name == L"strin") {
    procStrExpr();
    emitSetImmOp(VM::STRIN);
  } else if (name == L"sethas") {
    procStrExpr();
    emitSetImmOp(VM::SETHAS);
  } else if (name == L"sethasany") {
    procStrArrExpr();
    emitSetImmOp(VM::SETHASANY);
  } else if (name == L"sethasall") {
    procStrArrExpr();
    emitSetImmOp(VM::SETHASALL);
  } else {
    parseError(L"Expected a boolean expression.");
  }
  stepToTag();
  assert(type == XML_READER_TYPE_END_ELEMENT);
  stepToTag();
}

void
MTXReader::procPred()
{
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
    ConstNVMap &const_map,
    size_t (MTXReader::*push_new)(std::string&),
    bool& exists)
{
  std::wstring const_name = attrib(ref_attr);
  if (!const_name.empty()) {
    exists = true;
    ConstNVMap::iterator sit = const_map.find(const_name);
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
  return getConstRef(L"set", "set-lit", L"set", set_names, &MTXReader::pushSetConst, exists);
}

size_t
MTXReader::getStrRef(bool& exists)
{
  return getConstRef(L"str", "str-lit", L"string", str_names, &MTXReader::pushStrConst, exists);
}

int
MTXReader::getInt(bool& exists)
{
  std::string int_lit = attrib("int");
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

template<typename GetT, typename EmitT>
void
MTXReader::emitAttr(
    std::wstring what, GetT (MTXReader::*getter)(bool&), void (MTXReader::*emitter)(EmitT))
{
  bool has_attr;
  GetT val = (this->*getter)(has_attr);
  if (!has_attr) {
    parseError(what + L" required");
  }
  (this->*emitter)(val);
}

void
MTXReader::emitStrRef()
{
  emitAttr(L"String", &MTXReader::getStrRef, &MTXReader::emitUInt);
}

void
MTXReader::emitSetRef()
{
  emitAttr(L"Set", &MTXReader::getSetRef, &MTXReader::emitUInt);
}

void
MTXReader::emitInt()
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
MTXReader::procFeat()
{
  spec.features.push_back(VM::Feature());
  cur_feat = &spec.features.back();
  while (type != XML_READER_TYPE_END_ELEMENT) {
    if (name == L"pred") {
      procPred();
    } else if (name == L"inst") {
      procInst();
    } else if (name == L"#text" || name == L"#comment") {
      // skip
    } else {
      unexpectedTag();
    }
    step();
  }
}

void
MTXReader::procFeats()
{
  while (type != XML_READER_TYPE_END_ELEMENT) {
    if (name == L"feat") {
      procFeat();
    } else if (name == L"#text" || name == L"#comment") {
      // skip
    } else {
      unexpectedTag();
    }
    step();
  }
}

void
MTXReader::parse()
{
  bool processedFeats = false;
  while (type != XML_READER_TYPE_END_ELEMENT) {
    if (name == L"feats") {
      procFeats();
      processedFeats = true;
    } else if (name == L"defns") {
      if (processedFeats) {
        parseError(L"<defns> must preceed <feats>");
      }
      procDefns();
    } else if (name == L"#text" || name == L"#comment") {
      // skip
    } else {
      unexpectedTag();
    }
    step();
  }
}
}
