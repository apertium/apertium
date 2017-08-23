#include <apertium/perceptron_spec.h>
#include <apertium/utf_converter.h>
#include <apertium/deserialiser.h>
#include <apertium/serialiser.h>
#include <lttoolbox/match_state.h>
#include <iomanip>


namespace Apertium {

void PerceptronSpec::printFeature(std::wostream &out, const PerceptronSpec::FeatureDefn &feat_defn)
{
  ios::fmtflags orig_flags(out.flags());
  out << std::hex << std::setw(2) << std::setfill(L'0');
  for (size_t j = 0; j < feat_defn.size(); j++) {
     out << +feat_defn[j]  << " ";
  }
  out.flags(orig_flags);
  out << "\n";
  for (size_t j = 0; j < feat_defn.size(); j++) {
    if (feat_defn[j] < PerceptronSpec::num_opcodes) {
      out << PerceptronSpec::opcode_names[feat_defn[j]].c_str() << " ";
    } else {
      out << "XX ";
    }
  }
  out << "\n";
}

std::wostream &
operator<<(std::wostream &out, PerceptronSpec const &ps) {
  out << "= Global predicate =\n";
  PerceptronSpec::printFeature(out, ps.global_pred);
  out << "= Globals (" << ps.global_defns.size() << ") =\n";
  for (size_t i = 0; i < ps.global_defns.size(); i++) {
    out << " Global " << i << "\n";
    PerceptronSpec::printFeature(out, ps.global_defns[i]);
  }
  out << "= Features (" << ps.features.size() << ") =\n";
  for (size_t i = 0; i < ps.features.size(); i++) {
    out << " Feature " << i << "\n";
    PerceptronSpec::printFeature(out, ps.features[i]);
  }
  return out;
}

#define X(a) #a,
const std::string PerceptronSpec::opcode_names[] = {
  OPCODES
};
#undef X

const std::string PerceptronSpec::type_names[] = {
  "integer", "boolean", "string", "string array", "wordoid", "wordoid array"
};

static Morpheme make_sentinel_wordoid(
    const std::wstring &lemma_str,
    const std::wstring &tag_str) {
  Morpheme morpheme;
  morpheme.TheLemma = lemma_str;
  Tag tag;
  tag.TheTag = tag_str;
  morpheme.TheTags.push_back(tag);
  return morpheme;
}

static std::vector<Morpheme> make_sentinel_wordoids(
    const std::wstring &lemma_str,
    const std::wstring &tag_str) {
  std::vector<Morpheme> morphemes;
  morphemes.push_back(make_sentinel_wordoid(lemma_str, tag_str));
  return morphemes;
}

static LexicalUnit make_sentinel_token(
    const std::wstring &surf,
    const std::wstring &lemma_str,
    const std::wstring &tag_str) {
  Analysis analy;
  analy.TheMorphemes = make_sentinel_wordoids(lemma_str, tag_str);
  LexicalUnit lu;
  lu.TheSurfaceForm = surf;
  lu.TheAnalyses.push_back(analy);
  return lu;
}

PerceptronSpec::PerceptronSpec() {
  if (!static_constructed) {
    num_opcodes = sizeof(opcode_names) / sizeof(opcode_names[0]);
    for (size_t i=0; i < num_opcodes; i++) {
      opcode_values[opcode_names[i]] = (Opcode)i;
    }

    untagged_sentinel = make_sentinel_wordoids(L"!UNTAGGED!", L"!UT!");
    token_wordoids_underflow = make_sentinel_token(L"!SURF_UNDERFLOW!", L"!TOK_UNDERFLOW!", L"!TUF!");
    token_wordoids_overflow = make_sentinel_token(L"!SURF_OVERFLOW!", L"!TOK_OVERFLOW!", L"!TOF!");

    static_constructed = true;
  }
}

unsigned char PerceptronSpec::num_opcodes;
bool PerceptronSpec::static_constructed = false;
std::map<const std::string, PerceptronSpec::Opcode>
PerceptronSpec::opcode_values;
std::vector<Morpheme> PerceptronSpec::untagged_sentinel;
LexicalUnit PerceptronSpec::token_wordoids_underflow;
LexicalUnit PerceptronSpec::token_wordoids_overflow;

void 
PerceptronSpec::get_features(
    const TaggedSentence &tagged, const Sentence &untagged,
    int token_idx, int wordoid_idx,
    UnaryFeatureVec &feat_vec_out) const 
{
  size_t i;
  global_results.clear();
  if (global_pred.size() > 0) 
  {
    Machine machine(
      *this, global_pred, 0, false,
      tagged, untagged, token_idx, wordoid_idx);
    StackValue result = machine.getValue();
    assert(result.type == BVAL);
    if (!result.boolVal()) 
    {
      return;
    }
  }
  for (i = 0; i < global_defns.size(); i++) {
    Machine machine(
      *this, global_defns[i], i, false,
      tagged, untagged, token_idx, wordoid_idx);
    global_results.push_back(machine.getValue());
  }
  std::vector<FeatureDefn>::const_iterator feat_it;
  UnaryFeatureVec feat_vec_delta;
  for (i = 0; i < features.size(); i++) {
    //feat_it = features.begin(); feat_it != features.end(); feat_it++) {
    feat_vec_delta.clear();
    feat_vec_delta.push_back(FeatureKey());
    FeatureKey &fk = feat_vec_delta.back();
    std::string prg_id;
    prg_id = i;
    fk.push_back(prg_id); // Each feature is tagged with the <feat> which created it to avoid collisions
    Machine machine(
      *this, features[i], i, true,
      tagged, untagged, token_idx, wordoid_idx);
    machine.getFeature(feat_vec_delta);
    feat_vec_out.insert(feat_vec_out.end(),
                        feat_vec_delta.begin(), feat_vec_delta.end());
  }
}

std::string
PerceptronSpec::coarsen(const Morpheme &wrd) const
{
  std::map<const Morpheme, std::string>::const_iterator it = coarsen_cache.find(wrd);
  if (it == coarsen_cache.end()) {
    std::string coarse_tag = UtfConverter::toUtf8(coarse_tags->coarsen(wrd));
    coarsen_cache[wrd] = coarse_tag;
    return coarse_tag;
  }
  return it->second;
}

void PerceptronSpec::clearCache() const
{
  coarsen_cache.clear();
}

std::string PerceptronSpec::dot = ".";

const std::string&
PerceptronSpec::Machine::get_str_operand() {
  size_t idx = *(++bytecode_iter);
  if (idx == 255) {
    return dot;
  }
  return spec.str_consts[idx];
}

const VMSet&
PerceptronSpec::Machine::get_set_operand() {
  return spec.set_consts[*(++bytecode_iter)];
}

int
PerceptronSpec::Machine::get_int_operand() {
  return (Bytecode){.uintbyte=*(++bytecode_iter)}.intbyte;
}

unsigned int
PerceptronSpec::Machine::get_uint_operand() {
  return (Bytecode){.uintbyte=*(++bytecode_iter)}.uintbyte;
}

const LexicalUnit&
PerceptronSpec::Machine::get_token(const Sentence &untagged) {
  int target_token_idx = stack.pop_off().intVal();
  if (target_token_idx < 0) {
    return token_wordoids_underflow;
  }
  if ((size_t)target_token_idx >= untagged.size()) {
    return token_wordoids_overflow;
  }
  return *untagged[target_token_idx].TheLexicalUnit;
}

const std::vector<Morpheme>&
PerceptronSpec::Machine::tagged_to_wordoids(const TaggedToken &tt) {
  // Replaces empty analyses with a sentinel value
  if (!tt) {
    return untagged_sentinel;
  }
  return tt->TheMorphemes;
}

const Morpheme&
PerceptronSpec::Machine::get_wordoid(const TaggedSentence &tagged) {
  int target_wordoid_idx = stack.pop_off().intVal();
  int target_token_idx = stack.pop_off().intVal();
  if (target_token_idx < 0) {
    return token_wordoids_underflow.TheAnalyses[0].TheMorphemes[0];
  }
  if ((size_t)target_token_idx >= tagged.size()) {
    return token_wordoids_overflow.TheAnalyses[0].TheMorphemes[0];
  }

  const vector<Morpheme> &wordoids = tagged_to_wordoids(tagged[target_token_idx]);
  if (target_wordoid_idx < 0) {
    return token_wordoids_underflow.TheAnalyses[0].TheMorphemes[0];
  }
  if ((size_t)target_wordoid_idx >= wordoids.size()) {
    return token_wordoids_overflow.TheAnalyses[0].TheMorphemes[0];
  }

  return wordoids[target_wordoid_idx];
}

PerceptronSpec::Machine::Machine(
    const PerceptronSpec &spec,
    const FeatureDefn &feat,
    size_t feat_idx,
    bool is_feature,
    const TaggedSentence &tagged,
    const Sentence &untagged,
    int token_idx,
    int wordoid_idx
    )
  : spec(spec), is_feature(is_feature), feat(feat), feat_idx(feat_idx),
    bytecode_iter(feat.begin()), tagged(tagged), untagged(untagged),
    token_idx(token_idx), wordoid_idx(wordoid_idx) {}


static bool
inRange(int lower, int upper, int x) {
  return lower <= x && x < upper;
}

static int
clamp(int lower, int upper, int x) {
  return std::min(std::max(x, lower), upper);
}

template <typename T> static void
slice(T &vec, int begin, int end) {
  if (begin < 0) {
    begin = vec.size() + begin;
  }
  if (end <= 0) {
    end = vec.size() + end;
  }
  begin = clamp(0, vec.size() - 1, begin);
  end = clamp(0, vec.size(), end);
  vec.assign(vec.begin() + begin, vec.begin() + end);
}

template <typename T> static T
subscript(std::vector<T> vec, int idx) {
  if (idx < 0) {
    idx = vec.size() - idx;
  }
  idx = clamp(0, vec.size() - 1, idx);
  return vec[idx];
}

void
PerceptronSpec::Machine::traceMachineState()
{
  std::wcerr << "pc: " << bytecode_iter - feat.begin() << "\n";
  std::wcerr << "peek: ";
  std::wcerr << *bytecode_iter;
  if (*bytecode_iter < num_opcodes) {
    std::wcerr << " (" << opcode_names[*bytecode_iter].c_str() << ")";
  }
  std::wcerr << "\n";
  std::wcerr << "stack: " << stack << "\n";
}

bool
PerceptronSpec::Machine::execCommonOp(Opcode op)
{
  //traceMachineState();
  switch (op) {
    case OR:
      stack.push(stack.pop_off().boolVal() || stack.pop_off().boolVal());
      break;
    case AND:
      stack.push(stack.pop_off().boolVal() && stack.pop_off().boolVal());
      break;
    case NOT:
      stack.push(!stack.pop_off().boolVal());
      break;
    case ADI:
      stack.push(stack.pop_off().intVal() + get_int_operand());
      break;
    case ADD:
      stack.push(stack.pop_off().intVal() + stack.pop_off().intVal());
      break;
    case ADD2: {
      int b2 = stack.pop_off().intVal();
      int b1 = stack.pop_off().intVal();
      int a2 = stack.pop_off().intVal();
      int a1 = stack.pop_off().intVal();
      stack.push(a1 + b1);
      stack.push(a2 + b2);
    } break;
    case PUSHINT:
      stack.push(get_int_operand());
      break;
    case LT:
      stack.push(stack.pop_off().intVal() < stack.pop_off().intVal());
      break;
    case LTE:
      stack.push(stack.pop_off().intVal() <= stack.pop_off().intVal());
      break;
    case GT:
      stack.push(stack.pop_off().intVal() > stack.pop_off().intVal());
      break;
    case GTE:
      stack.push(stack.pop_off().intVal() >= stack.pop_off().intVal());
      break;
    case EQ:
      stack.push(stack.pop_off().intVal() == stack.pop_off().intVal());
      break;
    case NEQ:
      stack.push(stack.pop_off().intVal() != stack.pop_off().intVal());
      break;
    case DUP:
      stack.push(stack.top());
      break;
    case DUP2: {
      StackValue b = stack.pop_off();
      StackValue a = stack.pop_off();
      stack.push(a);
      stack.push(b);
      stack.push(a);
      stack.push(b);
    } break;
    case FOREACHINIT: {
      loop_stack.push_back((LoopState){
          .initial_stack=stack.size() - 1,
          .iterable=stack.pop_off(),
          .iteration=0,
          .accumulator=StackValue(0)});
    } break;
    case FOREACH: {
      //std::wcerr << "size: " << loop_stack.back().iterable.size()
                 //<< " iteration: " << loop_stack.back().iteration << "\n";
      //std::wcerr << "foreach pc: " << bytecode_iter - feat.begin() << "\n";
      size_t slot = get_uint_operand();
      size_t end_offset = get_uint_operand();
      //std::wcerr << "after foreach pc: " << bytecode_iter - feat.begin() << "\n";
      if (loop_stack.back().iteration == loop_stack.back().iterable.size()) {
        stack.push(loop_stack.back().accumulator);
        loop_stack.pop_back();
        bytecode_iter += end_offset;
        break;
      }
      if (slots.size() <= slot) {
        slots.resize(slot + 1);
      }
      slots[slot] = loop_stack.back().iterable[loop_stack.back().iteration];
    } break;
    case ENDFOREACH: {
      // Is a map (produces vector)
      LoopState &loop_state = loop_stack.back();
      if (stack.size() > loop_state.initial_stack) {
        // First iteration
        if (loop_state.iteration == 0) {
          if (stack.top().type == WRDVAL) {
            loop_state.accumulator = StackValue(std::vector<Morpheme>());
            //std::wcerr << "Wordoid array size " << loop_state.iterable.size() << "\n";
          } else if (stack.top().type == STRVAL) {
            loop_state.accumulator = StackValue(std::vector<std::string>());
            //std::wcerr << "String array size " << loop_state.iterable.size() << "\n";
          } else {
            assert(false);
          }
        }
        if (stack.top().type == WRDVAL) {
          loop_state.accumulator.wrdArr().push_back(stack.top().wrd());
        } else if (stack.top().type == STRVAL) {
          //std::wcerr << "String array size " << loop_state.accumulator.size() << "\n";
          loop_state.accumulator.strArr().push_back(stack.top().str());
        } else {
          assert(false);
        }
        stack.pop();
        loop_state.iteration++;
      }
      size_t begin_offset = get_uint_operand();
      bytecode_iter -= begin_offset;
    } break;
    case GETGVAR: {
      int slot = get_uint_operand();
      //std::wcerr << "GETGVAR " << slot << " " << spec.global_results[slot] << "\n";
      stack.push(spec.global_results[slot]);
    } break;
    case GETVAR: {
      int slot = get_uint_operand();
      stack.push(slots[slot]);
    } break;
    case STREQ:
      stack.push(get_str_operand() == stack.pop_off().str());
      break;
    case STRIN: {
      stack.push(In(get_set_operand())(stack.pop_off().str()));
    } break;
    case PUSHTOKADDR:
      stack.push(token_idx);
      break;
    case PUSHWRDADDR:
      stack.push(wordoid_idx);
      break;
    case PUSHADDR:
      stack.push(token_idx);
      stack.push(wordoid_idx);
      break;
    case ADJADDR: {
      int wordoid_idx = stack.pop_off().intVal();
      int token_idx = stack.pop_off().intVal();
      int token_len = tagged.size();
      int wordoid_len;
      if (0 <= token_idx && token_idx < token_len) {
        while (wordoid_idx < 0 && token_idx > 0) {
          token_idx--;
          wordoid_len = tagged_to_wordoids(tagged[token_idx]).size();
          wordoid_idx += wordoid_len;
        }
        wordoid_len = tagged_to_wordoids(tagged[token_idx]).size();
        while (wordoid_idx >= wordoid_len && token_idx < (token_len - 1)) {
          token_idx++;
          wordoid_idx -= wordoid_len;
          wordoid_len = tagged_to_wordoids(tagged[token_idx]).size();
        }
      }
      stack.push(token_idx);
      stack.push(wordoid_idx);
    } break;
    case CLAMPADDR: {
      int wordoid_idx = stack.pop_off().intVal();
      int token_idx = stack.pop_off().intVal();
      token_idx = clamp(0, (int)tagged.size() - 1, token_idx);
      int wordoid_len = tagged_to_wordoids(tagged[token_idx]).size();
      wordoid_idx = clamp(0, wordoid_len - 1, wordoid_idx);
      stack.push(token_idx);
      stack.push(wordoid_idx);
    } break;
    case CLAMPTAGGEDTOKADDR:
      stack.push(clamp(0, (int)tagged.size() - 1, stack.pop_off().intVal()));
      break;
    case CLAMPTOKADDR:
      stack.push(clamp(0, (int)untagged.size() - 1, stack.pop_off().intVal()));
      break;
    case GETWRD: {
      //std::wcerr << "GETWRD start\n";
      stack.push(get_wordoid(tagged));
      //std::wcerr << "GETWRD done\n";
    } break;
    case EXTOKSURF: {
      std::wstring surf = get_token(untagged).TheSurfaceForm;
      stack.push(new std::string(UtfConverter::toUtf8(surf)));
    } break;
    case EXWRDLEMMA: {
      std::wstring lemma = stack.pop_off().wrd().TheLemma;
      stack.push(new std::string(UtfConverter::toUtf8(lemma)));
    } break;
    case EXWRDCOARSETAG: {
      assert(spec.coarse_tags);
      Morpheme &wrd = stack.top().wrd();
      std::string coarse_tag = spec.coarsen(wrd);
      stack.pop();
      stack.push(coarse_tag);
    } break;
    case EXAMBGSET: {
      assert(spec.coarse_tags);
      std::vector<std::string> ambgset;
      const std::vector<Analysis> &analyses = get_token(untagged).TheAnalyses;
      std::vector<Analysis>::const_iterator analy_it;
      for (analy_it = analyses.begin(); analy_it != analyses.end(); analy_it++) {
        ambgset.push_back(std::string());
        const std::vector<Morpheme> &wrds = analy_it->TheMorphemes;
        std::vector<Morpheme>::const_iterator wrd_it = wrds.begin();
        while (true) {
          ambgset.back() += spec.coarsen(*wrd_it);
          wrd_it++;
          if (wrd_it == wrds.end()) {
            break;
          } else {
            ambgset.back() += "+";
          }
        }
      }
      stack.push(ambgset);
    } break;
    case EXTAGS: {
      const std::vector<Tag> &tags = stack.top().wrd().TheTags;
      /*std::vector<Tag>::const_iterator it = tags.begin();
      std::wcerr << "tags: ";
      for (;it != tags.end(); it++) {
        std::wcerr << &(*it) << " " << it->TheTag << ", ";
      }
      std::wcerr << "\n";*/
      std::vector<std::string> *tags_str = new std::vector<std::string>;
      tags_str->resize(tags.size());
      transform(tags.begin(), tags.end(), tags_str->begin(), get_tag);
      stack.pop();
      stack.push(tags_str);
    } break;
    case SENTLENTOK:
      stack.push((int)untagged.size());
      break;
    case SENTLENTAGGEDTOK:
      stack.push((int)tagged.size());
      break;
    case SENTLENWRD: unimplemented_opcode("SENTLENWRD"); break; // How can we know?
    case TOKLENWRD: {
      int target_token_idx = stack.pop_off().intVal();
      assert(0 <= target_token_idx && (size_t)target_token_idx < tagged.size());
      stack.push((int)tagged_to_wordoids(tagged[target_token_idx]).size());
    } break;
    case ISVALIDTOKADDR: {
      int token_idx = stack.pop_off().intVal();
      stack.push((bool)(0 <= token_idx && (size_t)token_idx < untagged.size()));
    } break;
    case ISVALIDTAGGEDTOKADDR: {
      int token_idx = stack.pop_off().intVal();
      stack.push((bool)(0 <= token_idx && (size_t)token_idx < tagged.size()));
    } break;
    case ISVALIDADDR: {
      int wordoid_idx = stack.pop_off().intVal();
      int token_idx = stack.pop_off().intVal();
      bool tokaddr_valid = 0 <= token_idx && (size_t)token_idx < tagged.size();
      if (!tokaddr_valid) {
        stack.push(false);
        break;
      }
      int wordoid_len = tagged_to_wordoids(tagged[token_idx]).size();
      stack.push(0 <= wordoid_idx && wordoid_idx < wordoid_len);
    } break;
    case EXWRDARR: {
      int token_idx = stack.pop_off().intVal();
      if (token_idx < 0) {
        stack.push(token_wordoids_underflow.TheAnalyses[0].TheMorphemes);
      } else if ((size_t)token_idx >= tagged.size()) {
        stack.push(token_wordoids_overflow.TheAnalyses[0].TheMorphemes);
      } else {
        stack.push(tagged_to_wordoids(tagged[token_idx]));
      }
    } break;
    case FILTERIN: {
      const VMSet& set_op = get_set_operand();
      std::vector<std::string> &str_arr = stack.top().strArr();
      str_arr.erase(std::remove_if(
          str_arr.begin(), str_arr.end(), std::not1(In(set_op))));
    } break;
    /*
    case SETHAS: {
      const VMSet& set_op = get_set_operand();
      std::string str = stack.pop_off().str();
      stack.push(set_op.find(str) != set_op.end());
    } break;
    */
    case SETHASANY: {
      const VMSet& set_op = get_set_operand();
      std::vector<std::string> str_arr = stack.pop_off().strArr();
      stack.push(
        std::find_if(str_arr.begin(), str_arr.end(), In(set_op)) !=
        str_arr.end()
      );
    } break;
    case SETHASALL: {
      const VMSet& set_op = get_set_operand();
      std::vector<std::string> str_arr = stack.pop_off().strArr();
      stack.push(
        std::find_if(str_arr.begin(), str_arr.end(), std::not1(In(set_op))) ==
        str_arr.end()
      );
    } break;
    case HASSUBSTR: {
      std::string haystack = stack.pop_off().str();
      std::string needle = get_str_operand();
      stack.push(haystack.find(needle) != std::string::npos);
    } break;
    case HASANYSUBSTR: unimplemented_opcode("HASANYSUBSTR"); break;
    case CPYSTR: unimplemented_opcode("CPYSTR"); break;
    case LOWER: {
      // XXX: Eek! Bad! No Unicode. ICU please.
      std::string &str = stack.top().str();
      std::transform(str.begin(), str.end(), str.begin(), ::tolower);
    } break;
    case SLICE: {
      int begin = get_int_operand();
      int end = get_int_operand();
      if (stack.top().type == STRVAL) {
        slice(stack.top().str(), begin, end);
      } else if (stack.top().type == STRARRVAL) {
        slice(stack.top().strArr(), begin, end);
      } else if (stack.top().type == WRDARRVAL) {
        slice(stack.top().wrdArr(), begin, end);
      }
    } break;
    case SUBSCRIPT: {
      size_t idx = get_uint_operand();
      if (stack.top().type == STRARRVAL) {
        stack.push(subscript(stack.pop_off().strArr(), idx));
      } else if (stack.top().type == WRDARRVAL) {
        stack.push(subscript(stack.pop_off().wrdArr(), idx));
      }
    } break;
    case STRLEN: {
      std::string str = stack.pop_off().str();
      stack.push((int)str.length());
    } break;
    case ARRLEN: {
      int str_arr_len = stack.pop_off().strArr().size();
      stack.push(str_arr_len);
    } break;
    case JOIN: {
      const std::string &sep = get_str_operand();
      std::stringstream ss;
      std::vector<std::string> str_arr = stack.pop_off().strArr();
      std::vector<std::string>::const_iterator it;
      for (it = str_arr.begin(); it != str_arr.end(); it++) {
        ss << *it;
        if (it + 1 != str_arr.end()) {
          ss << sep;
        }
      }
      stack.push(StackValue(ss.str()));
    } break;
    default:
      return false;
  }
  return true;
}

void
PerceptronSpec::Machine::getFeature(
    UnaryFeatureVec &feat_vec_out) 
{
  for (; bytecode_iter != feat.end(); bytecode_iter++) {
    Opcode op = (Bytecode){.intbyte=static_cast<signed char>(*bytecode_iter)}.op;
    if (execCommonOp(op)) {
      continue;
    }
    switch ((Bytecode){.intbyte=static_cast<signed char>(*bytecode_iter)}.op) {
      case DIEIFFALSE:
        if (!stack.pop_off().boolVal()) {
          feat_vec_out.clear();
          return;
        }
        break;
      case FCATSTRARR: {
        std::vector<std::string> &str_arr = stack.top().strArr();
        if (str_arr.size() == 0) {
          feat_vec_out.clear();
          return;
        } else {
          UnaryFeatureVec new_feat_vec;
          new_feat_vec.reserve(feat_vec_out.size() * str_arr.size());
          std::vector<std::string>::const_iterator str_arr_it;
          for (str_arr_it = str_arr.begin(); str_arr_it != str_arr.end(); str_arr_it++) {
            UnaryFeatureVec::iterator append_begin_it = new_feat_vec.end();
            std::copy(feat_vec_out.begin(), feat_vec_out.end(),
                      back_inserter(new_feat_vec));
            UnaryFeatureVec::iterator append_end_it = new_feat_vec.end();
            appendStr(append_begin_it, append_end_it, *str_arr_it);
          }
          std::swap(feat_vec_out, new_feat_vec);
        }
        stack.pop();
      } break;
      case FCATSTR: {
        std::string &str = stack.top().str();
        appendStr(feat_vec_out, str);
        stack.pop();
      } break;
      case FCATBOOL: {
        bool b = stack.top().boolVal();
        appendStr(feat_vec_out, b ? "t" : "f");
        stack.pop();
      } break;
      case FCATINT: {
        int i  = stack.top().intVal();
        stringstream ss;
        ss << i;
        appendStr(feat_vec_out, ss.str());
        stack.pop();
      } break;
      default:
        unimplemented_opcode(opcode_names[*bytecode_iter]);
        break;
    }
  }
  assert(stack.empty());
}

PerceptronSpec::StackValue
PerceptronSpec::Machine::getValue()
{
  for (; bytecode_iter != feat.end(); bytecode_iter++) 
  {
    Opcode op = (Bytecode){.intbyte=static_cast<signed char>(*bytecode_iter)}.op;
    if (execCommonOp(op)) 
    {
      continue;
    }
    unimplemented_opcode(opcode_names[*bytecode_iter]);
  }
  StackValue result = stack.pop_off();
  assert(stack.empty());
  return result;
}

void
PerceptronSpec::Machine::unimplemented_opcode(std::string opstr) {
  int bytecode_idx = bytecode_iter - feat.begin();
  std::stringstream msg;
  msg << "Unimplemented opcode: " << opstr
      << " at " << (is_feature ? "feature" : "global") << " #" << feat_idx << " address #" << bytecode_idx;
  throw Apertium::Exception::apertium_tagger::UnimplementedOpcode(msg);
}

PerceptronSpec::In::In(const VMSet &haystack) : haystack(haystack) {};

bool
PerceptronSpec::In::operator() (const std::string &needle) const {
  return haystack.find(needle) != haystack.end();
};

void PerceptronSpec::appendStr(UnaryFeatureVec &feat_vec,
                               const std::string &tail_str) {
  appendStr(feat_vec.begin(), feat_vec.end(), tail_str);
}

void PerceptronSpec::appendStr(UnaryFeatureVec::iterator begin,
                               UnaryFeatureVec::iterator end,
                               const std::string &tail_str) {
  for (;begin != end; begin++) {
    begin->push_back(tail_str);
  }
}

std::string
PerceptronSpec::Machine::get_tag(const Tag &in) {
  return UtfConverter::toUtf8(in.TheTag);
}

void PerceptronSpec::serialiseFeatDefn(
    std::ostream &serialised, const FeatureDefn &defn) const {
  Serialiser<std::string>::serialise(
      std::string((char*)&(defn.front()), defn.size()),
      serialised);
}

void PerceptronSpec::deserialiseFeatDefn(
    std::istream &serialised, FeatureDefn &feat) {
  std::string feat_str = Deserialiser<std::string>::deserialise(serialised);
  feat.reserve(feat_str.size());
  std::string::iterator feat_str_it;
  for (feat_str_it = feat_str.begin(); feat_str_it != feat_str.end(); feat_str_it++) {
    feat.push_back(*feat_str_it);
  }
}

void PerceptronSpec::serialiseFeatDefnVec(
    std::ostream &serialised, const std::vector<FeatureDefn> &defn_vec) const {
  Serialiser<size_t>::serialise(defn_vec.size(), serialised);
  std::vector<FeatureDefn>::const_iterator feat_it;
  for (feat_it = defn_vec.begin(); feat_it != defn_vec.end(); feat_it++) {
    serialiseFeatDefn(serialised, *feat_it);
  }
}

void PerceptronSpec::deserialiseFeatDefnVec(
    std::istream &serialised, std::vector<FeatureDefn> &defn_vec) {
  size_t num_features = Deserialiser<size_t>::deserialise(serialised);
  while (num_features-- > 0) {
    defn_vec.push_back(FeatureDefn());
    FeatureDefn &feat = defn_vec.back();
    deserialiseFeatDefn(serialised, feat);
  }
}

void PerceptronSpec::serialise(std::ostream &serialised) const {
  Serialiser<size_t>::serialise(beam_width, serialised);
  Serialiser<std::vector<std::string> >::serialise(str_consts, serialised);
  Serialiser<std::vector<VMSet> >::serialise(set_consts, serialised);
  serialiseFeatDefnVec(serialised, features);
  serialiseFeatDefnVec(serialised, global_defns);
  serialiseFeatDefn(serialised, global_pred);
  if (coarse_tags) {
    Serialiser<size_t>::serialise(1, serialised);
    coarse_tags->serialise(serialised);
  } else {
    Serialiser<size_t>::serialise(0, serialised);
  }
}

void PerceptronSpec::deserialise(std::istream &serialised) {
  beam_width = Deserialiser<size_t>::deserialise(serialised);
  str_consts = Deserialiser<std::vector<std::string> >::deserialise(serialised);
  set_consts = Deserialiser<std::vector<VMSet> >::deserialise(serialised);
  deserialiseFeatDefnVec(serialised, features);
  deserialiseFeatDefnVec(serialised, global_defns);
  deserialiseFeatDefn(serialised, global_pred);
  if (serialised.eof()) {
    return;
  }
  size_t has_coarse_tags = Deserialiser<size_t>::deserialise(serialised);
  if (has_coarse_tags == 1) {
    coarse_tags = Optional<TaggerDataPercepCoarseTags>(TaggerDataPercepCoarseTags());
    coarse_tags->deserialise(serialised);
  }
}

}
