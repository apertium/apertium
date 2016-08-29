#include <apertium/perceptron_spec.h>
#include <apertium/utf_converter.h>
#include <apertium/deserialiser.h>
#include <apertium/serialiser.h>


namespace Apertium {

template <typename OStream>
OStream&
operator<<(OStream & out, PerceptronSpec const &ps) {
  for (size_t i = 0; i < ps.features.size(); i++) {
    out << "Feature " << i << "\n";
    for (size_t j = 0; j < ps.features[i].size(); j++) {
      out << std::hex << ps.features[i][j] << " ";
    }
    out << "\n";
    for (size_t j = 0; j < ps.features[i].size(); j++) {
      if (ps.features[i][j] < PerceptronSpec::num_opcodes) {
        out << PerceptronSpec::opcode_names[ps.features[i][j]].c_str() << " ";
      } else {
        out << "XX ";
      }
    }
    out << "\n";
  }
  return out;
}

template std::wostream&
operator<<(std::wostream& out, PerceptronSpec const &ps);

template std::ostream&
operator<<(std::ostream& out, PerceptronSpec const &ps);

#define X(a) #a,
const std::string PerceptronSpec::opcode_names[] = {
  OPCODES
};
#undef X

PerceptronSpec::PerceptronSpec() {
  if (!static_constructed) {
    num_opcodes = sizeof(opcode_names) / sizeof(opcode_names[0]);
    for (size_t i=0; i < num_opcodes; i++) {
      opcode_values[opcode_names[i]] = (Opcode)i;
    }

    Morpheme untagged_morpheme;
    untagged_morpheme.TheLemma = L"!!UNTAGGED";
    Tag untagged_tag;
    untagged_tag.TheTag = L"!ut";
    untagged_morpheme.TheTags.push_back(untagged_tag);
    untagged_sentinel.push_back(untagged_morpheme);

    static_constructed = true;
  }
}

unsigned char PerceptronSpec::num_opcodes;
bool PerceptronSpec::static_constructed = false;
std::map<const std::string, PerceptronSpec::Opcode>
PerceptronSpec::opcode_values;
std::vector<Morpheme> PerceptronSpec::untagged_sentinel;

void PerceptronSpec::get_features(
    const TaggedSentence &tagged, const Sentence &untagged,
    int token_idx, int wordoid_idx,
    UnaryFeatureVec &feat_vec_out) const {
  std::vector<FeatureDefn>::const_iterator feat_it;
  UnaryFeatureVec feat_vec_delta;
  for (feat_it = features.begin(); feat_it != features.end(); feat_it++) {
    feat_vec_delta.clear();
    feat_vec_delta.push_back(FeatureKey());
    FeatureKey &fk = feat_vec_delta.back();
    fk.push_back(std::string((char*)&(feat_it->front()), feat_it->size()));
    Machine machine(*this, feat_it);
    machine.get_feature(tagged, untagged, token_idx, wordoid_idx, feat_vec_delta);
    feat_vec_out.insert(feat_vec_out.end(),
                        feat_vec_delta.begin(), feat_vec_delta.end());
  }
}

const std::string&
PerceptronSpec::Machine::get_str_operand() {
  return spec.str_consts[*(++bytecode_iter)];
}

const VMSet&
PerceptronSpec::Machine::get_set_operand() {
  return spec.set_consts[*(++bytecode_iter)];
}

int
PerceptronSpec::Machine::get_int_operand() {
  return (Bytecode){.uintbyte=*(++bytecode_iter)}.intbyte;
}

const LexicalUnit&
PerceptronSpec::Machine::get_token(const Sentence &untagged) {
  int target_token_idx = stack.pop_off().intVal();
  assert(0 <= target_token_idx && (size_t)target_token_idx < untagged.size());
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
  assert(0 <= target_token_idx && (size_t)target_token_idx < tagged.size());

  const vector<Morpheme> &wordoids = tagged_to_wordoids(tagged[target_token_idx]);
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
    UnaryFeatureVec &feat_vec_out) {
  for (; bytecode_iter != feat_iter->end(); bytecode_iter++) {
    /*
    std::wcerr << "pc: " << bytecode_iter - feat_iter->begin() << "\n";
    std::wcerr << "peek: ";
    std::wcerr << bytecode_iter->uintbyte;
    if (bytecode_iter->uintbyte < num_opcodes) {
      std::wcerr << " (" << opcode_names[bytecode_iter->uintbyte].c_str() << ")";
    }
    std::wcerr << "\n";
    std::wcerr << "stack: " << stack << "\n";
    */
    switch ((Bytecode){.intbyte=*bytecode_iter}.op) {
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
        stack.push(StackValue(ss.str()));
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
        int wordoid_len = tagged_to_wordoids(tagged[token_idx]).size();
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

bool PerceptronSpec::inRange(int lower, int upper, int x) {
  return lower <= x && x < upper;
}

int PerceptronSpec::clamp(int lower, int upper, int x) {
  return std::min(std::max(x, lower), upper);
}

void PerceptronSpec::serialise(std::ostream &serialised) const {
  Serialiser<size_t>::serialise(beam_width, serialised);
  Serialiser<std::vector<std::string> >::serialise(str_consts, serialised);
  Serialiser<std::vector<VMSet> >::serialise(set_consts, serialised);
  Serialiser<size_t>::serialise(features.size(), serialised);
  std::vector<FeatureDefn>::const_iterator feat_it;
  for (feat_it = features.begin(); feat_it != features.end(); feat_it++) {
    Serialiser<std::string>::serialise(
        std::string((char*)&(feat_it->front()), feat_it->size()),
        serialised);
  }
}

void PerceptronSpec::deserialise(std::istream &serialised) {
  beam_width = Deserialiser<size_t>::deserialise(serialised);
  str_consts = Deserialiser<std::vector<std::string> >::deserialise(serialised);
  set_consts = Deserialiser<std::vector<VMSet> >::deserialise(serialised);
  size_t num_features = Deserialiser<size_t>::deserialise(serialised);
  while (num_features-- > 0) {
    features.push_back(FeatureDefn());
    FeatureDefn &feat = features.back();
    std::string feat_str = Deserialiser<std::string>::deserialise(serialised);
    feat.reserve(feat_str.size());
    std::string::iterator feat_str_it;
    for (feat_str_it = feat_str.begin(); feat_str_it != feat_str.end(); feat_str_it++) {
      feat.push_back(*feat_str_it);
    }
  }
}

}
