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

#ifndef OPTIONAL_H
#define OPTIONAL_H

#include "exception.h"

#include <algorithm>
#include <cstddef>
#include <exception>
#include <new>

namespace Apertium {
template <typename OptionalType> class Optional;

template <typename OptionalType>
void swap(Optional<OptionalType> &A, Optional<OptionalType> &B);

template <typename OptionalType> class Optional {
public:
  friend void swap<OptionalType>(Optional &A, Optional &B);
  Optional();
  Optional(const OptionalType &OptionalType_);
  Optional(const Optional &Optional_);
  Optional &operator=(Optional Optional_);
  ~Optional();
  const OptionalType &operator*() const;
  OptionalType &operator*();
  const OptionalType *operator->() const;
  OptionalType *operator->();
  operator bool() const;

private:
  OptionalType *TheOptionalTypePointer;
};

template <typename OptionalType>
void swap(Optional<OptionalType> &A, Optional<OptionalType> &B) {
  using std::swap;
  swap(A.TheOptionalTypePointer, B.TheOptionalTypePointer);
}

template <typename OptionalType>
Optional<OptionalType>::Optional()
    : TheOptionalTypePointer(NULL) {}

template <typename OptionalType>
Optional<OptionalType>::Optional(const OptionalType &OptionalType_)
    : TheOptionalTypePointer(new OptionalType(OptionalType_)) {}

template <typename OptionalType>
Optional<OptionalType>::Optional(const Optional &Optional_) {
  if (Optional_.TheOptionalTypePointer == NULL) {
    TheOptionalTypePointer = NULL;
    return;
  }

  TheOptionalTypePointer =
      new OptionalType(*(Optional_.TheOptionalTypePointer));
}

template <typename OptionalType>
Optional<OptionalType> &Optional<OptionalType>::operator=(Optional Optional_) {
  swap(*this, Optional_);
  return *this;
}

template <typename OptionalType> Optional<OptionalType>::~Optional() {
  if (TheOptionalTypePointer == NULL)
    return;

  delete TheOptionalTypePointer;
}

template <typename OptionalType>
const OptionalType &Optional<OptionalType>::operator*() const {
  if (TheOptionalTypePointer == NULL)
    throw Exception::Optional::TheOptionalTypePointer_null(
        "can't dereference Optional comprising null OptionalType pointer");

  return *TheOptionalTypePointer;
}

template <typename OptionalType>
OptionalType &Optional<OptionalType>::operator*() {
  return const_cast<OptionalType &>(
      static_cast<const Optional &>(*this).operator*());
}

template <typename OptionalType>
const OptionalType *Optional<OptionalType>::operator->() const {
  if (TheOptionalTypePointer == NULL)
    throw Exception::Optional::TheOptionalTypePointer_null(
        "can't dereference Optional comprising null OptionalType pointer");

  return TheOptionalTypePointer;
}

template <typename OptionalType>
OptionalType *Optional<OptionalType>::operator->() {
  return const_cast<OptionalType *>(
      static_cast<const Optional &>(*this).operator->());
}

template <typename OptionalType> Optional<OptionalType>::operator bool() const {
  return TheOptionalTypePointer != NULL;
}
}

#endif
