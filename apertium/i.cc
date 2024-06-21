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

#include "i.h"

#include "exception.h"
#include <lttoolbox/i18n.h>
#include <unicode/ustring.h>

namespace Apertium {
bool operator==(const i &a_, const i &b_) { return a_.TheTags == b_.TheTags; }

bool operator<(const i &a_, const i &b_) { return a_.TheTags < b_.TheTags; }

i::i() {}

i::i(const Analysis &Analysis_) : TheTags() {
  if (Analysis_.TheMorphemes.empty())
    throw Exception::Analysis::TheMorphemes_empty(I18n(APR_I18N_DATA, "apertium").format("APR80130", {"letter"}, {"i"}));

  if (Analysis_.TheMorphemes.front().TheTags.empty())
    throw Exception::Morpheme::TheTags_empty(I18n(APR_I18N_DATA, "apertium").format("APR80460", {"letter"}, {"i"}));

  TheTags = Analysis_.TheMorphemes.front().TheTags;
}

i::i(const Morpheme &Morpheme_) : TheTags() {
  if (Morpheme_.TheTags.empty())
    throw Exception::Morpheme::TheTags_empty(I18n(APR_I18N_DATA, "apertium").format("APR80450", {"letter"}, {"i"}));

  TheTags = Morpheme_.TheTags;
}
}
