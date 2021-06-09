#include <apertium/transfer_base.h>
#include <apertium/xml_walk_util.h>
#include <apertium/string_utils.h>
#include <lttoolbox/compression.h>
#include <apertium/trx_reader.h>

using namespace Apertium;
using namespace std;

TransferBase::TransferBase()
  : me(nullptr), doc(nullptr), root_element(nullptr),
    lword(0), lastrule(nullptr), nwords(0), output(nullptr),
    any_char(0), any_tag(0), in_let_var(false), in_out(false),
    null_flush(false), internal_null_flush(false), trace(false)
{}

TransferBase::~TransferBase()
{
  if (me) {
    delete me;
    me = nullptr;
  }
  if (doc) {
    xmlFreeDoc(doc);
    doc = nullptr;
  }
}

void
TransferBase::read(const char* transferfile, const char* datafile)
{
  doc = xmlReadFile(transferfile, NULL, 0);
  if (doc == NULL) {
    cerr << "Error: Could not parse file '" << transferfile << "'." << endl;
    exit(EXIT_FAILURE);
  }
  root_element = xmlDocGetRootElement(doc);

  for (auto i : children(root_element)) {
    if (!xmlStrcmp(i->name, (const xmlChar*) "section-def-macros")) {
      collectMacros(i);
    } else if (!xmlStrcmp(i->name, (const xmlChar*) "section-rules")) {
      collectRules(i);
    }
  }


  FILE* in = fopen(datafile, "rb");
  if (!in) {
    cerr << "Error: Could not open file '" << datafile << "' for reading." << endl;
    exit(EXIT_FAILURE);
  }
  
  alphabet.read(in);
  any_char = alphabet(TRXReader::ANY_CHAR);
  any_tag = alphabet(TRXReader::ANY_TAG);

  Transducer t;
  t.read(in, alphabet.size());

  map<int, int> finals;

  // finals
  for(int i = 0, limit = Compression::multibyte_read(in); i != limit; i++)
  {
    int key = Compression::multibyte_read(in);
    finals[key] = Compression::multibyte_read(in);
  }

  me = new MatchExe(t, finals);

  // attr_items
  bool icu = Compression::string_read(in).empty();
  for(int i = 0, limit = Compression::multibyte_read(in); i != limit; i++)
  {
    UString const cad_k = Compression::string_read(in);
    attr_items[cad_k].read(in);
    UString fallback = Compression::string_read(in);
    if (!icu && cad_k == "chname"_u) {
      // chname was previously "({([^/]+)\\/)"
      // which is fine for PCRE, but ICU chokes on the unmatched bracket
      fallback = "(\\{([^/]+)\\/)"_u;
    }
    attr_items[cad_k].compile(fallback);
  }

  // variables
  for(int i = 0, limit = Compression::multibyte_read(in); i != limit; i++)
  {
    UString const cad_k = Compression::string_read(in);
    variables[cad_k] = Compression::string_read(in);
    variable_defaults[cad_k] = variables[cad_k];
  }

  // macros
  for(int i = 0, limit = Compression::multibyte_read(in); i != limit; i++)
  {
    UString const cad_k = Compression::string_read(in);
    macros[cad_k] = Compression::multibyte_read(in);
  }

  // lists
  for(int i = 0, limit = Compression::multibyte_read(in); i != limit; i++)
  {
    UString const cad_k = Compression::string_read(in);

    for(int j = 0, limit2 = Compression::multibyte_read(in); j != limit2; j++)
    {
      UString const cad_v = Compression::string_read(in);
      lists[cad_k].insert(cad_v);
      listslow[cad_k].insert(StringUtils::tolower(cad_v));
    }
  }
}

void
TransferBase::collectRules(xmlNode* localroot)
{
  for (auto rule : children(localroot)) {
    size_t line = rule->line;
    for (auto rulechild : children(rule)) {
      if(!xmlStrcmp(rulechild->name, (const xmlChar *) "action")) {
        rule_map.push_back(rulechild);
        rule_lines.push_back(line);
        break;
      }
    }
  }
}

void
TransferBase::collectMacros(xmlNode* localroot)
{
  for (auto i : children(localroot)) {
    macro_map.push_back(i);
  }
}

bool
TransferBase::gettingLemmaFromWord(const UString& attr)
{
  return attr == "lem"_u || attr == "lemh"_u || attr == "whole"_u;
}

UString
TransferBase::combineWblanks(const UString& first, const UString& second)
{
  if (first.empty()) {
    return second;
  } else if (second.empty()) {
    return first;
  }
  UString ret;
  ret.reserve(first.size() + second.size());
  if (endsWith(first, "]]"_u)) {
    if (first.size() > 2) {
      size_t i = first.size() - 3;
      bool esc = false;
      while (first[i] == '\\') {
        i--;
        esc = !esc;
      }
      if (esc) {
        ret.append(first);
      } else {
        ret.append(first.substr(0, first.size()-2));
      }
    } else {
      ret.append(first.substr(0, first.size()-2));
    }
  } else {
    ret.append(first);
  }
  ret += ';';
  ret += ' ';
  if (beginsWith(second, "[["_u)) {
    ret.append(second.substr(2));
  } else {
    ret.append(second);
  }
  return ret;
}

UString
TransferBase::evalString(xmlNode* element)
{
  if (!element) {
    throw "evalString() was called on a NULL element";
  }
  if (evalStringCache.find(element) != evalStringCache.end()) {
    return evalCachedString(element);
  }
  if (!xmlStrcmp(element->name, (const xmlChar*) "clip")) {
    processClip(element);
  } else if (!xmlStrcmp(element->name, (const xmlChar*) "lit-tag")) {
    evalStringCache[element] = TransferInstr(ti_lit_tag, tags(getattr(element, "v")), 0);
  } else if (!xmlStrcmp(element->name, (const xmlChar*) "lit")) {
    evalStringCache[element] = TransferInstr(ti_lit, getattr(element, "v"), 0);
  } else if (!xmlStrcmp(element->name, (const xmlChar*) "b")) {
    processBlank(element);
  } else if (!xmlStrcmp(element->name, (const xmlChar*) "get-case-from")) {
    int pos = atoi((const char*) element->properties->children->content);
    xmlNode* param = NULL;
    for (auto it : children(element)) {
      param = it;
      break;
    }
    evalStringCache[element] = TransferInstr(ti_get_case_from, "lem"_u, pos, param);
  } else if (!xmlStrcmp(element->name, (const xmlChar*) "var")) {
    evalStringCache[element] = TransferInstr(ti_var, getattr(element, "n"), 0);
  } else if (!xmlStrcmp(element->name, (const xmlChar*) "lu-count")) {
    processLuCount(element);
  } else if (!xmlStrcmp(element->name, (const xmlChar*) "case-of")) {
    processCaseOf(element);
  } else if (!xmlStrcmp(element->name, (const xmlChar*) "concat")) {
    UString value;
    for (auto it : children(element)) {
      value.append(evalString(it));
    }
    return value;
  } else if (!xmlStrcmp(element->name, (const xmlChar*) "lu")) {
    return processLu(element);
  } else if (!xmlStrcmp(element->name, (const xmlChar*) "mlu")) {
    return processMlu(element);
  } else if (!xmlStrcmp(element->name, (const xmlChar*) "chunk")) {
    return processChunk(element);
  } else {
    cerr << "Error: unexpected expression: '" << element->name << "'" << endl;
    exit(EXIT_FAILURE);
  }
  return evalCachedString(element);
}

int
TransferBase::processRule(xmlNode* localroot)
{
  int words_to_consume = -1;
  // iterating over the <action> tag
  for (auto i : children(localroot)) {
    words_to_consume = processInstruction(i);
    // When an instruction which modifies the number of words to be consumed
    // from the input is found, execution of the rule is stopped
    if (words_to_consume != -1) {
      break;
    }
  }
  // flush remaining non-space blanks
  while (!blank_queue.empty()) {
    if (blank_queue.front() != " "_u) {
      write(blank_queue.front(), output);
    }
    blank_queue.pop();
  }
  return words_to_consume;
}

int
TransferBase::processInstruction(xmlNode* localroot)
{
  int words_to_consume = -1;
  if(!xmlStrcmp(localroot->name, (const xmlChar *) "choose"))
  {
    words_to_consume = processChoose(localroot);
  }
  else if(!xmlStrcmp(localroot->name, (const xmlChar *) "let"))
  {
    processLet(localroot);
  }
  else if(!xmlStrcmp(localroot->name, (const xmlChar *) "append"))
  {
    processAppend(localroot);
  }
  else if(!xmlStrcmp(localroot->name, (const xmlChar *) "out"))
  {
    processOut(localroot);
  }
  else if(!xmlStrcmp(localroot->name, (const xmlChar *) "call-macro"))
  {
    processCallMacro(localroot);
  }
  else if(!xmlStrcmp(localroot->name, (const xmlChar *) "modify-case"))
  {
    processModifyCase(localroot);
  }
  else if(!xmlStrcmp(localroot->name, (const xmlChar *) "reject-current-rule"))
  {
    words_to_consume = processRejectCurrentRule(localroot);
  }
  return words_to_consume;
}

int
TransferBase::processRejectCurrentRule(xmlNode* localroot)
{
  bool shifting = (getattr(localroot, "shifting") == "yes"_u);
  return shifting ? 1 : 0;
}

int
TransferBase::processChoose(xmlNode* localroot)
{
  int words_to_consume = -1;
  for (auto option : children(localroot)) {
    if (!xmlStrcmp(option->name, (const xmlChar*) "when")) {
      bool picked = false;
      for (auto it : children(option)) {
        if (!xmlStrcmp(it->name, (const xmlChar*) "test")) {
          if (!processTest(it)) {
            break;
          } else {
            picked = true;
          }
        } else {
          words_to_consume = processInstruction(it);
          if (words_to_consume != -1) {
            return words_to_consume;
          }
        }
      }
      if (picked) {
        return words_to_consume;
      }
    } else if (!xmlStrcmp(option->name, (const xmlChar*) "otherwise")) {
      for (auto it : children(option)) {
        words_to_consume = processInstruction(it);
        if (words_to_consume != -1) {
          return words_to_consume;
        }
      }
    }
  }
  return words_to_consume;
}

void
TransferBase::processAppend(xmlNode* localroot)
{
  UString name = getattr(localroot, "n");
  for (auto i : children(localroot)) {
    in_let_var = true;
    var_val = name;
    variables[name].append(evalString(i));
    in_let_var = false;
  }
}

bool
TransferBase::processLogical(xmlNode *localroot)
{
  if(!xmlStrcmp(localroot->name, (const xmlChar *) "equal"))
  {
    return processEqual(localroot);
  }
  else if(!xmlStrcmp(localroot->name, (const xmlChar *) "begins-with"))
  {
    return processBeginsWith(localroot);
  }
  else if(!xmlStrcmp(localroot->name, (const xmlChar *) "begins-with-list"))
  {
    return processBeginsWithList(localroot);
  }
  else if(!xmlStrcmp(localroot->name, (const xmlChar *) "ends-with"))
  {
    return processEndsWith(localroot);
  }
  else if(!xmlStrcmp(localroot->name, (const xmlChar *) "ends-with-list"))
  {
    return processEndsWithList(localroot);
  }
  else if(!xmlStrcmp(localroot->name, (const xmlChar *) "contains-substring"))
  {
    return processContainsSubstring(localroot);
  }
  else if(!xmlStrcmp(localroot->name, (const xmlChar *) "or"))
  {
    return processOr(localroot);
  }
  else if(!xmlStrcmp(localroot->name, (const xmlChar *) "and"))
  {
    return processAnd(localroot);
  }
  else if(!xmlStrcmp(localroot->name, (const xmlChar *) "not"))
  {
    return processNot(localroot);
  }
  else if(!xmlStrcmp(localroot->name, (const xmlChar *) "in"))
  {
    return processIn(localroot);
  }

  return false;
}

bool
TransferBase::processTest(xmlNode* localroot)
{
  for (auto i : children(localroot)) {
    return processLogical(i);
  }
  return false;
}

bool
TransferBase::processAnd(xmlNode* localroot)
{
  for (auto i : children(localroot)) {
    if (!processLogical(i)) {
      return false;
    }
  }
  return true;
}

bool
TransferBase::processOr(xmlNode* localroot)
{
  for (auto i : children(localroot)) {
    if (processLogical(i)) {
      return true;
    }
  }
  return false;
}

bool
TransferBase::processNot(xmlNode* localroot)
{
  for (auto i : children(localroot)) {
    return !processLogical(i);
  }
  return false;
}

bool
TransferBase::beginsWith(const UString& haystack, const UString& needle)
{
  const size_t hlen = haystack.size();
  const size_t nlen = needle.size();
  if (hlen < nlen) {
    return false;
  }
  for (size_t i = 0; i < nlen; i++) {
    if (haystack[i] != needle[i]) {
      return false;
    }
  }
  return true;
}

bool
TransferBase::endsWith(const UString& haystack, const UString& needle)
{
  if (needle.size() > haystack.size()) {
    return false;
  }
  for (int h = haystack.size()-1, n = needle.size()-1; n >= 0; h--, n--) {
    if (haystack[h] != needle[n]) {
      return false;
    }
  }
  return true;
}

pair<xmlNode*, xmlNode*>
TransferBase::twoChildren(xmlNode* localroot)
{
  xmlNode* first = nullptr;
  xmlNode* second = nullptr;
  for (auto i : children(localroot)) {
    if (!first) {
      first = i;
    } else {
      second = i;
      break;
    }
  }
  return make_pair(first, second);
}

bool
TransferBase::processBeginsWith(xmlNode* localroot)
{
  auto ch = twoChildren(localroot);
  if (getattr(localroot, "caseless") == "yes"_u) {
    return beginsWith(StringUtils::tolower(evalString(ch.first)),
                      StringUtils::tolower(evalString(ch.second)));
  } else {
    return beginsWith(evalString(ch.first), evalString(ch.second));
  }
}

bool
TransferBase::processBeginsWithList(xmlNode* localroot)
{
  auto ch = twoChildren(localroot);
  UString needle = evalString(ch.first);
  UString idlist = getattr(ch.second, "n");
  bool caseless = (getattr(localroot, "caseless") == "yes"_u);
  if (caseless) {
    needle = StringUtils::tolower(needle);
  }
  for (auto it : (caseless ? listslow[idlist] : lists[idlist])) {
    if (beginsWith(needle, it)) {
      return true;
    }
  }
  return false;
}

bool
TransferBase::processEndsWith(xmlNode* localroot)
{
  auto ch = twoChildren(localroot);
  if (getattr(localroot, "caseless") == "yes"_u) {
    return endsWith(StringUtils::tolower(evalString(ch.first)),
                    StringUtils::tolower(evalString(ch.second)));
  } else {
    return endsWith(evalString(ch.first), evalString(ch.second));
  }
}

bool
TransferBase::processEndsWithList(xmlNode* localroot)
{
  auto ch = twoChildren(localroot);
  UString needle = evalString(ch.first);
  UString idlist = getattr(ch.second, "n");
  bool caseless = (getattr(localroot, "caseless") == "yes"_u);
  if (caseless) {
    needle = StringUtils::tolower(needle);
  }
  for (auto it : (caseless ? listslow[idlist] : lists[idlist])) {
    if (endsWith(needle, it)) {
      return true;
    }
  }
  return false;
}

bool
TransferBase::processContainsSubstring(xmlNode* localroot)
{
  auto ch = twoChildren(localroot);
  if (getattr(localroot, "caseless") == "yes"_u) {
    return StringUtils::tolower(evalString(ch.first)).find(StringUtils::tolower(evalString(ch.second))) != UString::npos;
  } else {
    return evalString(ch.first).find(evalString(ch.second)) != UString::npos;
  }
}

bool
TransferBase::processEqual(xmlNode* localroot)
{
  auto ch = twoChildren(localroot);
  if (getattr(localroot, "caseless") == "yes"_u) {
    return StringUtils::tolower(evalString(ch.first)) == StringUtils::tolower(evalString(ch.second));
  } else {
    return evalString(ch.first) == evalString(ch.second);
  }
}

bool
TransferBase::processIn(xmlNode* localroot)
{
  auto ch = twoChildren(localroot);
  UString sval = evalString(ch.first);
  UString idlist = getattr(ch.second, "n");
  if (getattr(localroot, "caseless") == "yes"_u) {
    set<UString>& myset = listslow[idlist];
    return (myset.find(StringUtils::tolower(sval)) != myset.end());
  } else {
    set<UString>& myset = lists[idlist];
    return (myset.find(sval) != myset.end());
  }
}

UString
TransferBase::tags(const UString& str) const
{
  UString ret;
  ret.reserve(str.size()+2);
  ret += '<';
  for (auto c : u16iter(str)) {
    if (c == '.') {
      ret += '>';
      ret += '<';
    } else {
      ret += c;
    }
  }
  ret += '>';
  return ret;
}

bool
TransferBase::getNullFlush(void)
{
  return null_flush;
}

void
TransferBase::setNullFlush(bool val)
{
  null_flush = val;
}

void
TransferBase::setTrace(bool val)
{
  trace = val;
}

