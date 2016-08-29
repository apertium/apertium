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

#ifndef APERTIUM_SERIALISER_H
#define APERTIUM_SERIALISER_H

#include "a.h"
#include "basic_exception_type.h"
#include "analysis.h"
#include "exception.h"
#include "i.h"
#include "lemma.h"
#include "morpheme.h"
#include "tag.h"

#include <lttoolbox/serialiser.h>
#include <stdint.h>
#include <apertium/perceptron_spec.h>
#include <cstddef>
#include <limits>
#include <ios>
#include <limits>
#include <map>
#include <ostream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

using namespace Apertium;

namespace {
template <> class Serialiser<a> {
public:
  inline static void serialise(const a &SerialisedType_, std::ostream &Output);
};

template <> class Serialiser<Analysis> {
public:
  inline static void serialise(const Analysis &SerialisedType_,
                               std::ostream &Output);
};

template <> class Serialiser<i> {
public:
  inline static void serialise(const i &SerialisedType_, std::ostream &Output);
};

template <> class Serialiser<Lemma> {
public:
  inline static void serialise(const Lemma &SerialisedType_,
                               std::ostream &Output);
};

template <> class Serialiser<Morpheme> {
public:
  inline static void serialise(const Morpheme &SerialisedType_,
                               std::ostream &Output);
};

template <> class Serialiser<Tag> {
public:
  inline static void serialise(const Tag &SerialisedType_,
                               std::ostream &Output);
};

}

void Serialiser<a>::serialise(const a &SerialisedType_, std::ostream &Output) {
  ::serialise(SerialisedType_.TheTags, Output);
  ::serialise(SerialisedType_.TheMorphemes, Output);
}

void Serialiser<Analysis>::serialise(const Analysis &SerialisedType_,
                                     std::ostream &Output) {
  ::serialise(SerialisedType_.TheMorphemes, Output);
}

void Serialiser<i>::serialise(const i &SerialisedType_, std::ostream &Output) {
  ::serialise(SerialisedType_.TheTags, Output);
}

void Serialiser<Lemma>::serialise(const Lemma &SerialisedType_,
                                  std::ostream &Output) {
  ::serialise(SerialisedType_.TheLemma, Output);
}

void Serialiser<Morpheme>::serialise(const Morpheme &SerialisedType_,
                                     std::ostream &Output) {
  ::serialise(SerialisedType_.TheLemma, Output);
  ::serialise(SerialisedType_.TheTags, Output);
}

void Serialiser<Tag>::serialise(const Tag &SerialisedType_,
                                std::ostream &Output) {
  ::serialise(SerialisedType_.TheTag, Output);
}

// [1] operator+ promotes its operand to a printable integral type.

#endif // SERIALISER_H
