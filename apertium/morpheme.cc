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

#include "morpheme.h"

#include "exception.h"

namespace Apertium {
bool operator==(const Morpheme &a, const Morpheme &b) {
  return a.TheLemma == b.TheLemma && a.TheTags == b.TheTags;
}

bool operator<(const Morpheme &a, const Morpheme &b) {
  if (a.TheLemma != b.TheLemma)
    return a.TheLemma < b.TheLemma;

  return a.TheTags < b.TheTags;
}

std::ostream& operator<<(std::ostream& out, const Morpheme &morph) {
  ::operator<<(out, morph.TheLemma);
  for (auto& it : morph.TheTags) {
    out << '<';
    ::operator<<(out, it);
    out << '>';
  }
  // namespace issue
  return out;
}

Morpheme::operator UString() const {
  if (TheTags.empty())
    throw Exception::Morpheme::TheTags_empty("can't convert Morpheme "
                                             "comprising empty Tag std::vector "
                                             "to UString");

  if (TheLemma.empty())
    throw Exception::Morpheme::TheLemma_empty("can't convert Morpheme "
                                              "comprising empty TheLemma "
                                              "UString to UString");

  UString ustring_ = TheLemma;

  for (auto& Tag_ : TheTags) {
    ustring_ += '<';
    ustring_ += Tag_;
    ustring_ += '>';
  }

  return ustring_;
}

void
Morpheme::read(InputFile& in)
{
  UChar32 c = in.get();
  while (c != '<' && c != '$' && c != '/' && c != '\0' && c != '+') {
    TheLemma += c;
    if (c == '\\') {
      if (in.eof() || in.peek() == '\0') {
        throw Exception::Stream::UnexpectedEndOfFile("Unterminted lexical unit");
      }
      TheLemma += in.get();
    }
    c = in.get();
  }
  if (TheLemma.empty()) {
    throw Exception::Morpheme::TheLemma_empty("empty lemma");
  }
  while (c == '<') {
    UString tg = in.readBlock('<', '>');
    if (tg.size() == 2) {
      throw Exception::Morpheme::TheTags_empty("invalid tag <>");
    }
    TheTags.push_back(tg.substr(1, tg.size()-2));
    c = in.get();
  }
  if (TheTags.empty()) {
    throw Exception::Morpheme::TheTags_empty("morpheme has no tags");
  }
  if (c == '#') {
    while (c != '<' && c != '$' && c != '/' && c != '\0' && c != '+') {
      TheLemma += c;
      if (c == '\\') {
        if (in.eof() || in.peek() == '\0') {
          throw Exception::Stream::UnexpectedEndOfFile("trailing backslash");
        }
        TheLemma += in.get();
      }
      c = in.get();
    }
    if (c == '<') {
      throw Exception::Stream::UnexpectedCharacter("unexpected < after lemma queue");
    }
  }
  in.unget(c);
}
}
