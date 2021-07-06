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
// along with this program; if not, see <https://www.gnu.org/licenses/>.

#ifndef STREAM_H
#define STREAM_H

#include "tagger_flags.h"
#include "optional.h"
#include "streamed_type.h"
#include "analysis.h"

#include <lttoolbox/input_file.h>

#include <cstddef>
#include <ostream>

namespace Apertium {
class Stream {
public:
  Stream(TaggerFlags& Flags_);
  Stream(TaggerFlags &Flags_, const char *const Filename_);
  StreamedType get();
  bool peekIsBlank();
  bool flush_() const;

  static void outputLexicalUnit(
    const LexicalUnit &lexical_unit, const Optional<Analysis> analysis,
    std::ostream &output, TaggerFlags &flags);
private:
  bool private_flush_ = false;
  InputFile TheCharacterStream;
  TaggerFlags &TheFlags;
};
}

#endif // STREAM_H
