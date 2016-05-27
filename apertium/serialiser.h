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

#ifndef SERIALISER_H
#define SERIALISER_H

#include "a.h"
#include "basic_exception_type.h"
#include "analysis.h"
#include "exception.h"
#include "i.h"
#include "lemma.h"
#include "morpheme.h"
#include "tag.h"

#include <cstddef>
#include <ios>
#include <limits>
#include <map>
#include <ostream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace Apertium {
namespace {
template <typename SerialisedType>
static unsigned char compressedSize(const SerialisedType &SerialisedType_) {
  unsigned char compressedSize_ = 0;

  for (; static_cast<unsigned char>(SerialisedType_ >>
                                    std::numeric_limits<unsigned char>::digits *
                                        compressedSize_) != 0;
       ++compressedSize_) {
  }

  return compressedSize_;
}

template <typename SerialisedType> class Serialiser;

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

template <typename value_type>
class Serialiser<std::basic_string<value_type> > {
public:
  inline static void
  serialise(const std::basic_string<value_type> &SerialisedType_,
            std::ostream &Output);
};

template <typename key_type, typename mapped_type>
class Serialiser<std::map<key_type, mapped_type> > {
public:
  inline static void
  serialise(const std::map<key_type, mapped_type> &SerialisedType_,
            std::ostream &Output);
};

template <typename first_type, typename second_type>
class Serialiser<std::pair<first_type, second_type> > {
public:
  inline static void
  serialise(const std::pair<first_type, second_type> &SerialisedType_,
            std::ostream &Output);
};

template <> class Serialiser<std::size_t> {
public:
  inline static void serialise(const std::size_t &SerialisedType_,
                               std::ostream &Output);
};

template <typename value_type> class Serialiser<std::vector<value_type> > {
public:
  inline static void serialise(const std::vector<value_type> &SerialisedType_,
                               std::ostream &Output);
};

template <> class Serialiser<wchar_t> {
public:
  inline static void serialise(const wchar_t &SerialisedType_,
                               std::ostream &Output);
};
}

template <typename SerialisedType>
inline void serialise(const SerialisedType &SerialisedType_,
                      std::ostream &Output) {
  Serialiser<SerialisedType>::serialise(SerialisedType_, Output);
}

void Serialiser<a>::serialise(const a &SerialisedType_, std::ostream &Output) {
  Serialiser<std::vector<Tag> >::serialise(SerialisedType_.TheTags, Output);
  Serialiser<std::vector<Morpheme> >::serialise(SerialisedType_.TheMorphemes,
                                                Output);
}

void Serialiser<Analysis>::serialise(const Analysis &SerialisedType_,
                                     std::ostream &Output) {
  Serialiser<std::vector<Morpheme> >::serialise(SerialisedType_.TheMorphemes,
                                                Output);
}

void Serialiser<i>::serialise(const i &SerialisedType_, std::ostream &Output) {
  Serialiser<std::vector<Tag> >::serialise(SerialisedType_.TheTags, Output);
}

void Serialiser<Lemma>::serialise(const Lemma &SerialisedType_,
                                  std::ostream &Output) {
  serialise(SerialisedType_.TheLemma, Output);
}

void Serialiser<Morpheme>::serialise(const Morpheme &SerialisedType_,
                                     std::ostream &Output) {
  serialise(SerialisedType_.TheLemma, Output);
  Serialiser<std::vector<Tag> >::serialise(SerialisedType_.TheTags, Output);
}

void Serialiser<Tag>::serialise(const Tag &SerialisedType_,
                                std::ostream &Output) {
  serialise(SerialisedType_.TheTag, Output);
}

template <typename value_type>
void Serialiser<std::basic_string<value_type> >::serialise(
    const std::basic_string<value_type> &SerialisedType_,
    std::ostream &Output) {
  serialise(SerialisedType_.size(), Output);

  for (typename std::basic_string<value_type>::const_iterator
           SerialisedType_iterator = SerialisedType_.begin();
       // Call .end() each iteration to save memory.
       SerialisedType_iterator != SerialisedType_.end();
       ++SerialisedType_iterator) {
    serialise(*SerialisedType_iterator, Output);
  }
}

template <typename key_type, typename mapped_type>
void Serialiser<std::map<key_type, mapped_type> >::serialise(
    const std::map<key_type, mapped_type> &SerialisedType_,
    std::ostream &Output) {
  serialise(SerialisedType_.size(), Output);

  for (typename std::map<key_type, mapped_type>::const_iterator
           SerialisedType_iterator = SerialisedType_.begin();
       // Call .end() each iteration to save memory.
       SerialisedType_iterator != SerialisedType_.end();
       ++SerialisedType_iterator) {
    Serialiser<std::pair<key_type, mapped_type> >::serialise(
        *SerialisedType_iterator, Output);
  }
}

template <typename first_type, typename second_type>
void Serialiser<std::pair<first_type, second_type> >::serialise(
    const std::pair<first_type, second_type> &SerialisedType_,
    std::ostream &Output) {
  serialise(SerialisedType_.first, Output);
  serialise(SerialisedType_.second, Output);
}

void Serialiser<std::size_t>::serialise(const std::size_t &SerialisedType_,
                                        std::ostream &Output) {
  try {
    Output.put(compressedSize(SerialisedType_));

    if (!Output) {
      std::stringstream what_;
      what_ << "can't serialise size " << std::hex
            << /* [1] */ +compressedSize(SerialisedType_) << std::dec;
      throw Exception::Serialiser::not_Stream_good(what_);
    }

    for (unsigned char CompressedSize = compressedSize(SerialisedType_);
         CompressedSize != 0; Output.put(static_cast<unsigned char>(
             SerialisedType_ >>
             std::numeric_limits<unsigned char>::digits * --CompressedSize))) {
      if (!Output) {
        std::stringstream what_;
        what_ << "can't serialise byte " << std::hex
              << /* [1] */ +static_cast<unsigned char>(
                     SerialisedType_ >>
                     std::numeric_limits<unsigned char>::digits *
                         CompressedSize) << std::dec;
        throw Exception::Serialiser::not_Stream_good(what_);
      }
    }
  } catch (const basic_ExceptionType &basic_ExceptionType_) {
    std::stringstream what_;
    what_ << "can't serialise const std::size_t & : "
          << basic_ExceptionType_.what();
    throw Exception::Serialiser::size_t_(what_);
  }
}

template <typename value_type>
void Serialiser<std::vector<value_type> >::serialise(
    const std::vector<value_type> &SerialisedType_, std::ostream &Output) {
  serialise(SerialisedType_.size(), Output);

  for (typename std::vector<value_type>::const_iterator value_type_ =
           SerialisedType_.begin();
       // Call .end() each iteration to save memory.
       value_type_ != SerialisedType_.end(); ++value_type_) {
    serialise(*value_type_, Output);
  }
}

void Serialiser<wchar_t>::serialise(const wchar_t &SerialisedType_,
                                    std::ostream &Output) {
  try {
    Output.put(compressedSize(SerialisedType_));

    if (!Output) {
      std::stringstream what_;
      what_ << "can't serialise size " << std::hex
            << /* [1] */ +compressedSize(SerialisedType_);
      throw Exception::Serialiser::not_Stream_good(what_);
    }

    for (unsigned char CompressedSize = compressedSize(SerialisedType_);
         CompressedSize != 0; Output.put(static_cast<unsigned char>(
             static_cast<unsigned wchar_t>(SerialisedType_) >>
             std::numeric_limits<unsigned char>::digits * --CompressedSize))) {
      if (!Output) {
        std::stringstream what_;
        what_ << "can't serialise byte " << std::hex
              << /* [1] */ +(static_cast<unsigned wchar_t>(SerialisedType_) >>
                             std::numeric_limits<unsigned char>::digits *
                                 CompressedSize);
        throw Exception::Serialiser::not_Stream_good(what_);
      }
    }
  } catch (const basic_ExceptionType &basic_ExceptionType_) {
    std::stringstream what_;
    what_ << "can't serialise const wchar_t & : "
          << basic_ExceptionType_.what();
    throw Exception::Serialiser::wchar_t_(what_);
  }
}
}

// [1] operator+ promotes its operand to a printable integral type.

#endif // SERIALISER_H
