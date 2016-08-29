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

  bool isAfterSentenceEnd(StreamedType &token);
  bool isSentenceEnd(StreamedType &token);
  bool isSentenceEnd(Stream &in, bool sent_seg = false);
  class SentenceTagger {
  public:
    void tag(Stream &in, std::wostream &out, bool sent_seg) const;
    SentenceTagger();
  protected:
    virtual TaggedSentence tagSentence(const Sentence &untagged) const = 0;
    virtual void outputLexicalUnit(
      const LexicalUnit &lexical_unit, const Optional<Analysis> analysis,
      std::wostream &output) const = 0;
  private:
    void clearBuffers() const;
    void tagAndPutSentence(std::wostream &out) const;
    void putTaggedSent(
      std::wostream &out, TaggedSentence &tagged_sent, Sentence &full_sent,
      std::vector<bool> &flushes) const;
    mutable Sentence full_sent;
    mutable Sentence lexical_sent;
    mutable std::vector<bool> flushes;
  };

  class TrainingCorpus {
    void prematureEnd();
    bool contToEndOfSent(Stream &stream, StreamedType token,
                         unsigned int &line);
    bool sent_seg;
  public:
    unsigned int skipped;
    TrainingCorpus(Stream &tagged, Stream &untagged, bool skip_on_error, bool sent_seg);
    void shuffle();
    std::vector<TrainingSentence> sentences;
  };
}
}

#endif
