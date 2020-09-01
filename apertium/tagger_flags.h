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

#ifndef _TAGGER_FLAGS_H_
#define _TAGGER_FLAGS_H_

namespace Apertium {
class TaggerFlags {
public:
  TaggerFlags();
  bool getDebug();
  void setDebug(const bool &Debug_);
  bool getSentSeg();
  void setSentSeg(const bool &SentSeg_);
  bool getSkipErrors();
  void setSkipErrors(const bool &SkipErrors_);
  bool getFirst();
  void setFirst(const bool &First_);
  bool getMark();
  void setMark(const bool &Mark_);
  bool getShowSuperficial();
  void setShowSuperficial(const bool &ShowSuperficial_);
  bool getNullFlush();
  void setNullFlush(const bool &NullFlush_);
  static bool (TaggerFlags::*GetDebug)() const;
  static void (TaggerFlags::*SetDebug)(const bool &);
  static bool (TaggerFlags::*GetFirst)() const;
  static void (TaggerFlags::*SetFirst)(const bool &);
  static bool (TaggerFlags::*GetMark)() const;
  static void (TaggerFlags::*SetMark)(const bool &);
  static bool (TaggerFlags::*GetShowSuperficial)() const;
  static void (TaggerFlags::*SetShowSuperficial)(const bool &);
  static bool (TaggerFlags::*GetNullFlush)() const;
  static void (TaggerFlags::*SetNullFlush)(const bool &);

private:
  bool Debug : 1;
  bool SentSeg : 1;
  bool SkipErrors : 1;
  bool First : 1;
  bool Mark : 1;
  bool ShowSuperficial : 1;
  bool NullFlush : 1;
};
}

#endif
