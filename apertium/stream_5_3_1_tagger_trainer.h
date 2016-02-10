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

#ifndef STREAM_5_3_1_TAGGER_TRAINER_H
#define STREAM_5_3_1_TAGGER_TRAINER_H

#include "basic_5_3_1_tagger.h"
#include "basic_stream_tagger_trainer.h"

#include "analysis.h"
#include "stream.h"

#include <ostream>

namespace Apertium {
class Stream_5_3_1_TaggerTrainer : private basic_5_3_1_Tagger,
                                   public basic_StreamTaggerTrainer {
public:
  Stream_5_3_1_TaggerTrainer(const Flags &Flags_);
  void serialise(std::ostream &Serialised_basic_Tagger) const;

private:
  void train_Analysis(const Analysis &Analysis_,
                      const std::size_t &Coefficient_);
  void multiplyModel(const std::size_t &OccurrenceCoefficientMultiplier);
};
}

#endif // STREAM_5_3_1_TAGGER_TRAINER_H
