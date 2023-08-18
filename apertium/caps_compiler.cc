#include <apertium/caps_compiler.h>
#include <lttoolbox/compression.h>
#include <lttoolbox/string_utils.h>
#include <lttoolbox/xml_walk_util.h>
#include <limits>

const UString CapsCompiler::CAPS_COMPILER_CAPITALIZATION_ELEM = "capitalization"_u;
const UString CapsCompiler::CAPS_COMPILER_RULES_ELEM = "rules"_u;
const UString CapsCompiler::CAPS_COMPILER_RULE_ELEM = "rule"_u;
const UString CapsCompiler::CAPS_COMPILER_MATCH_ELEM = "match"_u;
const UString CapsCompiler::CAPS_COMPILER_OR_ELEM = "or"_u;
const UString CapsCompiler::CAPS_COMPILER_REPEAT_ELEM = "repeat"_u;
const UString CapsCompiler::CAPS_COMPILER_BEGIN_ELEM = "begin"_u;

const UString CapsCompiler::CAPS_COMPILER_USE_ATTR = "use"_u;
const UString CapsCompiler::CAPS_COMPILER_WEIGHT_ATTR = "weight"_u;
const UString CapsCompiler::CAPS_COMPILER_SELECT_ATTR = "select"_u;
const UString CapsCompiler::CAPS_COMPILER_LEMMA_ATTR = "lemma"_u;
const UString CapsCompiler::CAPS_COMPILER_TAGS_ATTR = "tags"_u;
const UString CapsCompiler::CAPS_COMPILER_SURFACE_ATTR = "surface"_u;
const UString CapsCompiler::CAPS_COMPILER_SRCSURF_ATTR = "srcsurf"_u;
const UString CapsCompiler::CAPS_COMPILER_TRGSURF_ATTR = "trgsurf"_u;
const UString CapsCompiler::CAPS_COMPILER_SRCLEM_ATTR = "srclem"_u;
const UString CapsCompiler::CAPS_COMPILER_TRGLEM_ATTR = "trglem"_u;
const UString CapsCompiler::CAPS_COMPILER_FROM_ATTR = "from"_u;
const UString CapsCompiler::CAPS_COMPILER_UPTO_ATTR = "upto"_u;

const UString CapsCompiler::CAPS_COMPILER_AA_VAL = "AA"_u;
const UString CapsCompiler::CAPS_COMPILER_Aa_VAL = "Aa"_u;
const UString CapsCompiler::CAPS_COMPILER_aa_VAL = "aa"_u;
const UString CapsCompiler::CAPS_COMPILER_DIX_VAL = "dix"_u;
const UString CapsCompiler::CAPS_COMPILER_LEFT_VAL = "left"_u;
const UString CapsCompiler::CAPS_COMPILER_RIGHT_VAL = "right"_u;

const UString CapsCompiler::CAPS_COMPILER_TYPE_AA = "<AA>"_u;
const UString CapsCompiler::CAPS_COMPILER_TYPE_Aa = "<Aa>"_u;
const UString CapsCompiler::CAPS_COMPILER_TYPE_aa = "<aa>"_u;
const UString CapsCompiler::CAPS_COMPILER_TYPE_DIX = "<dix>"_u;
const UString CapsCompiler::CAPS_COMPILER_TYPE_SKIP = "<skip>"_u;

const double  CapsCompiler::CAPS_COMPILER_DEFAULT_WEIGHT = 1.0;

CapsCompiler::CapsCompiler()
{
  alpha.includeSymbol(CAPS_COMPILER_TYPE_AA);
  alpha.includeSymbol(CAPS_COMPILER_TYPE_Aa);
  alpha.includeSymbol(CAPS_COMPILER_TYPE_aa);
  alpha.includeSymbol(CAPS_COMPILER_TYPE_DIX);
  alpha.includeSymbol(CAPS_COMPILER_TYPE_SKIP);
  alpha.includeSymbol("<ANY_TAG>"_u);
  alpha.includeSymbol("<ANY_CHAR>"_u);
  alpha.includeSymbol("<ANY_UPPER>"_u);
  alpha.includeSymbol("<ANY_LOWER>"_u);
  alpha.includeSymbol("<$>"_u);
  alpha.includeSymbol("<$$>"_u);

  any_tag        = alpha("<ANY_TAG>"_u);
  any_char       = alpha("<ANY_CHAR>"_u);
  any_upper      = alpha("<ANY_UPPER>"_u);
  any_lower      = alpha("<ANY_LOWER>"_u);
  word_boundary  = alpha(alpha("<$>"_u), alpha("<$>"_u));
  null_boundary  = alpha("<$$>"_u);
  AA_sym         = alpha(0, alpha(CAPS_COMPILER_TYPE_AA));
  Aa_sym         = alpha(0, alpha(CAPS_COMPILER_TYPE_Aa));
  aa_sym         = alpha(0, alpha(CAPS_COMPILER_TYPE_aa));
  dix_sym        = alpha(0, alpha(CAPS_COMPILER_TYPE_DIX));
  skip_sym       = alpha(0, alpha(CAPS_COMPILER_TYPE_SKIP));
}

CapsCompiler::~CapsCompiler()
{
}

UString
name(xmlNode* node)
{
  return to_ustring((const char*) node->name);
}

void
CapsCompiler::parse(const std::string& fname)
{
  xmlNode* root = load_xml(fname.c_str());
  if (name(root) == CAPS_COMPILER_CAPITALIZATION_ELEM) {
    for (auto ch : children(root)) {
      if (name(ch) == CAPS_COMPILER_RULES_ELEM) {
        for (auto rule : children(ch)) {
          if (name(rule) == CAPS_COMPILER_RULE_ELEM) {
            compile_rule(rule);
          }
        }
      }
    }
  }
  trans.minimize();
}

void
CapsCompiler::compile_rule(xmlNode* node)
{
  UString xweight = getattr(node, CAPS_COMPILER_WEIGHT_ATTR);
  double weight = CAPS_COMPILER_DEFAULT_WEIGHT;
  if (!xweight.empty()) {
    weight = StringUtils::stod(xweight);
    if(weight <= -std::numeric_limits<int>::max()) {
      weight = CAPS_COMPILER_DEFAULT_WEIGHT;
    }
  }
  int32_t state = trans.getInitial();
  state = trans.insertNewSingleTransduction(alpha(0, 0), state);
  UString ruleID = "<"_u + StringUtils::itoa(rule_weights.size()) + ">"_u;
  rule_weights.push_back(weight);
  for (auto ch : children(node)) {
    state = compile_node(ch, state);
  }
  state = trans.insertSingleTransduction(word_boundary, state);
  alpha.includeSymbol(ruleID);
  state = trans.insertSingleTransduction(alpha(0, alpha(ruleID)), state);
  trans.setFinal(state);
}

int32_t
CapsCompiler::compile_node(xmlNode* node, int32_t state)
{
  UString inner_name = name(node);
  if (inner_name == CAPS_COMPILER_MATCH_ELEM)
    return compile_match(node, state);
  else if (inner_name == CAPS_COMPILER_OR_ELEM)
    return compile_or(node, state);
  else if (inner_name == CAPS_COMPILER_REPEAT_ELEM)
    return compile_repeat(node, state);
  else if (inner_name == CAPS_COMPILER_BEGIN_ELEM)
    return trans.insertSingleTransduction(alpha(null_boundary, 0), state);
  else {
    error_and_die(node, "Unexpected tag <%S>", inner_name.c_str());
    return 0;
  }
}

int32_t
CapsCompiler::add_loop(int32_t sym, int32_t state)
{
  state = trans.insertSingleTransduction(alpha(sym, 0), state);
  trans.linkStates(state, state, alpha(sym, 0));
  return state;
}

int32_t
CapsCompiler::compile_caps_specifier(const UString& spec, int32_t state)
{
  for (auto& c : spec) {
    if (c == '*') {
      state = add_loop(any_char, state);
    } else if (c == ' ') {
      state = add_loop(' ', state);
    } else if (u_isupper(c)) {
      state = add_loop(any_upper, state);
    } else {
      state = add_loop(any_lower, state);
    }
  }
  return state;
}

int32_t
CapsCompiler::compile_match(xmlNode* node, int32_t state)
{
  UString lemma = getattr(node, CAPS_COMPILER_LEMMA_ATTR, "*"_u);
  UString tags = getattr(node, CAPS_COMPILER_TAGS_ATTR, "*"_u);
  UString surf = getattr(node, CAPS_COMPILER_SURFACE_ATTR, "*"_u);
  UString sscaps = getattr(node, CAPS_COMPILER_SRCSURF_ATTR, "*"_u);
  UString tscaps = getattr(node, CAPS_COMPILER_TRGSURF_ATTR, "*"_u);
  UString slcaps = getattr(node, CAPS_COMPILER_SRCLEM_ATTR, "*"_u);
  UString tlcaps = getattr(node, CAPS_COMPILER_TRGLEM_ATTR, "*"_u);
  UString select = getattr(node, CAPS_COMPILER_SELECT_ATTR);

  if (lemma != "*"_u && tlcaps != "*"_u) {
    I18n(APER_I18N_DATA, "apertium").error("APER1029", {"file_name", "line_number"},
                                                       {(char*)node->doc->URL, node->line}, true);
  }
  if (surf != "*"_u && tscaps != "*"_u) {
    I18n(APER_I18N_DATA, "apertium").error("APER1030", {"file_name", "line_number"},
                                                       {(char*)node->doc->URL, node->line}, true);
  }

  state = compile_caps_specifier(sscaps, state);
  state = trans.insertSingleTransduction(alpha('/', 0), state);
  state = compile_caps_specifier(slcaps, state);
  state = trans.insertSingleTransduction(alpha('/', 0), state);
  if (lemma == "*"_u) {
    state = compile_caps_specifier(tlcaps, state);
  } else {
    for (auto& c : lemma) {
      if (c == '*') {
        state = add_loop(any_char, state);
      } else {
        state = trans.insertSingleTransduction(alpha(c, 0), state);
      }
    }
  }
  auto tag_list = StringUtils::split_escaped(tags, '.');
  for (auto& it : tag_list) {
    if (it == "+"_u) {
      state = add_loop(any_tag, state);
    } else if (it == "*"_u) {
      state = trans.insertNewSingleTransduction(0, state);
      trans.linkStates(state, state, alpha(any_tag, 0));
    } else if (it == "?"_u) {
      state = trans.insertSingleTransduction(alpha(any_tag, 0), state);
    } else if (it.empty()) {
      continue;
    } else {
      UString tag = "<"_u + it + ">"_u;
      alpha.includeSymbol(tag);
      state = trans.insertSingleTransduction(alpha(alpha(tag), 0), state);
    }
  }
  state = trans.insertSingleTransduction(alpha('/', 0), state);
  if (surf == "*"_u) {
    state = compile_caps_specifier(tscaps, state);
  } else {
    for (auto& c : surf) {
      if (c == '*') {
        state = add_loop(any_char, state);
      } else {
        state = trans.insertSingleTransduction(alpha(c, 0), state);
      }
    }
  }

  state = trans.insertSingleTransduction(word_boundary, state);
  if (select.empty()) {
    state = trans.insertSingleTransduction(skip_sym, state);
  } else if (select == CAPS_COMPILER_AA_VAL) {
    state = trans.insertSingleTransduction(AA_sym, state);
  } else if (select == CAPS_COMPILER_Aa_VAL) {
    state = trans.insertSingleTransduction(Aa_sym, state);
  } else if (select == CAPS_COMPILER_aa_VAL) {
    state = trans.insertSingleTransduction(aa_sym, state);
  } else if (select == CAPS_COMPILER_DIX_VAL) {
    state = trans.insertSingleTransduction(dix_sym, state);
  } else {
    I18n(APER_I18N_DATA, "apertium").error("APER1031", {"file_name", "line_number", "select"},
                                                       {(char*)node->doc->URL, node->line, (char*)select.c_str()}, true);
  }

  return state;
}

int32_t
CapsCompiler::compile_or(xmlNode* node, int32_t start_state)
{
  int32_t end_state = start_state;
  for (auto ch : children(node)) {
    int32_t next = trans.insertNewSingleTransduction(0, start_state);
    next = compile_node(ch, next);
    if (end_state == start_state) end_state = next;
    else trans.linkStates(next, end_state, 0);
  }
  return end_state;
}

int32_t
CapsCompiler::compile_repeat(xmlNode* node, int32_t start_state)
{
  UString xfrom = getattr(node, CAPS_COMPILER_FROM_ATTR);
  UString xupto = getattr(node, CAPS_COMPILER_UPTO_ATTR);
  int from = StringUtils::stoi(xfrom);
  int upto = StringUtils::stoi(xupto);
  if(from < 0 || upto < 0) {
    I18n(APER_I18N_DATA, "apertium").error("APER1032", {"file_name", "line_number"},
                                                       {(char*)node->doc->URL, node->line}, true);
  } else if(from > upto) {
    I18n(APER_I18N_DATA, "apertium").error("APER1033", {"file_name", "line_number"},
                                                       {(char*)node->doc->URL, node->line}, true);
  }
  int count = upto - from;
  Transducer temp = trans;
  trans.clear();
  int32_t state = trans.getInitial();
  for (auto ch : children(node)) {
    state = compile_node(ch, state);
  }
  trans.setFinal(state);
  state = start_state;
  for (int i = 0; i < from; i++) {
    state = temp.insertTransducer(state, trans);
  }
  trans.optional();
  for (int i = 0; i < count; i++) {
    state = temp.insertTransducer(state, trans);
  }
  trans = temp;
  return state;
}

void
CapsCompiler::write(FILE* output)
{
  Compression::multibyte_write(0, output); // source caps mode
  alpha.write(output);
  trans.write(output);
  Compression::multibyte_write(rule_weights.size(), output);
  for (auto& it : rule_weights) {
    Compression::long_multibyte_write(it, output);
  }
}
