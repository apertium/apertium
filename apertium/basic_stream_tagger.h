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

#ifndef BASIC_STREAM_TAGGER_H
#define BASIC_STREAM_TAGGER_H

#include "apertium_config.h"

#include "basic_tagger.h"
#include "lexical_unit.h"
#include "stream.h"

#include <istream>
#include <ostream>

#if ENABLE_DEBUG

#include <string>

#endif // ENABLE_DEBUG

namespace Apertium {
class basic_StreamTagger : protected basic_Tagger {
public:
  virtual ~basic_StreamTagger();
  virtual void deserialise(std::istream &Serialised_basic_Tagger) = 0;
  void tag(Stream &Input, std::wostream &Output) const;

protected:
  basic_StreamTagger(const Flags &Flags_);
  virtual long double score(const Analysis &Analysis_) const = 0;

#if ENABLE_DEBUG

  virtual std::wstring score_DEBUG(const Analysis &Analysis_) const = 0;

#endif // ENABLE_DEBUG

private:
  void tag(const LexicalUnit &LexicalUnit_, std::wostream &Output) const;
};
}

#endif // BASIC_STREAM_TAGGER_H
