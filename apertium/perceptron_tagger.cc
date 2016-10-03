#include <apertium/mtx_reader.h>
#include <apertium/perceptron_tagger.h>
#include <apertium/perceptron_spec.h>
#include <apertium/wchar_t_exception.h>
#include <algorithm>
#include <map>
#include <set>

namespace Apertium {

PerceptronTagger::PerceptronTagger(basic_Tagger::Flags flags) : basic_Tagger(flags) {};

PerceptronTagger::~PerceptronTagger() {};

void PerceptronTagger::tag(Stream &in, std::wostream &out) const {
  SentenceStream::SentenceTagger::tag(in, out, TheFlags.getSentSeg());
}

void PerceptronTagger::read_spec(const std::string &filename) {
  MTXReader(spec).read(filename);
}

std::wostream &
operator<<(std::wostream &out, PerceptronTagger const &pt) {
  out << "== Spec ==\n";
  out << pt.spec;
  out << "== Weights " << pt.weights.size() << " ==\n";
  out << pt.weights;
  return out;
}

TaggedSentence
PerceptronTagger::tagSentence(const Sentence &untagged_sent) const {
  const size_t sent_len = untagged_sent.size();

  std::vector<AgendaItem> agenda;
  agenda.reserve(spec.beam_width);
  agenda.push_back(AgendaItem());
  agenda.back().tagged.reserve(sent_len);

  UnaryFeatureVec feat_vec_delta;
  std::vector<Analysis>::const_iterator analys_it;
  std::vector<AgendaItem>::const_iterator agenda_it;
  std::vector<Morpheme>::const_iterator wordoid_it;

  for (size_t token_idx = 0; token_idx < sent_len; token_idx++) {
    const std::vector<Analysis> &analyses =
        untagged_sent[token_idx].TheLexicalUnit->TheAnalyses;

    std::vector<AgendaItem> new_agenda;
    new_agenda.reserve(spec.beam_width * analyses.size());

    if (analyses.size() == 1) {
      extendAgendaAll(agenda, analyses.front());
      continue;
    } else if (analyses.size() == 0) {
      extendAgendaAll(agenda, Optional<Analysis>());
      continue;
    }

    for (agenda_it = agenda.begin(); agenda_it != agenda.end(); agenda_it++) {
      for (analys_it = analyses.begin(); analys_it != analyses.end(); analys_it++) {
        const std::vector<Morpheme> &wordoids = analys_it->TheMorphemes;

        new_agenda.push_back(*agenda_it);
        AgendaItem &new_agenda_item = new_agenda.back();
        new_agenda_item.tagged.push_back(*analys_it);

        for (wordoid_it = wordoids.begin(); wordoid_it != wordoids.end(); wordoid_it++) {
          int wordoid_idx = wordoid_it - wordoids.begin();
          feat_vec_delta.clear();
          spec.get_features(new_agenda_item.tagged, untagged_sent,
                            token_idx, wordoid_idx, feat_vec_delta);
          if (TheFlags.getDebug()) {
            FeatureVec fv(feat_vec_delta);
            std::wcerr << "Token " << token_idx << "\t\tWordoid " << wordoid_idx << "\n";
            std::wcerr << fv;
            std::wcerr << "Score: " << weights * feat_vec_delta << "\n";
          }
          new_agenda_item.score += weights * feat_vec_delta;
        }
      }
    }
    // Apply the beam
    if (TheFlags.getDebug()) {
      std::wcerr << "-- Before beam: --\n" << new_agenda;
    }
    size_t new_agenda_size = std::min((size_t)spec.beam_width, new_agenda.size());
    agenda.resize(new_agenda_size);
    std::partial_sort_copy(new_agenda.begin(), new_agenda.end(),
                           agenda.begin(), agenda.end());
    if (TheFlags.getDebug()) {
      std::wcerr << "-- After beam: --\n" << agenda;
    }
  }

  spec.clearCache();
  return agenda.front().tagged;
}

void PerceptronTagger::outputLexicalUnit(
    const LexicalUnit &lexical_unit, const Optional<Analysis> analysis,
    std::wostream &output) const {
  StreamTagger::outputLexicalUnit(lexical_unit, analysis, output);
}

bool PerceptronTagger::trainSentence(
    const TrainingSentence &sentence,
    FeatureVecAverager &avg_weights)
{
  const TaggedSentence &tagged_sent = sentence.first;
  const Sentence &untagged_sent = sentence.second;
  assert(tagged_sent.size() == untagged_sent.size());
  const size_t sent_len = tagged_sent.size();

  std::vector<TrainingAgendaItem> agenda;
  agenda.reserve(spec.beam_width);
  agenda.push_back(TrainingAgendaItem());
  agenda.back().tagged.reserve(sent_len);
  std::vector<TrainingAgendaItem>::const_iterator correct_agenda_it
      = agenda.begin();

  TrainingAgendaItem correct_sentence;
  correct_sentence.tagged.reserve(sent_len);

  UnaryFeatureVec feat_vec_delta;
  std::vector<Analysis>::const_iterator analys_it;
  std::vector<TrainingAgendaItem>::const_iterator agenda_it;
  std::vector<Morpheme>::const_iterator wordoid_it;

  for (size_t token_idx = 0; token_idx < sent_len; token_idx++) {
    //std::wcerr << "Token idx: " << token_idx << "\n";
    const TaggedToken &tagged_tok(tagged_sent[token_idx]);
    const StreamedType &untagged_tok(untagged_sent[token_idx]);
    correct_sentence.tagged.push_back(tagged_tok);

    const std::vector<Analysis> &analyses =
        untagged_tok.TheLexicalUnit->TheAnalyses;

    std::vector<TrainingAgendaItem> new_agenda;
    new_agenda.reserve(spec.beam_width * analyses.size());

    if (analyses.size() <= 1 || !tagged_tok) {
      // Case |analyses| = 0, nothing we can do
      // Case !tagged_tok, |analyses| > 0, no point penalising a guess which
      //   can only be incorrect when there's no correct answer
      // Case |analyses| = 1, everything will cancel out anyway
      if (analyses.size() == 1) {
        extendAgendaAll(agenda, analyses.front());
        continue;
      } else {
        extendAgendaAll(agenda, Optional<Analysis>());
        continue;
      }
    }

    bool correct_available = false;
    for (agenda_it = agenda.begin(); agenda_it != agenda.end(); agenda_it++) {
      //std::wcerr << *agenda_it;
      for (analys_it = analyses.begin(); analys_it != analyses.end(); analys_it++) {
        const std::vector<Morpheme> &wordoids = analys_it->TheMorphemes;

        new_agenda.push_back(*agenda_it);
        TrainingAgendaItem &new_agenda_item = new_agenda.back();
        new_agenda_item.tagged.push_back(*analys_it);

        for (wordoid_it = wordoids.begin(); wordoid_it != wordoids.end(); wordoid_it++) {
          int wordoid_idx = wordoid_it - wordoids.begin();
          feat_vec_delta.clear();
          spec.get_features(new_agenda_item.tagged, untagged_sent,
                            token_idx, wordoid_idx, feat_vec_delta);
          new_agenda_item.vec += feat_vec_delta;
          new_agenda_item.score += weights * feat_vec_delta;
          if (agenda_it == correct_agenda_it && *analys_it == *tagged_tok) {
            correct_sentence = new_agenda_item;
            correct_available = true;
          }
        }
      }
    }
    if (!correct_available) {
      if (TheFlags.getSkipErrors()) {
        return true;
      } else {
        std::wstringstream what_;
        what_ << L"Tagged analysis unavailable in untagged/ambigous input.\n";
        what_ << L"Available:\n";
        for (analys_it = analyses.begin(); analys_it != analyses.end(); analys_it++) {
          what_ << *analys_it << L"\n";
        }
        what_ << L"Required: " << *tagged_tok << L"\n";
        what_ << L"Rerun with --skip-on-error to skip this sentence.";
        throw Apertium::wchar_t_Exception::PerceptronTagger::CorrectAnalysisUnavailable(what_);
      }
    }
    // Apply the beam
    //std::wcerr << "-- Before beam: --\n" << new_agenda;
    size_t new_agenda_size = std::min((size_t)spec.beam_width, new_agenda.size());
    agenda.resize(new_agenda_size);
    std::partial_sort_copy(new_agenda.begin(), new_agenda.end(),
                           agenda.begin(), agenda.end());
    //std::wcerr << "-- After beam: --\n" << agenda;

    // Early update "fallen off the beam"
    bool any_match = false;
    for (agenda_it = agenda.begin(); agenda_it != agenda.end(); agenda_it++) {
      if (agenda_it->tagged == correct_sentence.tagged) {
        correct_agenda_it = agenda_it;
        any_match = true;
        break;
      }
    }
    if (!any_match) {
      /*std::wcerr << "Early update time!\n";
      std::wcerr << "Before:\n" << weights << "\n";
      std::wcerr << "Incorrect:\n" << agenda.front().vec << "\n";
      std::wcerr << "Correct:\n" << correct_sentence.vec << "\n";*/
      avg_weights -= agenda.front().vec;
      avg_weights += correct_sentence.vec;
      avg_weights.incIteration();
      //std::wcerr << "After:\n" << weights << "\n";
      return false;
    }
  }
  // Normal update
  /*std::wcerr << "Best match:\n" << agenda.front().tagged << "\n\n";
  std::wcerr << "Correct:\n" << correct_sentence.tagged << "\n\n";*/
  if (agenda.front().tagged != correct_sentence.tagged) {
    /*std::wcerr << "Normal update time!\n";
    std::wcerr << "Before:\n" << weights << "\n";
    std::wcerr << "Incorrect:\n" << agenda.front().vec << "\n";
    std::wcerr << "Correct:\n" << correct_sentence.vec << "\n";*/
    avg_weights -= agenda.front().vec;
    avg_weights += correct_sentence.vec;
    avg_weights.incIteration();
    //std::wcerr << "After:\n" << weights << "\n";
  }
  return false;
}

void PerceptronTagger::train(Stream&) {}  // dummy

void PerceptronTagger::train(
    Stream &tagged,
    Stream &untagged,
    int iterations) {
  FeatureVecAverager avg_weights(weights);
  TrainingCorpus tc(tagged, untagged, TheFlags.getSkipErrors(), TheFlags.getSentSeg());
  size_t avail_skipped;
  for (int i = 0; i < iterations; i++) {
    std::wcerr << "Iteration " << i + 1 << " of " << iterations << "\n";
    avail_skipped = 0;
    tc.shuffle();
    std::vector<TrainingSentence>::const_iterator si;
    for (si = tc.sentences.begin(); si != tc.sentences.end(); si++) {
      avail_skipped += trainSentence(*si, avg_weights);
      spec.clearCache();
    }
  }
  avg_weights.average();
  if (avail_skipped) {
    std::wcerr << "Skipped " << tc.skipped << " sentences due to token "
               << "misalignment and " << avail_skipped << " sentences due to "
               << "tagged token being unavailable in untagged file out of "
               << tc.sentences.size() << " total sentences.\n";
  }
  //std::wcerr << *this;
}

void PerceptronTagger::serialise(std::ostream &serialised) const
{
  spec.serialise(serialised);
  weights.serialise(serialised);
};

void PerceptronTagger::deserialise(std::istream &serialised)
{
  spec.deserialise(serialised);
  weights.deserialise(serialised);
};

template <typename T> void
PerceptronTagger::extendAgendaAll(
    std::vector<T> &agenda,
    Optional<Analysis> analy) {
  typename std::vector<T>::iterator agenda_it;
  for (agenda_it = agenda.begin(); agenda_it != agenda.end(); agenda_it++) {
    agenda_it->tagged.push_back(analy);
  }
}

std::wostream&
operator<<(std::wostream &out, const TaggedSentence &tagged) {
  TaggedSentence::const_iterator tsi;
  for (tsi = tagged.begin(); tsi != tagged.end(); tsi++) {
    if (*tsi) {
      out << **tsi;
    } else {
      out << "*";
    }
    out << " ";
  }
  return out;
}

std::wostream&
operator<<(std::wostream &out, const PerceptronTagger::TrainingAgendaItem &tai) {
  out << "Score: " << tai.score << "\n";
  out << "Sentence: " << tai.tagged << "\n";
  out << "\n";
  out << "Vector:\n" << tai.vec;
  return out;
}

std::wostream&
operator<<(std::wostream &out, const std::vector<PerceptronTagger::TrainingAgendaItem> &agenda) {
  std::vector<PerceptronTagger::TrainingAgendaItem>::const_iterator agenda_it;
  for (agenda_it = agenda.begin(); agenda_it != agenda.end(); agenda_it++) {
    out << *agenda_it;
  }
  out << "\n\n";
  return out;
}

std::wostream&
operator<<(std::wostream &out, const PerceptronTagger::AgendaItem &ai) {
  out << "Score: " << ai.score << "\n";
  out << "Sentence: " << ai.tagged << "\n";
  return out;
}

std::wostream&
operator<<(std::wostream &out, const std::vector<PerceptronTagger::AgendaItem> &agenda) {
  std::vector<PerceptronTagger::AgendaItem>::const_iterator agenda_it;
  for (agenda_it = agenda.begin(); agenda_it != agenda.end(); agenda_it++) {
    out << *agenda_it;
  }
  out << "\n\n";
  return out;
}

bool operator<(const PerceptronTagger::AgendaItem &a,
               const PerceptronTagger::AgendaItem &b) {
  return a.score > b.score;
};
}
