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

#include "lemma.h"

#include "analysis.h"
#include "exception.h"
#include "morpheme.h"

namespace Apertium {
bool operator==(const Lemma &a_, const Lemma &b_) {
  return a_.TheLemma == b_.TheLemma;
}

bool operator<(const Lemma &a_, const Lemma &b_) {
  return a_.TheLemma < b_.TheLemma;
}

Lemma::Lemma() : TheLemma() {}

Lemma::Lemma(const Analysis &Analysis_) : TheLemma() {
  if (Analysis_.TheMorphemes.empty())
    throw Exception::Analysis::TheMorphemes_empty(
        "can't convert const Analysis & comprising empty Morpheme std::vector "
        "to Lemma");

  if (Analysis_.TheMorphemes.front().TheLemma.empty())
    throw Exception::Morpheme::TheLemma_empty(
        "can't convert const Analysis & comprising Morpheme comprising empty "
        "Lemma std::wstring to Lemma");

  TheLemma = Analysis_.TheMorphemes.front().TheLemma;
}

Lemma::Lemma(const Morpheme &Morpheme_) : TheLemma() {
  if (Morpheme_.TheLemma.empty())
    throw Exception::Morpheme::TheLemma_empty("can't convert const Morpheme & "
                                              "comprising empty Lemma "
                                              "std::wstring to Lemma");

  TheLemma = Morpheme_.TheLemma;
}
}
