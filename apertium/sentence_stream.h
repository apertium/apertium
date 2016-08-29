#ifndef _SENTENCE_STREAM_H
#define _SENTENCE_STREAM_H

#include <memory>
#include <apertium/optional.h>
#include <apertium/stream.h>
#include <apertium/streamed_type.h>
#include <apertium/stream_tagger.h>

namespace Apertium {
namespace SentenceStream {
  typedef Optional<Analysis> TaggedToken;
  typedef std::vector<StreamedType> Sentence;
  typedef std::vector<TaggedToken> TaggedSentence;
  typedef std::pair<TaggedSentence, Sentence> TrainingSentence;

  bool isSentenceEnd(StreamedType &token);
  class SentenceTagger {
  public:
    void tag(Stream &in, std::wostream &out) const;
  protected:
    virtual TaggedSentence tagSentence(const Sentence &untagged) const = 0;
    virtual void outputLexicalUnit(
      const LexicalUnit &lexical_unit, const Optional<Analysis> analysis,
      std::wostream &output) const = 0;
  private:
    void putTaggedSent(
      std::wostream &out, TaggedSentence &tagged_sent, Sentence &full_sent,
      std::vector<bool> &flushes) const;
  };

  class TrainingCorpus {
  public:
    TrainingCorpus(Stream &stream_tagged, Stream &stream_untagged);
    void shuffle();
    std::vector<TrainingSentence> sentences;
  };
}
}

#endif
