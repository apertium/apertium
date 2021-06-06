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
#ifndef _POSTCHUNK_
#define _POSTCHUNK_

#include <apertium/transfer_instr.h>
#include <apertium/transfer_token.h>
#include <apertium/interchunk_word.h>
#include <apertium/apertium_re.h>
#include <lttoolbox/alphabet.h>
#include <lttoolbox/buffer.h>
#include <lttoolbox/ltstr.h>
#include <lttoolbox/match_exe.h>
#include <lttoolbox/match_state.h>

#include <cstdio>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <map>
#include <set>
#include <vector>
#include <queue>

using namespace std;

class Postchunk
{
private:

  Alphabet alphabet;
  MatchExe *me;
  MatchState ms;
  map<string, ApertiumRE> attr_items;
  map<string, string> variables;
  map<string, int> macros;
  map<string, set<string>> lists;
  map<string, set<string>> listslow;
  vector<xmlNode *> macro_map;
  vector<xmlNode *> rule_map;
  vector<size_t> rule_lines;
  xmlDoc *doc;
  xmlNode *root_element;
  InterchunkWord **word;
  queue <string> blank_queue;
  int lword;
  Buffer<TransferToken> input_buffer;
  vector<UString *> tmpword;
  vector<UString *> tmpblank;
  
  bool in_out;
  bool in_lu;
  bool in_let_var;
  string var_val;
  bool in_wblank;
  string out_wblank;
  map <string, string> var_out_wblank;

  UFILE *output;
  int any_char;
  int any_tag;

  xmlNode *lastrule;
  unsigned int nwords;

  map<xmlNode *, TransferInstr> evalStringCache;

  bool inword;
  bool null_flush;
  bool internal_null_flush;
  bool trace;

  void destroy();
  void readData(FILE *input);
  void readPostchunk(string const &input);
  void collectMacros(xmlNode *localroot);
  void collectRules(xmlNode *localroot);
  static string caseOf(string const &str);
  static UString caseOf(UString const &str);
  string copycase(string const &source_word, string const &target_word);

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
  string evalString(xmlNode *localroot);
  void processInstruction(xmlNode *localroot);
  void processChoose(xmlNode *localroot);
  void processTags(xmlNode *localroot);
  bool beginsWith(string const &str1, string const &str2) const;
  bool endsWith(string const &str1, string const &str2) const;
  string tolower(string const &str) const;
  string tags(string const &str) const;
  string readWord(InputFile& in);
  string readBlank(InputFile& in);
  string readUntil(InputFile& in, int const symbol) const;
  void applyWord(UString const &word_str);
  void applyRule();
  TransferToken & readToken(InputFile& in);
  static void unchunk(UString const &chunk, UFILE *output);
  static vector<UString> getVecTags(UString const &chunk);
  static int beginChunk(UString const &chunk);
  static int endChunk(UString const &chunk);
  static void splitWordsAndBlanks(UString const &chunk,
				  vector<UString *> &words,
				  vector<UString *> &blanks);
  static UString pseudolemma(UString const &chunk);
  static UString wordzero(UString const &chunk);
  bool checkIndex(xmlNode *element, int index, int limit);
  void postchunk_wrapper_null_flush(InputFile& in, UFILE* out);
  bool gettingLemmaFromWord(string attr);
  string combineWblanks(string wblank_current, string wblank_to_add);

public:
  Postchunk();
  ~Postchunk();

  void read(string const &transferfile, string const &datafile);
  void postchunk(InputFile& in, UFILE* out);
  bool getNullFlush(void);
  void setNullFlush(bool null_flush);
  void setTrace(bool trace);
};

#endif
