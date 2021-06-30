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
#ifndef _POSTCHUNK_
#define _POSTCHUNK_

#include <apertium/transfer_base.h>

#include <apertium/interchunk_word.h>
#include <lttoolbox/input_file.h>

using namespace std;

class Postchunk : public TransferBase
{
private:

  InterchunkWord **word;

  bool in_lu;
  bool in_wblank;
  UString out_wblank;
  map <UString, UString> var_out_wblank;

  bool inword;

  UString evalCachedString(xmlNode* element);
  void processClip(xmlNode* element);
  void processBlank(xmlNode* element);
  void processLuCount(xmlNode* element);
  void processCaseOf(xmlNode* element);
  UString processLu(xmlNode* element);
  UString processMlu(xmlNode* element);

  UString processChunk(xmlNode* element);

  void processLet(xmlNode *localroot);
  void processOut(xmlNode *localroot);
  void processCallMacro(xmlNode *localroot);
  void processModifyCase(xmlNode *localroot);
  void processTags(xmlNode *localroot);
  UString readWord(InputFile& in);
  UString readBlank(InputFile& in);
  UString readUntil(InputFile& in, int const symbol) const;
  void applyWord(UString const &word_str);
  int applyRule();
  TransferToken & readToken(InputFile& in);
  static void unchunk(UString const &chunk, UFILE *output);
  static vector<UString> getVecTags(UString const &chunk);
  static int beginChunk(UString const &chunk);
  static int endChunk(UString const &chunk);
  static void splitWordsAndBlanks(UString const &chunk,
				  vector<UString *> &words,
				  vector<UString *> &blanks);
  static UString pseudolemma(UString const &chunk);
  static UString wordzero(UString const &chunk);
  bool checkIndex(xmlNode *element, int index, int limit);
  void postchunk_wrapper_null_flush(InputFile& in, UFILE* out);

public:
  Postchunk();

  void postchunk(InputFile& in, UFILE* out);
};

#endif
