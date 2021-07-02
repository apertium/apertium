#ifndef _APERTIUM_TRANSFER_BASE_
#define _APERTIUM_TRANSFER_BASE_

#include <lttoolbox/ustring.h>
#include <lttoolbox/alphabet.h>
#include <lttoolbox/buffer.h>
#include <lttoolbox/match_exe.h>
#include <lttoolbox/match_state.h>

#include <apertium/apertium_re.h>
#include <apertium/transfer_instr.h>
#include <apertium/transfer_token.h>

#include <libxml/parser.h>
#include <libxml/tree.h>

#include <map>
#include <set>
#include <queue>
#include <vector>

using namespace std;

class TransferBase
{
protected:
  Alphabet alphabet;
  MatchExe* me;
  MatchState ms;
  map<UString, ApertiumRE> attr_items;
  map<UString, UString> variables;
  map<UString, UString> variable_defaults;
  map<UString, int> macros;
  map<UString, set<UString>> lists;
  map<UString, set<UString>> listslow;
  vector<xmlNode*> macro_map;
  vector<xmlNode*> rule_map;
  vector<size_t> rule_lines;
  xmlDoc* doc;
  xmlNode* root_element;

  queue<UString> blank_queue;
  Buffer<TransferToken> input_buffer;
  int lword;
  vector<UString*> tmpword;
  vector<UString*> tmpblank;
  xmlNode* lastrule;
  unsigned int nwords;

  UFILE* output;

  int32_t any_char;
  int32_t any_tag;

  bool in_let_var;
  bool in_out;
  UString var_val;
  map<xmlNode *, TransferInstr> evalStringCache;

  bool null_flush;
  bool internal_null_flush;
  bool trace;

  void collectMacros(xmlNode *localroot);
  void collectRules(xmlNode *localroot);

  bool gettingLemmaFromWord(const UString& attr);
  UString combineWblanks(const UString& first, const UString& second);
  UString copycase(const UString& src, const UString& dest);

  UString evalString(xmlNode* element);
  virtual UString evalCachedString(xmlNode* element) = 0;

  virtual void processClip(xmlNode* element) = 0;
  virtual void processBlank(xmlNode* element) = 0;
  virtual void processLuCount(xmlNode* element) = 0;
  virtual void processCaseOf(xmlNode* element) = 0;
  virtual UString processLu(xmlNode* element) = 0;
  virtual UString processMlu(xmlNode* element) = 0;
  virtual UString processChunk(xmlNode* element) = 0;

  int processRule(xmlNode* localroot);
  int processInstruction(xmlNode* localroot);
  int processRejectCurrentRule(xmlNode* localroot);
  int processChoose(xmlNode* localroot);
  void processAppend(xmlNode* localroot);

  virtual void processLet(xmlNode* localroot) = 0;
  virtual void processOut(xmlNode* localroot) = 0;
  virtual void processCallMacro(xmlNode* localroot) = 0;
  virtual void processModifyCase(xmlNode* localroot) = 0;

  bool processLogical(xmlNode *localroot);
  bool processTest(xmlNode *localroot);
  bool processAnd(xmlNode *localroot);
  bool processOr(xmlNode *localroot);
  bool processNot(xmlNode *localroot);

  bool beginsWith(const UString& haystack, const UString& needle);
  bool endsWith(const UString& haystack, const UString& needle);

  pair<xmlNode*, xmlNode*> twoChildren(xmlNode* localroot);

  bool processBeginsWith(xmlNode *localroot);
  bool processBeginsWithList(xmlNode *localroot);
  bool processEndsWith(xmlNode *localroot);
  bool processEndsWithList(xmlNode *localroot);
  bool processContainsSubstring(xmlNode *localroot);
  bool processEqual(xmlNode *localroot);
  bool processIn(xmlNode *localroot);

  UString tags(const UString& s) const;

public:
  TransferBase();
  ~TransferBase();

  void read(const char* transferfile, const char* datafile);
  bool getNullFlush(void);
  void setNullFlush(bool null_flush);
  void setTrace(bool trace);
};

#endif
