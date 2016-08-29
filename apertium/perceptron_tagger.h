#ifndef __PERCEPTRON_TAGGER_H
#define __PERCEPTRON_TAGGER_H

#include <apertium/file_tagger.h>
#include <apertium/sentence_stream.h>
#include <apertium/tagger_data_perceptron.h>
#include <apertium/perceptron_spec.h>
#include <apertium/stream.h>
#include <apertium/stream_tagger.h>
#include <apertium/stream_tagger_trainer.h>

namespace Apertium {
class PerceptronTagger : public StreamTagger, public StreamTaggerTrainer, SentenceStream::SentenceTagger {
public:
  /*
        # Each feature gets its own weight vector, so weights is a dict-of-dicts
        self.weights = {}
        self.classes = set()
        # The accumulated values, for the averaging. These will be keyed by
        # feature/clas tuples
        self._totals = defaultdict(int)
        # The last time the feature was changed, for the averaging. Also
        # keyed by feature/clas tuples
        # (tstamps is short for timestamps)
        self._tstamps = defaultdict(int)
        # Number of instances seen
        self.i = 0
  */
  PerceptronTagger(basic_Tagger::Flags TheFlags);
  virtual ~PerceptronTagger();
  // trainer
  virtual void serialise(std::ostream &serialised) const;
  virtual void train(Stream&);
  virtual void train(Stream &tagged, Stream &untagged, int iterations);
  // tagger
  virtual void deserialise(std::istream &serialised);
  virtual void tag(Stream &input, std::wostream &output) const;

  /*
    def predict(self, features):
        '''Dot-product the features and current weights and return the best label.'''
        scores = defaultdict(float)
        for feat, value in features.items():
            if feat not in self.weights or value == 0:
                continue
            weights = self.weights[feat]
            for label, weight in weights.items():
                scores[label] += value * weight
        # Do a secondary alphabetic sort, for stability
        return max(self.classes, key=lambda label: (scores[label], label))

    def update(self, truth, guess, features):
        '''Update the feature weights.'''
        def upd_feat(c, f, w, v):
            param = (f, c)
            self._totals[param] += (self.i - self._tstamps[param]) * w
            self._tstamps[param] = self.i
            self.weights[f][c] = w + v

        self.i += 1
        if truth == guess:
            return None
        for f in features:
            weights = self.weights.setdefault(f, {})
            upd_feat(truth, f, weights.get(truth, 0.0), 1.0)
            upd_feat(guess, f, weights.get(guess, 0.0), -1.0)
        return None

    def average_weights(self):
        '''Average weights from all iterations.'''
        for feat, weights in self.weights.items():
            new_feat_weights = {}
            for clas, weight in weights.items():
                param = (feat, clas)
                total = self._totals[param]
                total += (self.i - self._tstamps[param]) * weight
                averaged = round(total / float(self.i), 3)
                if averaged:
                    new_feat_weights[clas] = averaged
            self.weights[feat] = new_feat_weights
        return None

    def save(self, path):
        '''Save the pickled model weights.'''
        return pickle.dump(dict(self.weights), open(path, 'w'))

    def load(self, path):
        '''Load the pickled model weights.'''
        self.weights = pickle.load(open(path))
        return None
	*/
protected:
  virtual void deserialise(const TaggerData &Deserialised_FILE_Tagger);
  virtual TaggedSentence tagSentence(const Sentence &untagged) const;
  virtual void outputLexicalUnit(
    const LexicalUnit &lexical_unit, const Optional<Analysis> analysis,
    std::wostream &output) const;
private:
  void trainSentence(const TrainingSentence &sentence);
  TaggerDataPerceptron tdpercep;
  PerceptronSpec spec;
  struct AgendaItem {
    TaggedSentence tagged;
    double score;
    /*
    AgendaItem& operator=(const AgendaItem &other) {
      tagged = other.tagged;
      score = other.score;
      return *this;
    }*/
  };
  struct TrainingAgendaItem : AgendaItem {
    FeatureVec vec;
    /*
    TrainingAgendaItem& operator=(const TrainingAgendaItem &other) {
      AgendaItem::operator=(other);
      vec = other.vec;
      return *this;
    }*/
  };
  struct ExtendTagged {
    Optional<Analysis> analy;
    ExtendTagged(const Analysis &analy);
    ExtendTagged();
    void operator() (AgendaItem &ai);
  };
  template <typename T> static void extendAgendaAll(
    std::vector<T> &agenda, Optional<Analysis> analy);
  friend bool operator<(AgendaItem &a, AgendaItem &b);
};
}

#endif
