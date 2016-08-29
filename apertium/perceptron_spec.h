#ifndef _PERCEPTRON_SPEC_H
#define _PERCEPTRON_SPEC_H

#include <cassert>
#include <algorithm>
#include <string>
#include <vector>
#include <set>
#include <stack>
#include <map>
#include <apertium/sentence_stream.h>
#include <apertium/feature_vec.h>
#include <apertium/tagger_data_percep_coarse_tags.h>

/* The main part of the perceptron spec is the feature templates.  These are
 * represented as a bytecode for an simple intepreted stack virtual machine.
 * Instructions and data words are stored in 8-bit octets.  More complex
 * constants than 8 bit indegers such as sets or strings are references into
 * arrays which are part of this PerceptronSpec object.
 *
 * This class handles the execution of feature template programs to gather
 * features from a token/position in the prediction process and its context as
 * well as serialisation and.deserialisation.
 *
 * Compiling from the MTX XML format into bytecode is handled by MTXReader. */

using namespace Apertium::SentenceStream;

namespace Apertium {
typedef std::set<std::string> VMSet;
class PerceptronSpec
{
public:
  typedef std::vector<unsigned char> FeatureDefn;
  static void printFeature(std::wostream &out, const PerceptronSpec::FeatureDefn &feat_defn);
  friend std::wostream& operator<<(std::wostream &out, PerceptronSpec const &pt);
  PerceptronSpec();
  #define OPCODES \
    /** Boolean and arithmetic */\
    /* bool, bool -> bool */\
    X(OR) X(AND)\
    /* bool -> bool */\
    X(NOT)\
    /* int, int(operand) -> int */\
    X(ADI)\
    /* int, int -> int */\
    X(ADD)\
    /* int, int, int, int -> int, int */\
    X(ADD2)\
    /* int(operand) -> int */\
    X(PUSHINT)\
    /* int, int -> bool */\
    X(LT) X(LTE) X(GT) X(GTE) X(EQ) X(NEQ)\
    \
    /** Stack manipulation */\
    /* 'a -> 'a, 'a */\
    X(DUP)\
    /* 'a, 'b -> 'a, 'b, 'a, 'b */\
    X(DUP2)\
    /* 'a, 'b -> 'b, 'a */\
    X(SWAP)\
    \
    /** Flow control */\
    /* bool -> */\
    X(DIEIFFALSE)\
    /* -> */\
    X(FOREACHINIT)\
    /* int(operand), int(operand, slot), 'a[] -> {env: slot: 'a) */\
    X(FOREACH)\
    /* int(operand) -> */\
    X(ENDFOREACH)\
    \
    /** Registers/environment */\
    /* int(operand, slot), {globals: slot: 'b} -> 'b */\
    X(GETGVAR)\
    /* int(operand, slot), {env: slot: 'b} -> 'b */\
    X(GETVAR)\
    \
    /** Input addressing */\
    /* str, str(operand) -> bool */\
    X(STREQ)\
    /* str, setaddr(operand) -> bool */\
    X(STRIN)\
    /* -> tokaddr */\
    X(PUSHTOKADDR)\
    /* -> wrdaddr */\
    X(PUSHWRDADDR)\
    /* -> tokaddr, wrdaddr */\
    X(PUSHADDR)\
    /* tokenaddr, wrdaddr -> tokenaddr, wrdaddr */\
    X(ADJADDR)\
    /* tokenaddr, wrdaddr -> tokenaddr, wrdaddr */\
    X(CLAMPADDR)\
    /* tokenaddr -> tokenaddr */\
    X(CLAMPTAGGEDTOKADDR)\
    /* tokenaddr -> tokenaddr */\
    X(CLAMPTOKADDR)\
    \
    /* Input fetching and characterisation */\
    /* tokaddr, wrdaddr -> Wordoid */\
    X(GETWRD)\
    /* tokaddr -> str */\
    X(EXTOKSURF)\
    /* Wordoid -> str */\
    X(EXWRDLEMMA)\
    /* Wordoid -> str */\
    X(EXWRDCOARSETAG)\
    /* tokaddr -> str[] */\
    X(EXAMBGSET)\
    /* Wordoid -> str[] */\
    X(EXTAGS)\
    /* -> int */\
    X(SENTLENTOK)\
    /* -> int */\
    X(SENTLENTAGGEDTOK)\
    /* -> int */\
    X(SENTLENWRD)\
    /* tokaddr -> int */\
    X(TOKLENWRD)\
    /* tokaddr -> bool */\
    X(ISVALIDTOKADDR)\
    /* tokaddr -> bool */\
    X(ISVALIDTAGGEDTOKADDR)\
    /* tokaddr, wrdaddr -> bool */\
    X(ISVALIDADDR)\
    /* tokaddr -> Wordoid[] */\
    X(EXWRDARR)\
    \
    /** String/string array manipulation */\
    /* str[], setaddr(operand) -> str[] */\
    X(FILTERIN)\
    /* str, setaddr(operand) -> bool */\
    X(SETHAS)\
    /* str[], setaddr(operand) -> bool */\
    X(SETHASANY) X(SETHASALL)\
    /* str, str(operand) -> bool */\
    X(HASSUBSTR)\
    /* str, setaddr(operand) -> bool */\
    X(HASANYSUBSTR)\
    /* str -> str */\
    X(CPYSTR) X(LOWER)\
    /* a'[], int(operand), int(operand) -> a'[] (a' can be str) */\
    X(SLICE)\
    /* a'[], int(operand) -> a' (a' can not be str) */\
    X(SUBSCRIPT)\
    /* str[] -> str[] */\
    X(LOWERARR)\
    /* str -> int */\
    X(STRLEN)\
    /* str[] -> int */\
    X(ARRLEN)\
    /* str[], int(operand, str ref) -> str */\
    X(JOIN)\
    \
    /** Feature building */\
    /* str[] -> */\
    X(FCATSTRARR)\
    /* str -> */\
    X(FCATSTR)\
    /* bool -> */\
    X(FCATBOOL)\
    /* int -> */\
    X(FCATINT)
  #define X(a) a,
  enum Opcode {
    OPCODES
  };
  #undef X
  static bool static_constructed;
  static unsigned char num_opcodes;
  static const std::string opcode_names[];
  static const std::string type_names[];
  static std::map<const std::string, Opcode> opcode_values;
  static std::vector<Morpheme> untagged_sentinel;
  static LexicalUnit token_wordoids_underflow;
  static LexicalUnit token_wordoids_overflow;
  enum StackValueType {
    INTVAL, BVAL, STRVAL, STRARRVAL, WRDVAL, WRDARRVAL
  };
  class StackValue {
  public:
    friend std::wostream& operator<<(std::wostream& out, StackValue const &val) {
      switch (val.type) {
        case INTVAL:
          out << val.intVal();
          break;
        case BVAL:
          out << val.boolVal();
          break;
        case STRVAL:
          out << val.str().c_str();
          break;
        case STRARRVAL: {
          out << "[";
          std::vector<std::string> &str_arr = val.strArr();
          std::vector<std::string>::const_iterator it = str_arr.begin();
          for (; it != str_arr.end(); it++) {
            out << it->c_str();
          }
          out << "]";
        } break;
        case WRDVAL:
          out << val.wrd();
          break;
        case WRDARRVAL: {
          out << "[";
          std::vector<Morpheme> &wrd_arr = val.wrdArr();
          std::vector<Morpheme>::const_iterator it = wrd_arr.begin();
          for (; it != wrd_arr.end(); it++) {
            out << *it;
          }
          out << "]";
        } break;
        default: assert(false); break;
      }
      return out;
    }
    friend void swap(StackValue &a, StackValue &b) {
      using std::swap;

      swap(a.payload, b.payload);
      swap(a.type, b.type);
    }
    // Smart pointer + tagged union safe to store in STL types
    StackValue() {}
    StackValue(const StackValue &other) {
      // C++11: Probably reference counting with shared_ptr would be better
      // than all this copying if it were available
      //std::wcerr << "StackValue init\n";
      type = other.type;
      switch (type) {
        case STRVAL:
          payload.strval = new std::string(*other.payload.strval);
          break;
        case STRARRVAL:
          payload.strarrval =
              new std::vector<std::string>(*other.payload.strarrval);
          break;
        case WRDVAL:
          payload.wrdval = new Morpheme(*other.payload.wrdval);
          break;
        case WRDARRVAL:
          payload.wrdarrval = new std::vector<Morpheme>(*other.payload.wrdarrval);
          break;
        default:
          payload = other.payload;
          break;
      }
    }
    StackValue& operator=(StackValue other) {
      //std::wcerr << "StackValue assign\n";
      swap(*this, other);
      return *this;
    }
    StackValue(int intval) {
      payload.intval = intval;
      type = INTVAL;
    }
    StackValue(bool bval) {
      payload.bval = bval;
      type = BVAL;
    }
    StackValue(const std::string &strval) {
      payload.strval = new std::string(strval);
      type = STRVAL;
    }
    StackValue(const std::vector<std::string> &strarrval) {
      payload.strarrval = new std::vector<std::string>(strarrval);
      type = STRARRVAL;
    }
    StackValue(const Morpheme &wordoid) {
      /*std::wcerr << L"Before ";
      std::vector<Tag>::const_iterator it = wordoid.TheTags.begin();
      for (;it != wordoid.TheTags.end(); it++) {
        std::wcerr << &(*it) << " ";
      }
      std::wcerr << L"\n";
      std::wcerr << L"Copy morpheme " << &wordoid;*/
      payload.wrdval = new Morpheme(wordoid);
      /*std::wcerr << L" to " << payload.wrdval << "\n";
      std::wcerr << L"After ";
      it = payload.wrdval->TheTags.begin();
      for (;it != payload.wrdval->TheTags.end(); it++) {
        std::wcerr << &(*it) << " ";
      }
      std::wcerr << L"\n";*/
      type = WRDVAL;
    }
    StackValue(const std::vector<Morpheme> &wordoids) {
      payload.wrdarrval = new std::vector<Morpheme>(wordoids);
      type = WRDARRVAL;
    }
    StackValue(std::string *strval) {
      payload.strval = strval;
      type = STRVAL;
    }
    StackValue(std::vector<std::string> *strarrval) {
      payload.strarrval = strarrval;
      type = STRARRVAL;
    }
    StackValue(Morpheme *wordoid) {
      payload.wrdval = wordoid;
      type = WRDVAL;
    }
    StackValue(std::vector<Morpheme> *wordoids) {
      payload.wrdarrval = wordoids;
      type = WRDARRVAL;
    }
    ~StackValue() {
      switch (type) {
        case STRVAL:
          delete payload.strval;
          break;
        case STRARRVAL:
          delete payload.strarrval;
          break;
        case WRDVAL:
          delete payload.wrdval;
          break;
        case WRDARRVAL:
          delete payload.wrdarrval;
          break;
        default: break;
      }
    }
    int intVal() const {
      assert(type == INTVAL);
      return payload.intval;
    }
    bool boolVal() const {
      assert(type == BVAL);
      return payload.bval;
    }
    std::string& str() const {
      assert(type == STRVAL);
      return *payload.strval;
    }
    std::vector<std::string>& strArr() const {
      assert(type == STRARRVAL);
      return *payload.strarrval;
    }
    Morpheme& wrd() const {
      assert(type == WRDVAL);
      return *payload.wrdval;
    }
    std::vector<Morpheme>& wrdArr() const {
      assert(type == WRDARRVAL);
      return *payload.wrdarrval;
    }
    size_t size() const {
      if (type == STRARRVAL) {
        return strArr().size();
      } else if (type == WRDARRVAL) {
        return wrdArr().size();
      } else {
        assert(false);
      }
    }
    StackValue operator[](int n) const {
      if (type == STRARRVAL) {
        return StackValue(strArr()[n]);
      } else if (type == WRDARRVAL) {
        return StackValue(wrdArr()[n]);
      } else {
        assert(false);
      }
    }
    union StackValueUnion {
      int intval;
      bool bval;
      std::string* strval;
      std::vector<std::string>* strarrval;
      Morpheme* wrdval;
      std::vector<Morpheme>* wrdarrval;
    } payload;
    StackValueType type;
  };
  union Bytecode {
    Opcode op : 8;
    unsigned char uintbyte : 8;
    signed char intbyte : 8;
  };
  Optional<TaggerDataPercepCoarseTags> coarse_tags;
  static std::string dot;
  std::vector<std::string> str_consts;
  std::vector<VMSet> set_consts;
  mutable std::vector<StackValue> global_results;
  std::vector<FeatureDefn> global_defns;
  std::vector<FeatureDefn> features;
  FeatureDefn global_pred;
  void get_features(
    const TaggedSentence &tagged, const Sentence &untagged,
    int token_idx, int wordoid_idx,
    UnaryFeatureVec &feat_vec_out) const;
  std::string coarsen(const Morpheme &wrd) const;
  void clearCache() const;
  int beam_width;
  mutable std::map<const Morpheme, std::string> coarsen_cache;
private:
  class MachineStack {
    std::deque<StackValue> data;
    template <typename OStream> friend OStream& operator<<(OStream & out, MachineStack const &pt) {
      out << pt.data.size() << ": ";
      std::deque<StackValue>::const_iterator it;
      for (it = pt.data.begin(); it != pt.data.end(); it++) {
        out << it->payload.intval << " ";
      }
      return out;
    }
  public:
    void pop() {
      data.pop_back();
    }
    /*void push(StackValue val) {
      std::wcerr << "before copy push\n";
      data.push_back(val);
      std::wcerr << "after copy push\n";
    }*/
    void push(const StackValue &val) {
      //std::wcerr << "before push\n";
      data.push_back(val);
      //std::wcerr << "after push\n";
    }
    StackValue& top() {
      return data.back();
    }
    StackValue pop_off() {
      //std::wcerr << L"Top value: " << top().payload.intval << "\n";
      StackValue ret = top();
      pop();
      return ret;
    }
    size_t size() {
      return data.size();
    }
    bool empty() {
      return data.empty();
    }
  };
  class Machine {
    const PerceptronSpec &spec;
    bool is_feature;
    const FeatureDefn &feat;
    const size_t &feat_idx;
    std::vector<unsigned char>::const_iterator bytecode_iter;
    const TaggedSentence &tagged;
    const Sentence &untagged;
    int token_idx;
    int wordoid_idx;
    MachineStack stack;
    struct LoopState {
      size_t initial_stack;
      StackValue iterable;
      size_t iteration;
      StackValue accumulator;
    };
    std::deque<LoopState> loop_stack;
    std::vector<StackValue> slots;
    void unimplemented_opcode(std::string opstr);
    const LexicalUnit& get_token(const Sentence &untagged);
    const std::vector<Morpheme>& tagged_to_wordoids(const TaggedToken &tt);
    const Morpheme& get_wordoid(const TaggedSentence &tagged);
    const VMSet& get_set_operand();
    int get_int_operand();
    unsigned int get_uint_operand();
    const std::string& get_str_operand();
    static std::string get_tag(const Tag &in);
    bool execCommonOp(Opcode op);
  public:
    void traceMachineState();
    void getFeature(UnaryFeatureVec &feat_vec_out);
    StackValue getValue();
    Machine(
      const PerceptronSpec &spec,
      const FeatureDefn &feat,
      size_t feat_idx,
      bool is_feature,
      const TaggedSentence &tagged,
      const Sentence &untagged,
      int token_idx,
      int wordoid_idx);
  };
  struct In : public std::unary_function<const std::string&, bool> {
    const VMSet& haystack;
    In(const VMSet &haystack);
    bool operator() (const std::string &needle) const;
  };
  static void appendStr(UnaryFeatureVec &feat_vec,
                        const std::string &tail_str);
  static void appendStr(UnaryFeatureVec::iterator begin,
                        UnaryFeatureVec::iterator end,
                        const std::string &tail_str);
  void serialiseFeatDefn(
    std::ostream &serialised, const FeatureDefn &defn) const;
  void deserialiseFeatDefn(
    std::istream &serialised, FeatureDefn &feat);
  void serialiseFeatDefnVec(
    std::ostream &serialised, const std::vector<FeatureDefn> &defn_vec) const;
  void deserialiseFeatDefnVec(
    std::istream &serialised, std::vector<FeatureDefn> &defn_vec);
public:
  void serialise(std::ostream &serialised) const;
  void deserialise(std::istream &serialised);
};
}

#endif
