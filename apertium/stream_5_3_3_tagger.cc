// Copyright (C) 2005 Universitat d'Alacant / Universidad de Alicante
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

#include "stream_5_3_3_tagger.h"

#include "apertium_config.h"

#include "analysis.h"
#include "deserialiser.h"
#include "i.h"
#include "lemma.h"
#include "morpheme.h"

#include <vector>

#if ENABLE_DEBUG

#include <sstream>
#include <string>

#endif // ENABLE_DEBUG

namespace Apertium {
Stream_5_3_3_Tagger::Stream_5_3_3_Tagger(const Flags &Flags_)
    : basic_StreamTagger(Flags_) {}

void Stream_5_3_3_Tagger::deserialise(std::istream &Serialised_basic_Tagger) {
  Model = Deserialiser<
      std::pair<std::map<i, std::map<Lemma, std::size_t> >,
                std::pair<std::map<i, std::map<Lemma, std::size_t> >,
                          std::map<Lemma, std::map<i, std::size_t> > > > >::
      deserialise(Serialised_basic_Tagger);
}

long double Stream_5_3_3_Tagger::score(const Analysis &Analysis_) const {
  long double score = tokenCount_r_i(Analysis_) * tokenCount_i(Analysis_),
              score_Divisor = tokenCount_i(Analysis_) + typeCount_i(Analysis_);

  for (std::vector<Morpheme>::const_iterator Morpheme_ =
           Analysis_.TheMorphemes.begin() + 1;
       Morpheme_ != Analysis_.TheMorphemes.end(); ++Morpheme_) {
    score *= tokenCount_d_i_Morpheme(Lemma(*Morpheme_), i(*(Morpheme_ - 1))) *
             tokenCount_i_d_Morpheme(i(*Morpheme_), Lemma(*Morpheme_));
    score_Divisor *=
        (tokenCount_i_Morpheme(i(*(Morpheme_ - 1))) +
         typeCount_i_Morpheme(i(*(Morpheme_ - 1)), Lemma(*Morpheme_))) *
        (tokenCount_d_Morpheme(Lemma(*Morpheme_)) +
         typeCount_d_Morpheme(Lemma(*Morpheme_), i(*Morpheme_)));
  }

  return score / score_Divisor;
}

long double
Stream_5_3_3_Tagger::tokenCount_r_i(const Analysis &Analysis_) const {
  if (Model.first.find(i(Analysis_)) == Model.first.end())
    return 1;

  if (Model.first.find(i(Analysis_))->second.find(Lemma(Analysis_)) ==
      Model.first.find(i(Analysis_))->second.end())
    return 1;

  return 1 +
         Model.first.find(i(Analysis_))->second.find(Lemma(Analysis_))->second;
}

long double Stream_5_3_3_Tagger::tokenCount_i(const Analysis &Analysis_) const {
  if (Model.first.find(i(Analysis_)) == Model.first.end())
    return 1;

  long double tokenCount_i_ = 1;

  for (std::map<Lemma, std::size_t>::const_iterator Lemma_ =
           Model.first.find(i(Analysis_))->second.begin();
       Lemma_ != Model.first.find(i(Analysis_))->second.end(); ++Lemma_) {
    tokenCount_i_ += Lemma_->second;
  }

  return tokenCount_i_;
}

long double Stream_5_3_3_Tagger::typeCount_i(const Analysis &Analysis_) const {
  if (Model.first.find(i(Analysis_)) == Model.first.end())
    return 1;

  return (Model.first.find(i(Analysis_))->second.find(Lemma(Analysis_)) ==
                  Model.first.find(i(Analysis_))->second.end()
              ? 1
              : 0) +
         Model.first.find(i(Analysis_))->second.size();
}

long double Stream_5_3_3_Tagger::tokenCount_d_i_Morpheme(const Lemma &Lemma_,
                                                         const i &i_) const {
  if (Model.second.first.find(i_) == Model.second.first.end())
    return 1;

  if (Model.second.first.find(i_)->second.find(Lemma_) ==
      Model.second.first.find(i_)->second.end())
    return 1;

  return 1 + Model.second.first.find(i_)->second.find(Lemma_)->second;
}

long double
Stream_5_3_3_Tagger::tokenCount_i_d_Morpheme(const i &i_,
                                             const Lemma &Lemma_) const {
  if (Model.second.second.find(Lemma_) == Model.second.second.end())
    return 1;

  if (Model.second.second.find(Lemma_)->second.find(i_) ==
      Model.second.second.find(Lemma_)->second.end())
    return 1;

  return 1 + Model.second.second.find(Lemma_)->second.find(i_)->second;
}

long double Stream_5_3_3_Tagger::tokenCount_i_Morpheme(const i &i_) const {
  if (Model.second.first.find(i_) == Model.second.first.end())
    return 1;

  long double typeCount_i_Morpheme_ = 1;

  for (std::map<Lemma, std::size_t>::const_iterator Lemma_ =
           Model.second.first.find(i_)->second.begin();
       Lemma_ != Model.second.first.find(i_)->second.end(); ++Lemma_) {
    typeCount_i_Morpheme_ += Lemma_->second;
  }

  return typeCount_i_Morpheme_;
}

long double
Stream_5_3_3_Tagger::typeCount_i_Morpheme(const i &i_,
                                          const Lemma &Lemma_) const {
  if (Model.second.first.find(i_) == Model.second.first.end())
    return 1;

  return (Model.second.first.find(i_)->second.find(Lemma_) ==
                  Model.second.first.find(i_)->second.end()
              ? 1
              : 0) +
         Model.second.first.find(i_)->second.size();
}

long double
Stream_5_3_3_Tagger::tokenCount_d_Morpheme(const Lemma &Lemma_) const {
  if (Model.second.second.find(Lemma_) == Model.second.second.end())
    return 1;

  long double tokenCount_d_Morpheme_ = 1;

  for (std::map<i, std::size_t>::const_iterator i_ =
           Model.second.second.find(Lemma_)->second.begin();
       i_ != Model.second.second.find(Lemma_)->second.end(); ++i_) {
    tokenCount_d_Morpheme_ += i_->second;
  }

  return tokenCount_d_Morpheme_;
}

long double Stream_5_3_3_Tagger::typeCount_d_Morpheme(const Lemma &Lemma_,
                                                      const i &i_) const {
  if (Model.second.second.find(Lemma_) == Model.second.second.end())
    return 1;

  return (Model.second.second.find(Lemma_)->second.find(i_) ==
                  Model.second.second.find(Lemma_)->second.end()
              ? 1
              : 0) +
         Model.second.second.find(Lemma_)->second.size();
}

#if ENABLE_DEBUG

std::wstring Stream_5_3_3_Tagger::score_DEBUG(const Analysis &Analysis_) const {
  std::wstringstream score_DEBUG_;

  score_DEBUG_ << L"(" << tokenCount_r_i(Analysis_) << L" * "
               << tokenCount_i(Analysis_);

  for (std::vector<Morpheme>::const_iterator Morpheme_ =
           Analysis_.TheMorphemes.begin() + 1;
       Morpheme_ != Analysis_.TheMorphemes.end(); ++Morpheme_) {
    score_DEBUG_ << L" * " << tokenCount_d_i_Morpheme(Lemma(*Morpheme_),
                                                      i(*(Morpheme_ - 1)))
                 << L" * "
                 << tokenCount_i_d_Morpheme(i(*Morpheme_), Lemma(*Morpheme_));
  }

  score_DEBUG_ << L") /\n    [(" << tokenCount_i(Analysis_) << L" + "
               << typeCount_i(Analysis_) << L")";

  for (std::vector<Morpheme>::const_iterator Morpheme_ =
           Analysis_.TheMorphemes.begin() + 1;
       Morpheme_ != Analysis_.TheMorphemes.end(); ++Morpheme_) {
    score_DEBUG_ << L" * (" << tokenCount_i_Morpheme(i(*(Morpheme_ - 1)))
                 << L" + "
                 << typeCount_i_Morpheme(i(*(Morpheme_ - 1)), Lemma(*Morpheme_))
                 << L") * (" << tokenCount_d_Morpheme(Lemma(*Morpheme_))
                 << L" + "
                 << typeCount_d_Morpheme(Lemma(*Morpheme_), i(*Morpheme_))
                 << L")";
  }

  score_DEBUG_ << L"]";
  return score_DEBUG_.str();
}

#endif // ENABLE_DEBUG
}
