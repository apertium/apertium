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
// along with this program; if not, see <https://www.gnu.org/licenses/>.

#ifndef UNIGRAM_TAGGER_H
#define UNIGRAM_TAGGER_H

#include "stream.h"
#include "stream_tagger.h"
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

class UnigramTagger : public StreamTagger {
private:
  long double model3_score(const Analysis &Analysis_);
  void tag(const LexicalUnit &LexicalUnit_, std::wostream &Output);
  std::stringstream score_DEBUG;
protected:

  UnigramTaggerModel model;

  // Models are described in https://coltekin.net/cagri/papers/trmorph-tools.pdf
  // section 5.3

  // dar<v><inf>+se<prn>+lo<prn>

  // Analysis => count
  // P(analysis)
  // dar<v><inf>+se<prn>+lo<prn> => 1
  std::map<Analysis, std::size_t> Model1;

  // Analysis without lemma => lemma => count
  // P(lemma | tags)
  // <v><inf>+se<prn>+lo<prn> => dar => 1
  std::map<a, std::map<Lemma, std::size_t> > Model2;

  // First tag group => lemma => count
  // P(lemma | tags)
  // <v><inf> => dar => 1
  std::map<i, std::map<Lemma, std::size_t> > Model3_l_t;

  // Subsequent tag groups => lemma => count
  // P(compound lemma | previous tags)
  // <v><inf> => se => 1, <prn> => lo => 1
  std::map<i, std::map<Lemma, std::size_t> > Model3_cl_ct;

  // Lemma => first tag group => count
  // P(compound tags | corresponding lemma)
  // se => <prn> => 1, lo => <prn> => 1
  std::map<Lemma, std::map<i, std::size_t> > Model3_ct_cl;

  long double score(const Analysis& Analysis_);

  void train_Analysis(const Analysis& Analysis_, const std::size_t& Coefficient_);
  void multiplyModel(const std::size_t& OccurrenceCoefficientMultiplier);
  std::size_t OccurrenceCoefficient;

public:
  UnigramTagger(TaggerFlags& Flags_);
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
