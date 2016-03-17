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
#ifndef _TRANSFER_
#define _TRANSFER_

#include <apertium/transfer_instr.h>
#include <apertium/transfer_token.h>
#include <apertium/transfer_word.h>
#include <apertium/apertium_re.h>
#include <lttoolbox/alphabet.h>
#include <lttoolbox/buffer.h>
#include <lttoolbox/fst_processor.h>
#include <lttoolbox/ltstr.h>
#include <lttoolbox/match_exe.h>
#include <lttoolbox/match_state.h>

#include <cstdio>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <map>
#include <set>
#include <vector>

using namespace std;

class Transfer
{
private:

  Alphabet alphabet;
  MatchExe *me;
  MatchState ms;
  map<string, ApertiumRE, Ltstr> attr_items;
  map<string, string, Ltstr> variables;
  map<string, int, Ltstr> macros;
  map<string, set<string, Ltstr>, Ltstr> lists;
  map<string, set<string, Ltstr>, Ltstr> listslow;
  vector<xmlNode *> macro_map;
  vector<xmlNode *> rule_map;
  xmlDoc *doc;
  xmlNode *root_element;
  TransferWord **word;
  string **blank;
  int lword, lblank;
  Buffer<TransferToken> input_buffer;
  vector<wstring *> tmpword;
  vector<wstring *> tmpblank;

  FSTProcessor fstp;
  FSTProcessor extended;
  bool isExtended;
  FILE *output;
  int any_char;
  int any_tag;

  xmlNode *lastrule;
  unsigned int nwords;

  map<xmlNode *, TransferInstr> evalStringCache;

  enum OutputType{lu,chunk};

  OutputType defaultAttrs;
  bool preBilingual;
  bool useBilingual;
  bool null_flush;
  bool internal_null_flush;
  bool trace;
  bool trace_att;
  string emptyblank;
  
  void destroy();
  void readData(FILE *input);
  void readBil(string const &filename);
  void readTransfer(string const &input);
  void collectMacros(xmlNode *localroot);
  void collectRules(xmlNode *localroot);
  string caseOf(string const &str);
  string copycase(string const &source_word, string const &target_word);

  void processLet(xmlNode *localroot);
  void processAppend(xmlNode *localroot);
  int processRejectCurrentRule(xmlNode *localroot);
  void processOut(xmlNode *localroot);
  void processCallMacro(xmlNode *localroot);
  void processModifyCase(xmlNode *localroot);
  bool processLogical(xmlNode *localroot);
  bool processTest(xmlNode *localroot);
  bool processAnd(xmlNode *localroot);
  bool processOr(xmlNode *localroot);
  bool processEqual(xmlNode *localroot);
  bool processBeginsWith(xmlNode *localroot);
  bool processBeginsWithList(xmlNode *localroot);
  bool processEndsWith(xmlNode *localroot);
  bool processEndsWithList(xmlNode *local);
  bool processContainsSubstring(xmlNode *localroot);
  bool processNot(xmlNode *localroot);
  bool processIn(xmlNode *localroot);
  int processRule(xmlNode *localroot);
  string evalString(xmlNode *localroot);
  int processInstruction(xmlNode *localroot);
  int processChoose(xmlNode *localroot);
  string processChunk(xmlNode *localroot);
  string processTags(xmlNode *localroot);

  bool beginsWith(string const &str1, string const &str2) const;
  bool endsWith(string const &str1, string const &str2) const;
  string tolower(string const &str) const;
  string tags(string const &str) const;
  wstring readWord(FILE *in);
  wstring readBlank(FILE *in);
  wstring readUntil(FILE *in, int const symbol) const;
  void applyWord(wstring const &word_str);
  int applyRule();
  TransferToken & readToken(FILE *in);
  bool checkIndex(xmlNode *element, int index, int limit);
  void transfer_wrapper_null_flush(FILE *in, FILE *out);
public:
  Transfer();
  ~Transfer();
  
  void read(string const &transferfile, string const &datafile,
	    string const &fstfile = "");
  void transfer(FILE *in, FILE *out);
  void setUseBilingual(bool value);
  bool getUseBilingual(void) const;
  void setPreBilingual(bool value);
  bool getPreBilingual(void) const;
  void setExtendedDictionary(string const &fstfile);
  void setCaseSensitiveness(bool value);
  bool getNullFlush(void);
  void setNullFlush(bool null_flush);
  void setTrace(bool trace);
  void setTraceATT(bool trace);
};

#endif
