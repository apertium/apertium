/*
 * Copyright (C) 2005 Universitat d'Alacant / Universidad de Alicante
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 */
#ifndef _INTERCHUNK_
#define _INTERCHUNK_

#include <apertium/transfer_base.h>

#include <apertium/interchunk_word.h>
#include <lttoolbox/input_file.h>

using namespace std;

class Interchunk : public TransferBase
{
private:
  InterchunkWord **word;
  int last_lword;

  bool inword;

  void processLet(xmlNode *localroot);
  void processOut(xmlNode *localroot);
  void processCallMacro(xmlNode *localroot);
  void processModifyCase(xmlNode *localroot);
  UString processChunk(xmlNode *localroot);
  void processClip(xmlNode* localroot);
  void processBlank(xmlNode* localroot);
  void processCaseOf(xmlNode* localroot);

  void processLuCount(xmlNode* localroot);
  UString processLu(xmlNode* localroot);
  UString processMlu(xmlNode* localroot);

  UString evalCachedString(xmlNode* element);

  UString readWord(InputFile& in);
  UString readBlank(InputFile& in);
  UString readUntil(InputFile& in, int const symbol) const;
  void applyWord(UString const &word_str);
  void applyRule();
  TransferToken & readToken(InputFile& in);
  bool checkIndex(xmlNode *element, int index, int limit);
  void interchunk_wrapper_null_flush(InputFile& in, UFILE* out);

public:
  Interchunk();

  void interchunk(InputFile& in, UFILE* out);
};

#endif
