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

#include "basic_stream_tagger_trainer.h"

#include "analysis.h"
#include "basic_tagger.h"
#include "exception.h"
#include "stream.h"
#include "streamed_type.h"

namespace Apertium {
basic_StreamTaggerTrainer::~basic_StreamTaggerTrainer() {}

void basic_StreamTaggerTrainer::train(Stream &TaggedCorpus) {
  while (true) {
    StreamedType StreamedType_ = TaggedCorpus.get();

    if (!StreamedType_.TheLexicalUnit)
      break;

    if (StreamedType_.TheLexicalUnit->TheAnalyses.empty())
      throw Exception::LexicalUnit::TheAnalyses_empty(
          "can't train LexicalUnit comprising empty Analysis std::vector");

    if (OccurrenceCoefficient %
            StreamedType_.TheLexicalUnit->TheAnalyses.size() !=
        0) {
      OccurrenceCoefficient *= StreamedType_.TheLexicalUnit->TheAnalyses.size();
      multiplyModel(StreamedType_.TheLexicalUnit->TheAnalyses.size());
    }

    for (std::vector<Analysis>::const_iterator Analysis_ =
             StreamedType_.TheLexicalUnit->TheAnalyses.begin();
         Analysis_ != StreamedType_.TheLexicalUnit->TheAnalyses.end();
         ++Analysis_) {
      train_Analysis(*Analysis_,
                     OccurrenceCoefficient /
                         StreamedType_.TheLexicalUnit->TheAnalyses.size());
    }
  }
}

basic_StreamTaggerTrainer::basic_StreamTaggerTrainer()
    : OccurrenceCoefficient(1) {}
}
