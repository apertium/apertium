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

#ifndef DESERIALISER_H
#define DESERIALISER_H

#include "a.h"
#include "analysis.h"
#include "basic_exception_type.h"
#include "exception.h"
#include "i.h"
#include "lemma.h"
#include "morpheme.h"
#include "tag.h"

#include <cstddef>
#include <istream>
#include <limits>
#include <map>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace Apertium {
template <typename DeserialisedType> class Deserialiser;

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

template <typename value_type>
class Deserialiser<std::basic_string<value_type> > {
public:
  inline static std::basic_string<value_type>
  deserialise(std::istream &Stream_);
};

template <typename key_type, typename mapped_type>
class Deserialiser<std::map<key_type, mapped_type> > {
public:
  inline static std::map<key_type, mapped_type>
  deserialise(std::istream &Stream_);
};

template <typename first_type, typename second_type>
class Deserialiser<std::pair<first_type, second_type> > {
public:
  inline static std::pair<first_type, second_type>
  deserialise(std::istream &Stream_);
};

template <> class Deserialiser<std::size_t> {
public:
  inline static std::size_t deserialise(std::istream &Stream_);
};

template <typename value_type> class Deserialiser<std::vector<value_type> > {
public:
  inline static std::vector<value_type> deserialise(std::istream &Stream_);
};

template <> class Deserialiser<wchar_t> {
public:
  inline static wchar_t deserialise(std::istream &Stream_);
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

template <typename value_type>
std::basic_string<value_type>
Deserialiser<std::basic_string<value_type> >::deserialise(
    std::istream &Stream_) {
  std::size_t SerialisedValueCount =
      Deserialiser<std::size_t>::deserialise(Stream_);
  std::basic_string<value_type> SerialisedType_;

  for (; SerialisedValueCount != 0; --SerialisedValueCount) {
    SerialisedType_.push_back(Deserialiser<value_type>::deserialise(Stream_));
  }

  return SerialisedType_;
}

template <typename key_type, typename mapped_type>
std::map<key_type, mapped_type>
Deserialiser<std::map<key_type, mapped_type> >::deserialise(
    std::istream &Stream_) {
  std::size_t SerialisedValueCount =
      Deserialiser<std::size_t>::deserialise(Stream_);
  std::map<key_type, mapped_type> SerialisedType_;

  for (; SerialisedValueCount != 0; --SerialisedValueCount) {
    SerialisedType_.insert(
        Deserialiser<std::pair<key_type, mapped_type> >::deserialise(Stream_));
  }

  return SerialisedType_;
}

template <typename first_type, typename second_type>
std::pair<first_type, second_type>
Deserialiser<std::pair<first_type, second_type> >::deserialise(
    std::istream &Stream_) {
  std::pair<first_type, second_type> SerialisedType_;
  SerialisedType_.first = Deserialiser<first_type>::deserialise(Stream_);
  SerialisedType_.second = Deserialiser<second_type>::deserialise(Stream_);
  return SerialisedType_;
}

std::size_t Deserialiser<std::size_t>::deserialise(std::istream &Stream_) {
  try {
    std::size_t SerialisedType_ = 0;
    unsigned char SerialisedTypeSize = Stream_.get();

    if (!Stream_)
      throw Exception::Deserialiser::not_Stream_good("can't deserialise size");

    for (; SerialisedTypeSize != 0;) {
      SerialisedType_ +=
          static_cast<std::size_t>(Stream_.get())
          << std::numeric_limits<unsigned char>::digits * --SerialisedTypeSize;

      if (!Stream_)
        throw Exception::Deserialiser::not_Stream_good(
            "can't deserialise byte");
    }

    return SerialisedType_;
  } catch (const basic_ExceptionType &basic_ExceptionType_) {
    std::stringstream what_;
    what_ << "can't deserialise std::size_t: " << basic_ExceptionType_.what();
    throw Exception::Deserialiser::size_t_(what_);
  }
}

template <typename value_type>
std::vector<value_type>
Deserialiser<std::vector<value_type> >::deserialise(std::istream &Stream_) {
  std::size_t SerialisedValueCount =
      Deserialiser<std::size_t>::deserialise(Stream_);
  std::vector<value_type> SerialisedType_;

  for (; SerialisedValueCount != 0; --SerialisedValueCount) {
    SerialisedType_.push_back(Deserialiser<value_type>::deserialise(Stream_));
  }

  return SerialisedType_;
}

wchar_t Deserialiser<wchar_t>::deserialise(std::istream &Stream_) {
  try {
    unsigned wchar_t SerialisedType_ = 0;
    unsigned char SerialisedTypeSize = Stream_.get();

    if (!Stream_)
      throw Exception::Deserialiser::not_Stream_good("can't deserialise size");

    for (; SerialisedTypeSize != 0;) {
      SerialisedType_ +=
          static_cast<std::size_t>(Stream_.get())
          << std::numeric_limits<unsigned char>::digits * --SerialisedTypeSize;

      if (!Stream_)
        throw Exception::Deserialiser::not_Stream_good(
            "can't deserialise byte");
    }

    return static_cast<wchar_t>(SerialisedType_);
  } catch (const basic_ExceptionType &basic_ExceptionType_) {
    std::stringstream what_;
    what_ << "can't deserialise wchar_t: " << basic_ExceptionType_.what();
    throw Exception::Deserialiser::wchar_t_(what_);
  }
}
}

#endif // DESERIALISER_H
