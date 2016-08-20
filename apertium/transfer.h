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
#include <pcre.h>

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

  // all macros in xml in order of appearance in transfer file
  vector<xmlNode *> macro_map;

  // all rule actions in xml in order of appearance in transfer file
  vector<xmlNode *> rule_map;

  // rule number -> rule id, first meaningful rule at position 1
  vector<string> rule_ids;

  // rule id : rule number
  map<string, int> rule_id_map;

  // rule group number : rule ids
  vector<vector<string> > rule_groups; 

  // id : rule group number
  map<string, int> rule_group_map; 

  // all weighted patterns, grouped by rule group number
  // index of outer vector corresponds to rule group numbers
  // map is pattern string : vector of pairs <rule_id, weight>
  vector<map<string, vector<pair<string, double> > > > weighted_patterns;

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
  int lastrule_num;
  unsigned int nwords;

  map<xmlNode *, TransferInstr> evalStringCache;

  enum OutputType{lu,chunk};

  OutputType defaultAttrs;
  bool preBilingual;
  bool useBilingual;
  bool useWeights;
  bool null_flush;
  bool internal_null_flush;
  bool trace;
  bool trace_att;
  string emptyblank;
  
  void destroy();

  /**
    Read transfer rules from t*x xml file.
    In fact, only only default attribute value, macros, rule actions,
    and rule ids (if using weights) are read here.
  */
  void readTransfer(string const &input);

  /**
    Read transfer weights from w*x xml file.
  */
  void readTransferWeights(string const &in);

  /**
    Read macros from t*x xml file.
    localroot must point to 'section-macros' element in transfer xml tree.
  */
  void collectMacros(xmlNode *localroot);

  /**
    Read rule actions, and rule ids (if using weights) from t*x xml file.
    localroot must point to 'section-rules' element in transfer xml tree.
  */
  void collectRules(xmlNode *localroot);

  /**
    Get the value of 'id' attribute of 'rule' element.
    localrootand must point to 'rule' element in transfer xml tree.
  */
  string getRuleId(xmlNode *localroot);

  /**
    Get the value of attr_name attribute of xml tree element,
    which is pointed to by localroot.
  */
  string getNodeAttr(xmlNode *localroot, const char* attr_name);

  /**
    Read precompiled transfer rules from t*x.bin binary file.
  */
  void readData(FILE *input);

  /**
    Read data from bilingual letter transducer file if specified.
  */
  void readBil(string const &filename);

  string caseOf(string const &str);
  string copycase(string const &source_word, string const &target_word);

  /**
    Apply subelements of 'out' subelement of rule action, one subelement
    at a time, depending on subelement type.
  */
  void processOut(xmlNode *localroot);

  /**
    Apply various types of rule action subelements.
  */
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
  void processLet(xmlNode *localroot);
  void processAppend(xmlNode *localroot);
  int processRejectCurrentRule(xmlNode *localroot);

  /**
    Process instructions specified in previously stored 'action' part
    of the rule. localroot must point to an 'action' element of xml tree.
  */
  int processRule(xmlNode *localroot);

  /**
    Evaluate an xml element and execute appropriate instruction.
    Used for lowest-level action elements, such as 'clip' or 'lit-tag'.
  */
  string evalString(xmlNode *localroot);
  void evalStringClip(xmlNode *element, string &lemma, int &pos); // the dark horse

  /**
    Process instruction specified in rule action based on instruction name.
  */
  int processInstruction(xmlNode *localroot);
  int processChoose(xmlNode *localroot);

  /**
    Apply 'chunk' subelement of 'out' element of a rule,
    one subelement at a time, depending on subelement type.
  */
  string processChunk(xmlNode *localroot);
  string processTags(xmlNode *localroot);

  bool beginsWith(string const &str1, string const &str2) const;
  bool endsWith(string const &str1, string const &str2) const;
  string tolower(string const &str) const;
  string tags(string const &str) const;
  wstring readWord(FILE *in);
  wstring readBlank(FILE *in);
  wstring readUntil(FILE *in, int const symbol) const;

  /**
    Feed the token contained in word_str
    to internal FST by transiting over its states with ms.
  */
  void applyWord(wstring const &word_str);
  int applyRule();
  TransferToken & readToken(FILE *in);
  bool checkIndex(xmlNode *element, int index, int limit);
  void transfer_wrapper_null_flush(FILE *in, FILE *out);

public:
  Transfer();
  ~Transfer();
  
  /**
    Read all data needed for transfer
  */
  void read(string const &transferfile, string const &datafile, 
            string const &weightsfile = "", string const &fstfile = "");

  /**
    Perform transfer.
  */
  void transfer(FILE *in, FILE *out);

  /**
    Boilerplate for setting and getting values of private attributes.
  */
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
  void setUseWeights(bool weighted);
  bool getUseWeights(void) const;
};

#endif
