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

#include "stream_5_3_2_tagger.h"

#include "apertium_config.h"

#include "a.h"
#include "analysis.h"
#include "deserialiser.h"
#include "lemma.h"

#include <cstddef>
#include <istream>
#include <map>

#if ENABLE_DEBUG

#include <sstream>
#include <string>

#endif // ENABLE_DEBUG

namespace Apertium {
Stream_5_3_2_Tagger::Stream_5_3_2_Tagger(const Flags &Flags_)
    : basic_Tagger(Flags_), basic_5_3_2_Tagger() {}

void Stream_5_3_2_Tagger::deserialise(std::istream &Serialised_basic_Tagger) {
  Model =
      Deserialiser<std::map<a, std::map<Lemma, std::size_t> > >::deserialise(
          Serialised_basic_Tagger);
}

long double Stream_5_3_2_Tagger::score(const Analysis &Analysis_) const {
  return (tokenCount_r_a(Analysis_) * tokenCount_a(Analysis_)) /
         (tokenCount_a(Analysis_) + typeCount_a(Analysis_));
}

long double
Stream_5_3_2_Tagger::tokenCount_r_a(const Analysis &Analysis_) const {
  if (Model.find(a(Analysis_)) == Model.end())
    return 1;

  if (Model.find(a(Analysis_))->second.find(Lemma(Analysis_)) ==
      Model.find(a(Analysis_))->second.end())
    return 1;

  return 1 + Model.find(a(Analysis_))->second.find(Lemma(Analysis_))->second;
}

long double Stream_5_3_2_Tagger::tokenCount_a(const Analysis &Analysis_) const {
  if (Model.find(a(Analysis_)) == Model.end())
    return 1;

  long double tokenCount_a_ = 1;

  for (std::map<Lemma, std::size_t>::const_iterator Lemma_ =
           Model.find(a(Analysis_))->second.begin();
       Lemma_ != Model.find(a(Analysis_))->second.end(); ++Lemma_) {
    tokenCount_a_ += Lemma_->second;
  }

  return tokenCount_a_;
}

long double Stream_5_3_2_Tagger::typeCount_a(const Analysis &Analysis_) const {
  if (Model.find(a(Analysis_)) == Model.end())
    return 1;

  return (Model.find(a(Analysis_))->second.find(Lemma(Analysis_)) ==
                  Model.find(a(Analysis_))->second.end()
              ? 1
              : 0) +
         Model.find(a(Analysis_))->second.size();
}

#if ENABLE_DEBUG

std::wstring Stream_5_3_2_Tagger::score_DEBUG(const Analysis &Analysis_) const {
  std::wstringstream score_DEBUG_;

  score_DEBUG_ << L"(" << tokenCount_r_a(Analysis_) << L" * "
               << tokenCount_a(Analysis_) << L") /\n    ("
               << tokenCount_a(Analysis_) << L" + " << typeCount_a(Analysis_)
               << L")";

  return score_DEBUG_.str();
}

#endif // ENABLE_DEBUG

}
