#include <algorithm>
#include <apertium/stream_tagger.h>
#include <apertium/sentence_stream.h>
#include <apertium/exception.h>

namespace Apertium {
namespace SentenceStream {

bool isSentenceEnd(StreamedType &token) {
  std::vector<Analysis> &analyses = token.TheLexicalUnit->TheAnalyses;
  if (analyses.size() != 1) {
    return false;
  }
  std::vector<Morpheme> &morphemes = analyses.begin()->TheMorphemes;
  if (morphemes.size() != 1) {
    return false;
  }
  std::vector<Tag> &tags = morphemes.begin()->TheTags;
  if (tags.size() != 1) {
    return false;
  }
  Tag &tag = *tags.begin();
  if (tag.TheTag != L"sent") {
    return false;
  }
  return true;
}

void SentenceTagger::tag(Stream &in, std::wostream &out) const {
  Sentence full_sent;
  Sentence lexical_sent;
  std::vector<bool> flushes;
  TaggedSentence tagged_sent;

  while (true) {
    StreamedType token = in.get();
    full_sent.push_back(token);
    flushes.push_back(in.flush_());

    if (!token.TheLexicalUnit) {
      if (!in.flush_()) {
        tagged_sent = tagSentence(lexical_sent);
        putTaggedSent(out, tagged_sent, full_sent, flushes);
        break;
      }

      continue;
    }

    lexical_sent.push_back(token);
    if (isSentenceEnd(token)) {
      tagged_sent = tagSentence(lexical_sent);
      putTaggedSent(out, tagged_sent, full_sent, flushes);
    }

  }
}

void SentenceTagger::putTaggedSent(
    std::wostream &out, TaggedSentence &tagged_sent, Sentence &full_sent,
    std::vector<bool> &flushes) const {
  TaggedSentence::const_iterator ts_it = tagged_sent.begin();

  for (size_t full_idx = 0; full_idx < full_sent.size(); full_idx++) {
    StreamedType &token = full_sent[full_idx];
    out << token.TheString;
    if (!token.TheLexicalUnit) {
      if (flushes[full_idx]) {
        out.flush();
      }
      continue;
    }
    outputLexicalUnit(*token.TheLexicalUnit, *(ts_it++), out);
  }
}

TrainingCorpus::TrainingCorpus(Stream &tagged, Stream &untagged)
{
  TrainingSentence *training_sentence;
  bool was_sentence_end = true;
  while (1) {
    StreamedType tagged_token = tagged.get();
    StreamedType untagged_token = untagged.get();
    if (!tagged_token.TheLexicalUnit || !untagged_token.TheLexicalUnit) {
      if (tagged_token.TheLexicalUnit || untagged_token.TheLexicalUnit) {
        std::stringstream what_;
        what_ << "One stream has ended prematurely. "
              << "Please check if they are aligned.\n";
        throw Exception::UnalignedStreams(what_);
      }
      break;
    }
    if (was_sentence_end) {
      sentences.push_back(std::make_pair(0, 0));
      training_sentence = &sentences.back();
      was_sentence_end = false;
    }
    std::vector<Analysis> &analyses = tagged_token.TheLexicalUnit->TheAnalyses;
    if (analyses.empty()) {
      training_sentence->first.push_back(Optional<Analysis>());
    } else {
      training_sentence->first.push_back(analyses.front());
    }
    training_sentence->second.push_back(untagged_token);
    if (isSentenceEnd(tagged_token)) {
      was_sentence_end = true;
    }
  }
}

void TrainingCorpus::shuffle()
{
  random_shuffle(sentences.begin(), sentences.end());
}

}
}
