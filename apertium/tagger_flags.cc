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

#include "tagger_flags.h"

namespace Apertium {
TaggerFlags::TaggerFlags()
    : Debug(false), First(false), Mark(false), ShowSuperficial(false),
      NullFlush(false) {}

bool TaggerFlags::getDebug() { return Debug; }

void TaggerFlags::setDebug(const bool &Debug_) { Debug = Debug_; }

bool TaggerFlags::getSentSeg() { return SentSeg; }

void TaggerFlags::setSentSeg(const bool &SentSeg_) { SentSeg = SentSeg_; }

bool TaggerFlags::getSkipErrors() { return SkipErrors; }

void TaggerFlags::setSkipErrors(const bool &SkipErrors_) { SkipErrors = SkipErrors_; }

bool TaggerFlags::getFirst() { return First; }

void TaggerFlags::setFirst(const bool &First_) { First = First_; }

bool TaggerFlags::getMark() { return Mark; }

void TaggerFlags::setMark(const bool &Mark_) { Mark = Mark_; }

bool TaggerFlags::getShowSuperficial() { return ShowSuperficial; }

void TaggerFlags::setShowSuperficial(const bool &ShowSuperficial_) {
  ShowSuperficial = ShowSuperficial_;
}

bool TaggerFlags::getNullFlush() { return NullFlush; }

void TaggerFlags::setNullFlush(const bool &NullFlush_) {
  NullFlush = NullFlush_;
}
}
