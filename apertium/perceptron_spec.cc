#include <apertium/perceptron_spec.h>
#include <apertium/utf_converter.h>


namespace Apertium {

#define X(a) #a,
const std::string PerceptronSpec::opcode_names[] = {
  OPCODES
};
#undef X

std::map<const std::string, PerceptronSpec::Opcode>
PerceptronSpec::opcode_values;

PerceptronSpec::StaticConstruct::StaticConstruct() {
  size_t num_opcodes = sizeof(opcode_names) / sizeof(opcode_names[0]);
  for (size_t i=0; i < num_opcodes; i++) {
    PerceptronSpec::opcode_values[PerceptronSpec::opcode_names[i]] = (Opcode)i;
  }
}

void PerceptronSpec::get_features(
    const TaggedSentence &tagged, const Sentence &untagged,
    int token_idx, int wordoid_idx,
    std::vector<std::string> &feat_vec_out) const {
  std::vector<FeatureDefn>::const_iterator feat_it;
  vector<std::string> feat_vec_delta;
  for (feat_it = features.begin(); feat_it != features.end(); feat_it++) {
    Machine machine(*this, feat_it);
    machine.get_feature(tagged, untagged, token_idx, wordoid_idx, feat_vec_delta);
    feat_vec_out.insert(feat_vec_out.end(),
                        feat_vec_delta.begin(), feat_vec_delta.end());
  }
}

const std::string&
PerceptronSpec::Machine::get_str_operand() {
  return spec.str_consts[(++bytecode_iter)->uintbyte];
}

const VMSet&
PerceptronSpec::Machine::get_set_operand() {
  return spec.set_consts[(++bytecode_iter)->uintbyte];
}

int
PerceptronSpec::Machine::get_int_operand() {
  return (++bytecode_iter)->intbyte;
}

const LexicalUnit&
PerceptronSpec::Machine::get_token(const Sentence &untagged) {
  int target_token_idx = stack.pop_off().intVal();
  assert(0 <= target_token_idx && (size_t)target_token_idx < untagged.size());
  return *untagged[target_token_idx].TheLexicalUnit;
}

const std::vector<Morpheme>&
PerceptronSpec::Machine::tagged_to_wordoids(const TaggedToken &tt) {
  return tt->TheMorphemes;
}

const Morpheme&
PerceptronSpec::Machine::get_wordoid(const TaggedSentence &tagged) {
  int target_wordoid_idx = stack.pop_off().intVal();
  int target_token_idx = stack.pop_off().intVal();
  assert(0 <= target_token_idx && (size_t)target_token_idx < tagged.size());

  const vector<Morpheme> &wordoids = tagged[target_token_idx]->TheMorphemes;
  assert(0 <= target_wordoid_idx && (size_t)target_wordoid_idx < wordoids.size());

  return wordoids[target_wordoid_idx];
}

PerceptronSpec::Machine::Machine(
    const PerceptronSpec &spec,
    std::vector<FeatureDefn>::const_iterator feat_iter)
  : feat_iter(feat_iter), bytecode_iter(feat_iter->begin()), spec(spec) {}

void PerceptronSpec::Machine::get_feature(
    const TaggedSentence &tagged, const Sentence &untagged,
    int token_idx, int wordoid_idx,
    std::vector<std::string> &feat_vec_out) {
  feat_vec_out.clear();
  for (; bytecode_iter != feat_iter->end(); bytecode_iter++) {
    switch (bytecode_iter->op) {
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
      case PUSHINT:
        stack.push(get_int_operand());
        break;
      case PUSHZERO:
        stack.push(0);
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
      case DIEIFFALSE:
        if (!stack.pop_off().boolVal()) {
          feat_vec_out.clear();
          return;
        }
        break;
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
        token_idx = clamp(token_idx, 0, (int)tagged.size() - 1);
        int wordoid_len = tagged_to_wordoids(tagged[token_idx]).size();
        wordoid_idx = clamp(wordoid_idx, 0, wordoid_len - 1);
        stack.push(token_idx);
        stack.push(wordoid_idx);
      } break;
      case CLAMPTAGGEDTOKADDR:
        stack.push(clamp(stack.pop_off().intVal(), 0, (int)tagged.size() - 1));
        break;
      case CLAMPTOKADDR:
        stack.push(clamp(stack.pop_off().intVal(), 0, (int)untagged.size() - 1));
        break;
      case GETTOKLF: {
        std::wstring lf = get_token(untagged).TheSurfaceForm;
        stack.push(new std::string(UtfConverter::toUtf8(lf)));
      } break;
      case GETWRDLF: {
        std::wstring lf = get_wordoid(tagged).TheLemma;
        stack.push(new std::string(UtfConverter::toUtf8(lf)));
      } break;
      case GETTAGS: {
        const std::vector<Tag> &tags = get_wordoid(tagged).TheTags;
        std::vector<std::string> *tags_str = new std::vector<std::string>;
        transform(tags.begin(), tags.end(), tags_str->begin(), get_tag);
        stack.push(tags_str);
      } break;
      case GETTAGSFLAT: {
        const std::vector<Tag> &tags = get_wordoid(tagged).TheTags;
        std::stringstream ss;
        std::vector<Tag>::const_iterator ti = tags.begin();
        for (; ti != tags.end(); ti++) {
          ss << "<";
          ss << UtfConverter::toUtf8(ti->TheTag);
          ss << ">";
        }
      } break;
      case SENTLENTOK:
        stack.push((int)untagged.size());
        break;
      case SENTLENTAGGEDTOK:
        stack.push((int)tagged.size());
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
      }
      case ISVALIDADDR: {
        int wordoid_idx = stack.pop_off().intVal();
        int token_idx = stack.pop_off().intVal();
        bool tokaddr_valid = 0 <= token_idx && (size_t)token_idx < tagged.size();
        if (!tokaddr_valid) {
          stack.push(false);
          break;
        }
        int wordoid_len = tagged[token_idx]->TheMorphemes.size();
        stack.push(0 <= wordoid_idx && wordoid_idx < wordoid_len);
      } break;
      case FILTERIN: {
        const VMSet& set_op = get_set_operand();
        std::vector<std::string> &str_arr = stack.top().strArr();
        str_arr.erase(std::remove_if(
            str_arr.begin(), str_arr.end(), std::not1(In(set_op))));
      } break;
      case SETHAS: {
        const VMSet& set_op = get_set_operand();
        std::string str = stack.pop_off().str();
        stack.push(set_op.find(str) != set_op.end());
      } break;
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
      case SLICEBEGIN: {
        int len = get_int_operand();
        std::string &str = stack.top().str();
        assert(0 < len && (size_t)len <= str.length());
        str.assign(str, 0, len);
      } break;
      case SLICEEND: {
        int len = get_int_operand();
        std::string &str = stack.top().str();
        size_t str_len = str.length();
        assert(0 < len && (size_t)len <= str.length());
        str.assign(str, str_len - len, len);
      } break;
      case SLICEOFFBEGIN: unimplemented_opcode("SLICEOFFBEGIN"); break;
      case SLICEOFFEND: unimplemented_opcode("SLICEOFFEND"); break;
      case LOWERARR: unimplemented_opcode("LOWERARR"); break;
      case STRLEN: {
        std::string str = stack.pop_off().str();
        stack.push((int)str.length());
      } break;
      case ARRLEN: {
        int str_arr_len = stack.pop_off().strArr().size();
        stack.push(str_arr_len);
      } break;
      case FCATSTRARR: {
        std::vector<std::string> &str_arr = stack.top().strArr();
        if (str_arr.size() == 0) {
          feat_vec_out.clear();
        } else {
          std::vector<std::string> new_feat_vec;
          new_feat_vec.reserve(feat_vec_out.size() * str_arr.size());
          std::vector<std::string>::const_iterator str_arr_it;
          for (str_arr_it = str_arr.begin(); str_arr_it != str_arr.end(); str_arr_it++) {
            std::transform(feat_vec_out.begin(), feat_vec_out.end(),
                           back_inserter(new_feat_vec), AppendStr(*str_arr_it));
          }
          std::swap(feat_vec_out, new_feat_vec);
        }
        stack.pop();
      } break;
      case FCATSTR: {
        std::string &str = stack.top().str();
        std::transform(feat_vec_out.begin(), feat_vec_out.end(),
                       feat_vec_out.begin(), AppendStr(str));
        stack.pop();
      } break;
      case FCATBOOL: {
        bool b = stack.top().boolVal();
        std::transform(feat_vec_out.begin(), feat_vec_out.end(),
                       feat_vec_out.begin(), AppendStr(b ? "t" : "f"));
        stack.pop();
      } break;
      case FCATINT: {
        int i  = stack.top().intVal();
        stringstream ss;
        ss << i;
        std::transform(feat_vec_out.begin(), feat_vec_out.end(),
                       feat_vec_out.begin(), AppendStr(ss.str()));
        stack.pop();
      } break;
      default:
        unimplemented_opcode(opcode_names[bytecode_iter->op]);
        break;
    }
  }
}

void
PerceptronSpec::Machine::unimplemented_opcode(std::string opstr) {
  int feat_idx = feat_iter - spec.features.begin();
  int bytecode_idx = bytecode_iter - feat_iter->begin();
  std::stringstream msg;
  msg << "Unimplemented opcode: " << opstr
      << " at feature #" << feat_idx<< " address #" << bytecode_idx;
  throw Apertium::Exception::apertium_tagger::UnimplementedOpcode(msg);
}

PerceptronSpec::In::In(const VMSet &haystack) : haystack(haystack) {};

bool
PerceptronSpec::In::operator() (const std::string &needle) const {
  return haystack.find(needle) != haystack.end();
};

PerceptronSpec::AppendStr::AppendStr(const std::string &tail_str)
  : tail_str(tail_str) {};

std::string PerceptronSpec::AppendStr::operator()(const std::string &head_str) {
  return head_str + tail_str;
}

std::string
PerceptronSpec::Machine::get_tag(const Tag &in) {
  return UtfConverter::toUtf8(in.TheTag);
}

int PerceptronSpec::clamp(int x, int lower, int upper) {
  return std::min(std::max(x, lower), upper);
}
}
