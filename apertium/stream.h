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

#ifndef STREAM_H
#define STREAM_H

#include "basic_tagger.h"
#include "optional.h"
#include "streamed_type.h"

#include <cstddef>
#include <istream>
#include <sstream>
#include <string>

namespace Apertium {
class Stream {
public:
  Stream(const basic_Tagger::Flags &Flags_);
  Stream(const basic_Tagger::Flags &Flags_, std::wifstream &CharacterStream_,
         const char *const Filename_);
  Stream(const basic_Tagger::Flags &Flags_, std::wifstream &CharacterStream_,
         const std::string &Filename_);
  Stream(const basic_Tagger::Flags &Flags_, std::wifstream &CharacterStream_,
         const std::stringstream &Filename_);
  StreamedType get();
  StreamedType peek();
  bool peekIsBlank();
  bool flush_() const;

  static void outputLexicalUnit(
    const LexicalUnit &lexical_unit, const Optional<Analysis> analysis,
    std::wostream &output, const basic_Tagger::Flags &flags);

  std::size_t TheLineNumber;
private:
  class PreviousCaseType {
  public:
    PreviousCaseType(const wchar_t &PreviousCase_);
    wchar_t ThePreviousCase;
    bool isPreviousCharacter : 1;
  };
  bool is_eof_throw_if_not_TheCharacterStream_good() const;
  std::wstring Message_what(const std::wstringstream &Message) const;
  bool is_eof_throw_if_not_TheCharacterStream_good(StreamedType &StreamedType_,
                                                   std::wstring &Lemma,
                                                   const wchar_t &Character_);
  bool isTheCharacterStream_eof(StreamedType &StreamedType_,
                                std::wstring &Lemma, const wchar_t &Character_);
  void push_back_Character(StreamedType &StreamedType_, std::wstring &Lemma,
                           const wchar_t &Character_);
  void case_0x5c(StreamedType &StreamedType_, std::wstring &Lemma,
                 const wchar_t &Character_);
  std::wistream &TheCharacterStream;
  Optional<std::string> TheFilename;
  std::wstring TheLine;
  const basic_Tagger::Flags &TheFlags;
  bool private_flush_ : 1;
  Optional<PreviousCaseType> ThePreviousCase;
};
}

#endif // STREAM_H
