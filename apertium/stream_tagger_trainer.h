#ifndef STREAM_TAGGER_TRAINER_H
#define STREAM_TAGGER_TRAINER_H

#include "stream.h"
#include "basic_tagger.h"
#include <istream>
#include <ostream>

namespace Apertium {
class StreamTaggerTrainer : protected virtual basic_Tagger {
public:
  virtual ~StreamTaggerTrainer() = 0;
  virtual void serialise(std::ostream &Serialised_basic_Tagger) const = 0;
  virtual void train(Stream &TaggedCorpus) = 0;
protected:
  StreamTaggerTrainer();
};
}

#endif // STREAM_TAGGER_TRAINER_H
