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

#include "stream_5_3_2_tagger_trainer.h"

#include "a.h"
#include "analysis.h"
#include "lemma.h"
#include "serialiser.h"

#include <map>
#include <ostream>
#include <utility>

namespace Apertium {
Stream_5_3_2_TaggerTrainer::Stream_5_3_2_TaggerTrainer(const Flags &Flags_)
    : basic_StreamTaggerTrainer(Flags_) {}

void Stream_5_3_2_TaggerTrainer::serialise(
    std::ostream &Serialised_basic_Tagger) const {
  Serialiser<std::map<a, std::map<Lemma, std::size_t> > >::serialise(
      Model, Serialised_basic_Tagger);
}

void
Stream_5_3_2_TaggerTrainer::train_Analysis(const Analysis &Analysis_,
                                           const std::size_t &Coefficient_) {
  Model.insert(std::make_pair(static_cast<a>(Analysis_),
                              std::map<Lemma, std::size_t>()))
      .first->second.insert(std::make_pair(static_cast<Lemma>(Analysis_), 0))
      .first->second += Coefficient_;
}

void Stream_5_3_2_TaggerTrainer::multiplyModel(
    const std::size_t &OccurrenceCoefficientMultiplier) {
  for (std::map<a, std::map<Lemma, std::size_t> >::iterator a_ = Model.begin();
       a_ != Model.end(); ++a_) {
    for (std::map<Lemma, std::size_t>::iterator r_ = a_->second.begin();
         r_ != a_->second.end(); ++r_) {
      r_->second *= OccurrenceCoefficientMultiplier;
    }
  }
}
}
