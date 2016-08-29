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

#ifndef APERTIUM_DESERIALISER_H
#define APERTIUM_DESERIALISER_H

#include "a.h"
#include "analysis.h"
#include "basic_exception_type.h"
#include "exception.h"
#include "i.h"
#include "lemma.h"
#include "morpheme.h"
#include "tag.h"

#include <lttoolbox/deserialiser.h>
#include <stdint.h>
#include <apertium/perceptron_spec.h>
#include <cstddef>
#include <limits>
#include <istream>
#include <limits>
#include <map>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include <typeinfo>

using namespace Apertium;

template <> class Deserialiser<a> {
public:
  inline static a deserialise(std::istream &Stream_);
};

template <> class Deserialiser<Analysis> {
public:
  inline static Analysis deserialise(std::istream &Stream_);
};

template <> class Deserialiser<i> {
public:
  inline static i deserialise(std::istream &Stream_);
};

template <> class Deserialiser<Lemma> {
public:
  inline static Lemma deserialise(std::istream &Stream_);
};

template <> class Deserialiser<Morpheme> {
public:
  inline static Morpheme deserialise(std::istream &Stream_);
};

template <> class Deserialiser<Tag> {
public:
  inline static Tag deserialise(std::istream &Stream_);
};

a Deserialiser<a>::deserialise(std::istream &Stream_) {
  a StreamedType_;
  StreamedType_.TheTags = Deserialiser<std::vector<Tag> >::deserialise(Stream_);
  StreamedType_.TheMorphemes =
      Deserialiser<std::vector<Morpheme> >::deserialise(Stream_);
  return StreamedType_;
}

Analysis Deserialiser<Analysis>::deserialise(std::istream &Stream_) {
  Analysis SerialisedType_;
  SerialisedType_.TheMorphemes =
      Deserialiser<std::vector<Morpheme> >::deserialise(Stream_);
  return SerialisedType_;
}

i Deserialiser<i>::deserialise(std::istream &Stream_) {
  i StreamedType_;
  StreamedType_.TheTags = Deserialiser<std::vector<Tag> >::deserialise(Stream_);
  return StreamedType_;
}

Lemma Deserialiser<Lemma>::deserialise(std::istream &Stream_) {
  Lemma StreamedType_;
  StreamedType_.TheLemma = Deserialiser<std::wstring>::deserialise(Stream_);
  return StreamedType_;
}

Morpheme Deserialiser<Morpheme>::deserialise(std::istream &Stream_) {
  Morpheme SerialisedType_;
  SerialisedType_.TheLemma = Deserialiser<std::wstring>::deserialise(Stream_);
  SerialisedType_.TheTags =
      Deserialiser<std::vector<Tag> >::deserialise(Stream_);
  return SerialisedType_;
}

Tag Deserialiser<Tag>::deserialise(std::istream &Stream_) {
  Tag SerialisedType_;
  SerialisedType_.TheTag = Deserialiser<std::wstring>::deserialise(Stream_);
  return SerialisedType_;
}

#endif // DESERIALISER_H
