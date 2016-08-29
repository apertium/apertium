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

#include "tag.h"

#include "exception.h"

#include <iostream>

namespace Apertium {
bool operator==(const Tag &a, const Tag &b) { return a.TheTag == b.TheTag; }

bool operator<(const Tag &a, const Tag &b) { return a.TheTag < b.TheTag; }

Tag::operator std::wstring() const {
  if (TheTag.empty())
    throw Exception::Tag::TheTags_empty("can't convert Tag comprising empty "
                                        "TheTag std::wstring to std::wstring");

  return L"<" + TheTag + L">";
}
}
