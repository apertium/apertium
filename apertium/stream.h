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

#include <cstddef>
#include <istream>
#include <ostream>
#include <sstream>
#include <string>

namespace Apertium {
class Stream {
public:
  Stream(TaggerFlags &Flags_);
  Stream(TaggerFlags &Flags_, std::ifstream &CharacterStream_,
         const char *const Filename_);
  Stream(TaggerFlags &Flags_, std::ifstream &CharacterStream_,
         const std::string &Filename_);
  Stream(TaggerFlags &Flags_, std::ifstream &CharacterStream_,
         const std::stringstream &Filename_);
  StreamedType get();
  StreamedType peek();
  bool peekIsBlank();
  bool flush_() const;

  static void outputLexicalUnit(
    const LexicalUnit &lexical_unit, const Optional<Analysis> analysis,
    std::ostream &output, TaggerFlags &flags);

  std::size_t TheLineNumber;
private:
  class PreviousCaseType {
  public:
    PreviousCaseType(const wchar_t &PreviousCase_);
    wchar_t ThePreviousCase;
    bool isPreviousCharacter : 1;
  };
  bool is_eof_throw_if_not_TheCharacterStream_good() const;
  UString Message_what(const std::stringstream &Message) const;
  bool is_eof_throw_if_not_TheCharacterStream_good(StreamedType &StreamedType_,
                                                   UString &Lemma,
                                                   const wchar_t &Character_);
  bool isTheCharacterStream_eof(StreamedType &StreamedType_,
                                UString &Lemma, const wchar_t &Character_);
  void push_back_Character(StreamedType &StreamedType_, UString &Lemma,
                           const wchar_t &Character_);
  void case_0x5c(StreamedType &StreamedType_, UString &Lemma,
                 const wchar_t &Character_);
  std::istream &TheCharacterStream;
  Optional<std::string> TheFilename;
  UString TheLine;
  TaggerFlags &TheFlags;
  bool private_flush_ : 1;
  Optional<PreviousCaseType> ThePreviousCase;
};
}

#endif // STREAM_H
