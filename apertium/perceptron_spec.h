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

using namespace Apertium::SentenceStream;

namespace Apertium {
typedef std::set<std::string> VMSet;
class PerceptronSpec
{
public:
  template <typename OStream> friend OStream& operator<<(OStream & out, PerceptronSpec const &pt);
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
    /* int(operand) -> int */\
    X(PUSHINT)\
    /* -> int */\
    X(PUSHZERO)\
    /* int, int(operand) -> bool */\
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
    /* tokaddr -> str */\
    X(GETTOKLF)\
    /* tokaddr, wrdaddr -> str */\
    X(GETWRDLF)\
    /* tokaddr, wrdaddr -> str[] */\
    X(GETTAGS)\
    /* tokaddr, wrdaddr -> str */\
    X(GETTAGSFLAT)\
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
    /* str, int(operand) -> str */\
    X(SLICEBEGIN) X(SLICEEND)\
    /* str, int(operand), int(operand) -> str */\
    X(SLICEOFFBEGIN) X(SLICEOFFEND)\
    /* str[] -> str[] */\
    X(LOWERARR)\
    /* str -> int */\
    X(STRLEN)\
    /* str[] -> int */\
    X(ARRLEN)\
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
  static std::map<const std::string, Opcode> opcode_values;
  static std::vector<Morpheme> untagged_sentinel;
  template <typename T> struct CountPtr {
    int count;
    T *ptr;
  };
  class StackValue {
  public:
    // Smart pointer + tagged union safe to store in STL types
    StackValue() {}
    StackValue(const StackValue &other) {
      // C++11: Probably reference counting with shared_ptr would be better
      // than all this copying if it were available
      type = other.type;
      switch (type) {
        case STRVAL:
          payload.strval = new std::string(*other.payload.strval);
          break;
        case STRARRVAL:
          payload.strarrval =
              new std::vector<std::string>(*other.payload.strarrval);
          break;
        default:
          payload = other.payload;
          break;
      }
    }
    StackValue& operator=(StackValue other) {
      std::swap(*this, other);
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
    StackValue(std::string *strval) {
      payload.strval = strval;
      type = STRVAL;
    }
    StackValue(std::vector<std::string> *strarrval) {
      payload.strarrval = strarrval;
      type = STRARRVAL;
    }
    ~StackValue() {
      switch (type) {
        case STRVAL:
          delete payload.strval;
          break;
        case STRARRVAL:
          delete payload.strarrval;
          break;
        default: break;
      }
    }
    int intVal() {
      assert(type == INTVAL);
      return payload.intval;
    }
    bool boolVal() {
      assert(type == BVAL);
      return payload.bval;
    }
    std::string& str() {
      assert(type == STRVAL);
      return *payload.strval;
    }
    std::vector<std::string>& strArr() {
      assert(type == STRARRVAL);
      return *payload.strarrval;
    }
    union {
      int intval;
      bool bval;
      std::string* strval;
      std::vector<std::string>* strarrval;
    } payload;
    enum {
      INTVAL, BVAL, STRVAL, STRARRVAL
    } type;
  };
  union Bytecode {
    Opcode op : 8;
    unsigned char uintbyte : 8;
    signed char intbyte : 8;
  };
  std::vector<std::string> str_consts;
  std::vector<VMSet> set_consts;
  typedef std::vector<unsigned char> FeatureDefn;
  std::vector<FeatureDefn> features;
  void get_features(
    const TaggedSentence &tagged, const Sentence &untagged,
    int token_idx, int wordoid_idx,
    UnaryFeatureVec &feat_vec_out) const;
  int beam_width;
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
    void push(const StackValue &val) {
      data.push_back(val);
    }
    StackValue& top() {
      return data.back();
    }
    StackValue pop_off() {
      StackValue ret = top();
      pop();
      return ret;
    }
  };
  class Machine {
    std::vector<FeatureDefn>::const_iterator feat_iter;
    std::vector<unsigned char>::const_iterator bytecode_iter;
    const PerceptronSpec &spec;
    MachineStack stack;
    void unimplemented_opcode(std::string opstr);
    const LexicalUnit& get_token(const Sentence &untagged);
    const std::vector<Morpheme>& tagged_to_wordoids(const TaggedToken &tt);
    const Morpheme& get_wordoid(const TaggedSentence &tagged);
    const VMSet& get_set_operand();
    int get_int_operand();
    const std::string& get_str_operand();
    static std::string get_tag(const Tag &in);
  public:
    void get_feature(
      const TaggedSentence &tagged, const Sentence &untagged,
      int token_idx, int wordoid_idx,
      UnaryFeatureVec &feat_vec_out);
    Machine(
      const PerceptronSpec &spec,
      std::vector<FeatureDefn>::const_iterator feat_iter);
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
  static bool inRange(int lower, int upper, int x);
  static int clamp(int lower, int upper, int x);
public:
  void serialise(std::ostream &serialised) const;
  void deserialise(std::istream &serialised);
};
}

#endif
