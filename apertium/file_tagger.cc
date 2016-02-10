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

#include "file_tagger.h"

#include <apertium/tsx_reader.h>

#include <cstdio>

namespace Apertium {
FILE_Tagger::FILE_Tagger() : debug(false), show_sf(false), null_flush(false) {}

FILE_Tagger::~FILE_Tagger() {}

void FILE_Tagger::set_debug(const bool &Debug) { debug = Debug; }

void FILE_Tagger::set_show_sf(const bool &ShowSuperficial) {
  show_sf = ShowSuperficial;
}

void FILE_Tagger::setNullFlush(const bool &NullFlush) {
  null_flush = NullFlush;
}

void FILE_Tagger::deserialise(char *const TaggerSpecificationFilename) {
  TSXReader TaggerSpecificationReader_;
  TaggerSpecificationReader_.read(TaggerSpecificationFilename);
  deserialise(TaggerSpecificationReader_.getTaggerData());
}
}
