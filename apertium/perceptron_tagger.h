#ifndef __PERCEPTRON_TAGGER_H
#define __PERCEPTRON_TAGGER_H

#include <apertium/file_tagger.h>
#include <apertium/sentence_stream.h>
#include <apertium/perceptron_spec.h>
#include <apertium/stream.h>
#include <apertium/stream_tagger.h>
#include <apertium/feature_vec.h>
#include <apertium/feature_vec_averager.h>

namespace Apertium {
class PerceptronTagger : public StreamTagger, SentenceStream::SentenceTagger {
public:
  friend int perceptron_trace(int argc, char* argv[]);
  PerceptronTagger(TaggerFlags TheFlags);
  virtual ~PerceptronTagger();
  // trainer
  virtual void serialise(std::ostream &serialised) const;
  virtual void train(Stream&);
  virtual void train(Stream &tagged, Stream &untagged, int iterations);
  // tagger
  virtual void deserialise(std::istream &serialised);
  virtual void tag(Stream &input, std::ostream &output);

  void read_spec(const std::string &filename);

  friend std::ostream& operator<<(std::ostream &out, PerceptronTagger const &pt);
protected:
  virtual TaggedSentence tagSentence(const Sentence &untagged);
  virtual void outputLexicalUnit(
    const LexicalUnit &lexical_unit, const Optional<Analysis> analysis,
    std::ostream &output);
private:
  bool trainSentence(
    const TrainingSentence &sentence,
    FeatureVecAverager &avg_weights);
  FeatureVec weights;
  PerceptronSpec spec;
  struct AgendaItem {
    TaggedSentence tagged;
    double score;
  };
  struct TrainingAgendaItem : AgendaItem {
    FeatureVec vec;
  };
  struct ExtendTagged {
    Optional<Analysis> analy;
    ExtendTagged(const Analysis &analy);
    ExtendTagged();
    void operator() (AgendaItem &ai);
  };
  template <typename T> static void extendAgendaAll(
    std::vector<T> &agenda, Optional<Analysis> analy);
  friend std::ostream& operator<<(std::ostream &out,
                                   const TrainingAgendaItem &tai);
  friend std::ostream& operator<<(
      std::ostream &out, const std::vector<TrainingAgendaItem> &agenda);
  friend bool operator<(const AgendaItem &a, const AgendaItem &b);
  friend std::ostream& operator<<(
      std::ostream &out, const PerceptronTagger::AgendaItem &ai);
  friend std::ostream& operator<<(
      std::ostream &out, const std::vector<PerceptronTagger::AgendaItem> &agenda);
};

std::ostream& operator<<(std::ostream &out, const TaggedSentence &tagged);
}

#endif
