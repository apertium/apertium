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

#include "stream.h"

#include "exception.h"

#include <utility>
#include <lttoolbox/i18n.h>

namespace Apertium {
Stream::Stream(TaggerFlags &Flags_)
    : TheFlags(Flags_) {}

Stream::Stream(TaggerFlags &Flags_, const char *const Filename_)
    : TheFlags(Flags_)
{
  TheCharacterStream.open_or_exit(Filename_);
}

StreamedType Stream::get() {
  StreamedType TheStreamedType;
  private_flush_ = false;

  TheStreamedType.TheString = TheCharacterStream.readBlank(true);

  if (!TheCharacterStream.eof() && TheCharacterStream.peek() == '^') {
    TheCharacterStream.get();
    TheStreamedType.TheLexicalUnit = LexicalUnit();
    UChar32 c = TheCharacterStream.get();
    while (c != '/' && c != '$') {
      TheStreamedType.TheLexicalUnit->TheSurfaceForm += c;
      if (c == '\\') {
        c = TheCharacterStream.get();
        TheStreamedType.TheLexicalUnit->TheSurfaceForm += c;
      }
      c = TheCharacterStream.get();
    }
    if (c == '$') {
      throw Exception::Analysis::TheMorphemes_empty(I18n(APR_I18N_DATA, "apertium").format("APR81010"));
    } else if (TheStreamedType.TheLexicalUnit->TheSurfaceForm.empty()) {
      throw Exception::Stream::UnexpectedCharacter(I18n(APR_I18N_DATA, "apertium").format("APR81020"));
    }
    c = TheCharacterStream.get();
    if (c == '$') {
      throw Exception::Analysis::TheMorphemes_empty(I18n(APR_I18N_DATA, "apertium").format("APR81010"));
    } else if (c == '*') {
      TheCharacterStream.readBlock(c, '$');
    } else {
      TheCharacterStream.unget(c);
      do {
        TheStreamedType.TheLexicalUnit->TheAnalyses.push_back(Analysis());
        TheStreamedType.TheLexicalUnit->TheAnalyses.back().read(TheCharacterStream);
        c = TheCharacterStream.get();
      } while (c == '/');
      if (c != '$') {
        throw Exception::Stream::UnexpectedEndOfFile(I18n(APR_I18N_DATA, "apertium").format("APR81030"));
      }
    }
  }

  if (TheCharacterStream.peek() == '\0') {
    TheCharacterStream.get();
    private_flush_ = true;
  }

  return TheStreamedType;
}

bool Stream::peekIsBlank() {
  const UChar32 newline1 = TheCharacterStream.get();
  const UChar32 newline2 = TheCharacterStream.get();

  // somewhat dangerous to unget twice
  // but InputFile does have a 3 char buffer
  TheCharacterStream.unget(newline2);
  TheCharacterStream.unget(newline1);

  return newline1 == '\n' && newline2 == '\n';
}

bool Stream::flush_() const { return private_flush_; }

void Stream::outputLexicalUnit(
    const LexicalUnit &lexical_unit, const Optional<Analysis> analysis,
    std::ostream &output, TaggerFlags &flags) {
  using namespace std::rel_ops;
  output << "^";

  if (lexical_unit.TheAnalyses.empty() || !analysis) {
    if (flags.getShowSuperficial())
      output << lexical_unit.TheSurfaceForm << "/";

    output << "*" << lexical_unit.TheSurfaceForm << "$";
    return;
  }

  if (flags.getMark() && lexical_unit.TheAnalyses.size() != 1) {
    output << "=";
  }

  if (flags.getShowSuperficial())
    output << lexical_unit.TheSurfaceForm << "/";

  output << *analysis;

  if (flags.getFirst()) {
    for (auto& other_analysis : lexical_unit.TheAnalyses) {
      if (other_analysis != *analysis)
        output << "/" << other_analysis;
    }
  }

  output << "$";
}
}
