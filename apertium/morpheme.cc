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
#include <lttoolbox/i18n.h>

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
    throw Exception::Morpheme::TheTags_empty(I18n(APR_I18N_DATA, "apertium").format("APR80560"));

  if (TheLemma.empty())
    throw Exception::Morpheme::TheLemma_empty(I18n(APR_I18N_DATA, "apertium").format("APR80570"));

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
  while (c != '<' && c != '$' && c != '/' && c != '\0') {
    TheLemma += c;
    if (c == '\\') {
      if (in.eof() || in.peek() == '\0') {
        throw Exception::Stream::UnexpectedEndOfFile(I18n(APR_I18N_DATA, "apertium").format("APR80580"));
      }
      TheLemma += in.get();
    }
    c = in.get();
  }
  if (TheLemma.empty()) {
    throw Exception::Morpheme::TheLemma_empty(I18n(APR_I18N_DATA, "apertium").format("APR80590"));
  }
  while (c == '<') {
    UString tg = in.readBlock('<', '>');
    if (tg.size() == 2) {
      throw Exception::Morpheme::TheTags_empty(I18n(APR_I18N_DATA, "apertium").format("APR80600"));
    }
    TheTags.push_back(tg.substr(1, tg.size()-2));
    c = in.get();
  }
  if (TheTags.empty()) {
    throw Exception::Morpheme::TheTags_empty(I18n(APR_I18N_DATA, "apertium").format("APR80610"));
  }
  if (c == '#') {
    while (c != '<' && c != '$' && c != '/' && c != '\0' && c != '+') {
      TheLemma += c;
      if (c == '\\') {
        if (in.eof() || in.peek() == '\0') {
          throw Exception::Stream::UnexpectedEndOfFile(I18n(APR_I18N_DATA, "apertium").format("APR80620"));
        }
        TheLemma += in.get();
      }
      c = in.get();
    }
    if (c == '<') {
      throw Exception::Stream::UnexpectedCharacter(I18n(APR_I18N_DATA, "apertium").format("APR80630"));
    }
  }
  in.unget(c);
}
}
