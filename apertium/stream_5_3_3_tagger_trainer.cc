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

#include "analysis.h"
#include "i.h"
#include "lemma.h"
#include "serialiser.h"
#include "stream_5_3_3_tagger_trainer.h"

#include <cstddef>
#include <map>
#include <ostream>
#include <utility>
#include <vector>

namespace Apertium {
Stream_5_3_3_TaggerTrainer::Stream_5_3_3_TaggerTrainer(const Flags &Flags_)
    : basic_Tagger(Flags_) {}

void Stream_5_3_3_TaggerTrainer::serialise(
    std::ostream &Serialised_basic_Tagger) const {
  ::serialise(Model, Serialised_basic_Tagger);
}

void
Stream_5_3_3_TaggerTrainer::train_Analysis(const Analysis &Analysis_,
                                           const std::size_t &Coefficient_) {
  Model.first.insert(
                  std::make_pair(i(Analysis_), std::map<Lemma, std::size_t>()))
      .first->second.insert(std::make_pair(Lemma(Analysis_), 0))
      .first->second += Coefficient_;

  for (std::vector<Morpheme>::const_iterator Morpheme_ =
           Analysis_.TheMorphemes.begin() + 1;
       Morpheme_ != Analysis_.TheMorphemes.end(); ++Morpheme_) {
    Model.second.first.insert(std::make_pair(i(*(Morpheme_ - 1)),
                                             std::map<Lemma, std::size_t>()))
        .first->second.insert(std::make_pair(Lemma(*Morpheme_), 0))
        .first->second += Coefficient_;
    Model.second.second.insert(std::make_pair(Lemma(*Morpheme_),
                                              std::map<i, std::size_t>()))
        .first->second.insert(std::make_pair(i(*Morpheme_), 0))
        .first->second += Coefficient_;
  }
}

void Stream_5_3_3_TaggerTrainer::multiplyModel(
    const std::size_t &OccurrenceCoefficientMultiplier) {
  for (std::map<i, std::map<Lemma, std::size_t> >::iterator i_ =
           Model.first.begin();
       i_ != Model.first.end(); ++i_) {
    for (std::map<Lemma, std::size_t>::iterator Lemma_ = i_->second.begin();
         Lemma_ != i_->second.end(); ++Lemma_) {
      Lemma_->second *= OccurrenceCoefficientMultiplier;
    }
  }

  for (std::map<i, std::map<Lemma, std::size_t> >::iterator i_ =
           Model.second.first.begin();
       i_ != Model.second.first.end(); ++i_) {
    for (std::map<Lemma, std::size_t>::iterator Lemma_ = i_->second.begin();
         Lemma_ != i_->second.end(); ++Lemma_) {
      Lemma_->second *= OccurrenceCoefficientMultiplier;
    }
  }

  for (std::map<Lemma, std::map<i, std::size_t> >::iterator Lemma_ =
           Model.second.second.begin();
       Lemma_ != Model.second.second.end(); ++Lemma_) {
    for (std::map<i, std::size_t>::iterator i_ = Lemma_->second.begin();
         i_ != Lemma_->second.end(); ++i_) {
      i_->second *= OccurrenceCoefficientMultiplier;
    }
  }
}
}
