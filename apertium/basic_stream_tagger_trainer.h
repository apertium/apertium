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

#ifndef BASIC_STREAM_TAGGER_TRAINER_H
#define BASIC_STREAM_TAGGER_TRAINER_H

#include "stream_tagger_trainer.h"
#include "stream.h"

#include <ostream>

namespace Apertium {
class basic_StreamTaggerTrainer : public StreamTaggerTrainer {
public:
  virtual ~basic_StreamTaggerTrainer();
  virtual void train(Stream &TaggedCorpus);
  virtual void serialise(std::ostream &Serialised_basic_Tagger) const = 0;

protected:
  basic_StreamTaggerTrainer();
  virtual void train_Analysis(const Analysis &Analysis_,
                              const std::size_t &Coefficient_) = 0;
  virtual void
  multiplyModel(const std::size_t &OccurrenceCoefficientMultiplier) = 0;
  std::size_t OccurrenceCoefficient;
};
}

#endif // BASIC_STREAM_TAGGER_TRAINER_H
