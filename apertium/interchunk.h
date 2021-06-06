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
#ifndef _INTERCHUNK_
#define _INTERCHUNK_

#include <apertium/transfer_instr.h>
#include <apertium/transfer_token.h>
#include <apertium/interchunk_word.h>
#include <apertium/apertium_re.h>
#include <lttoolbox/alphabet.h>
#include <lttoolbox/buffer.h>
#include <lttoolbox/input_file.h>
#include <lttoolbox/match_exe.h>
#include <lttoolbox/match_state.h>
#include <lttoolbox/ustring.h>

#include <cstring>
#include <cstdio>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <map>
#include <set>
#include <vector>
#include <queue>

using namespace std;

class Interchunk
{
private:

  Alphabet alphabet;
  MatchExe *me;
  MatchState ms;
  map<UString, ApertiumRE> attr_items;
  map<UString, UString> variables;
  map<UString, int> macros;
  map<UString, set<UString>> lists;
  map<UString, set<UString>> listslow;
  vector<xmlNode *> macro_map;
  vector<xmlNode *> rule_map;
  vector<size_t> rule_lines;
  xmlDoc *doc;
  xmlNode *root_element;
  InterchunkWord **word;
  queue <UString> blank_queue;
  int lword;
  int last_lword;
  Buffer<TransferToken> input_buffer;
  vector<UString *> tmpword;
  vector<UString *> tmpblank;

  UFILE* output;
  int any_char;
  int any_tag;

  xmlNode *lastrule;
  unsigned int nwords;

  map<xmlNode *, TransferInstr> evalStringCache;
  bool inword;
  bool null_flush;
  bool internal_null_flush;
  bool trace;
  bool in_out;

  void destroy();
  void readData(FILE *input);
  void readInterchunk(const char* input);
  void collectMacros(xmlNode *localroot);
  void collectRules(xmlNode *localroot);
  UString caseOf(UString const &str);
  UString copycase(UString const &source_word, UString const &target_word);

  void processLet(xmlNode *localroot);
  void processAppend(xmlNode *localroot);
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
  bool processEndsWithList(xmlNode *localroot);
  bool processContainsSubstring(xmlNode *localroot);
  bool processNot(xmlNode *localroot);
  bool processIn(xmlNode *localroot);
  void processRule(xmlNode *localroot);
  UString evalString(xmlNode *localroot);
  void processInstruction(xmlNode *localroot);
  void processChoose(xmlNode *localroot);
  UString processChunk(xmlNode *localroot);

  bool beginsWith(UString const &str1, UString const &str2) const;
  bool endsWith(UString const &str1, UString const &str2) const;
  UString tolower(UString const &str) const;
  UString tags(UString const &str) const;
  UString readWord(InputFile& in);
  UString readBlank(InputFile& in);
  UString readUntil(InputFile& in, int const symbol) const;
  void applyWord(UString const &word_str);
  void applyRule();
  TransferToken & readToken(InputFile& in);
  bool checkIndex(xmlNode *element, int index, int limit);
  void interchunk_wrapper_null_flush(InputFile& in, UFILE* out);

public:
  Interchunk();
  ~Interchunk();

  void read(const char* transferfile, const char* datafile);
  void interchunk(InputFile& in, UFILE* out);
  bool getNullFlush(void);
  void setNullFlush(bool null_flush);
  void setTrace(bool trace);
};

#endif
