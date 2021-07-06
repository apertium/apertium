#include <apertium/sentence_stream.h>

#include <apertium/exception.h>
#include <iostream>
#include <algorithm>

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
  std::vector<UString> &tags = morphemes.begin()->TheTags;
  if (tags.size() != 1) {
    return false;
  }
  return (*tags.begin() == "sent"_u);
}

bool isSentenceEnd(StreamedType tok, Stream &in, bool sent_seg) {
  if (sent_seg) {
    return isSentenceEnd(tok) && in.peekIsBlank();
  } else {
    return isSentenceEnd(tok);
  }
}

SentenceTagger::SentenceTagger() {}

void SentenceTagger::tag(Stream &in, std::ostream &out, bool sent_seg) {
  clearBuffers();

  while (true) {
    StreamedType token = in.get();
    full_sent.push_back(token);
    flushes.push_back(in.flush_());

    if (token.TheLexicalUnit) {
      lexical_sent.push_back(token);
      if (isSentenceEnd(token, in, sent_seg)) {
        tagAndPutSentence(out);
      }
    } else {
      tagAndPutSentence(out);
      if (in.flush_()) {
        clearBuffers();
      } else {
        break;
      }
    }
  }
}

void SentenceTagger::clearBuffers() const {
  full_sent.clear();
  lexical_sent.clear();
  flushes.clear();
}

void SentenceTagger::tagAndPutSentence(std::ostream &out) {
  TaggedSentence tagged_sent = tagSentence(lexical_sent);
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
  clearBuffers();
}

TrainingCorpus::TrainingCorpus(Stream &tagged, Stream &untagged,
                               bool skip_on_error, bool sent_seg)
  : sent_seg(sent_seg), skipped(0)
{
  std::random_device rd;
  rnd.seed(rd());

  TrainingSentence *training_sentence;
  bool was_sentence_end = true;
  unsigned int tagged_line = 0;
  unsigned int untagged_line = 0;
  while (1) {
    StreamedType tagged_token = tagged.get();
    StreamedType untagged_token = untagged.get();
    tagged_line++;
    untagged_line++;
    if (!tagged_token.TheLexicalUnit || !untagged_token.TheLexicalUnit) {
      if (tagged_token.TheLexicalUnit || untagged_token.TheLexicalUnit) {
        std::cerr << "Normal perm\n";
        std::cerr << "tagged: " << tagged_line << " " << (!!tagged_token.TheLexicalUnit) << "\n";
        std::cerr << "untagged: " << untagged_line << " " << (!!untagged_token.TheLexicalUnit) << "\n";
        prematureEnd();
      }
      break;
    }
    //std::cerr << tagged_token.TheLexicalUnit->TheSurfaceForm << " || " << untagged_token.TheLexicalUnit->TheSurfaceForm << "\n";
    if (untagged_token.TheLexicalUnit->TheSurfaceForm != tagged_token.TheLexicalUnit->TheSurfaceForm) {
      if (!skip_on_error) {
        std::stringstream what_;
        what_ << "Streams diverged at line " << tagged_line << "\n";
        what_ << "Untagged token: "
              << untagged_token.TheLexicalUnit->TheSurfaceForm << "\n";
        what_ << "Tagged token: "
              << tagged_token.TheLexicalUnit->TheSurfaceForm << "\n";
        what_ << "Rerun with --skip-on-error to skip this sentence.";
        throw Exception::UnalignedStreams(what_);
      }

      skipped++;
      training_sentence->first.clear();
      training_sentence->second.clear();

      std::cerr << "fast forward\n";
      bool tagged_ended = contToEndOfSent(tagged, tagged_token, tagged_line);
      bool untagged_ended = contToEndOfSent(untagged, untagged_token, untagged_line);
      if (tagged_ended || untagged_ended) {
        if (!tagged_ended || !untagged_ended) {
          std::cerr << "fast forward prem\n";
          prematureEnd();
        }
        std::cerr << "fast forward finish\n";
        break;
      }
      std::cerr << "fast forwarded\n";
      continue;
    }
    if (was_sentence_end) {
      sentences.push_back(std::make_pair(TaggedSentence(), Sentence()));
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
    if (isSentenceEnd(tagged_token, tagged, sent_seg)) {
      was_sentence_end = true;
    }
  }
}

bool TrainingCorpus::contToEndOfSent(Stream &stream, StreamedType token,
                                     unsigned int &line)
{
  while (1) {
    if (!token.TheLexicalUnit) {
      return true;
    }
    if (isSentenceEnd(token, stream, sent_seg)) {
      return false;
    }
    std::cerr << "Skip " << token.TheLexicalUnit->TheSurfaceForm << "\n";
    token = stream.get();
    line++;
  }
}

void TrainingCorpus::prematureEnd()
{
  std::stringstream what_;
  what_ << "One stream has ended prematurely. "
        << "Please check if they are aligned.\n";
  throw Exception::UnalignedStreams(what_);
}

void TrainingCorpus::shuffle()
{
  std::shuffle(sentences.begin(), sentences.end(), rnd);
}

}
}
