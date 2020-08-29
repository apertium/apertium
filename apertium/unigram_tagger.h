// Copyright (C) 2020 Universitat d'Alacant / Universidad de Alicante
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 2 of the
// License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.

#ifndef UNIGRAM_TAGGER_H
#define UNIGRAM_TAGGER_H

#include "stream.h"
#include "stream_tagger.h"
#include "stream_tagger_trainer.h"
#include <istream>
#include <ostream>

#include "analysis.h"
#include "a.h"
#include "i.h"
#include "lemma.h"

#include <cstddef>
#include <map>
#include <utility>
#include <string>
#include <sstream>


namespace Apertium {

enum UnigramTaggerModel {
  UnigramTaggerModelUnknown = 0,
  UnigramTaggerModel1 = 1,
  UnigramTaggerModel2 = 2,
  UnigramTaggerModel3 = 3
};

class UnigramTagger : public StreamTagger, public StreamTaggerTrainer {
private:
  long double model3_score(const Analysis &Analysis_);
  void tag(const LexicalUnit &LexicalUnit_, std::wostream &Output);
  std::wstringstream score_DEBUG;
protected:

  UnigramTaggerModel model;

  std::map<Analysis, std::size_t> Model1;

  std::map<a, std::map<Lemma, std::size_t> > Model2;

  std::pair<std::map<i, std::map<Lemma, std::size_t> >,
            std::pair<std::map<i, std::map<Lemma, std::size_t> >,
                      std::map<Lemma, std::map<i, std::size_t> > > > Model3;

  long double score(const Analysis& Analysis_);

  void train_Analysis(const Analysis& Analysis_, const std::size_t& Coefficient_);
  void multiplyModel(const std::size_t& OccurrenceCoefficientMultiplier);
  std::size_t OccurrenceCoefficient;

public:
  UnigramTagger(Flags& Flags_);
  ~UnigramTagger();
  void setModel(const UnigramTaggerModel& m);
  UnigramTaggerModel getModel();
  void serialise(std::ostream& o) const;
  void deserialise(std::istream& s);
  void tag(Stream& Input, std::wostream& Output);
  void train(Stream& TaggedCorpus);
};
}

#endif // UNIGRAM_TAGGER_H
