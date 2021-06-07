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

#include <apertium/transfer_token.h>
#include <apertium/interchunk_word.h>
#include <lttoolbox/buffer.h>
#include <lttoolbox/input_file.h>

#include <cstdio>
#include <vector>
#include <queue>

using namespace std;

class Postchunk : TransferBase
{
private:

  InterchunkWord **word;
  queue <UString> blank_queue;
  int lword;
  Buffer<TransferToken> input_buffer;
  vector<UString *> tmpword;
  vector<UString *> tmpblank;
  
  bool in_out;
  bool in_lu;
  bool in_wblank;
  UString out_wblank;
  map <UString, UString> var_out_wblank;

  UFILE *output;

  xmlNode *lastrule;
  unsigned int nwords;

  bool inword;

  static UString caseOf(UString const &str);
  UString copycase(UString const &source_word, UString const &target_word);

  void processLet(xmlNode *localroot);
  void processOut(xmlNode *localroot);
  void processCallMacro(xmlNode *localroot);
  void processModifyCase(xmlNode *localroot);
  UString evalString(xmlNode *localroot);
  int processRule(xmlNode* localroot);
  void processInstruction(xmlNode *localroot);
  void processChoose(xmlNode *localroot);
  void processTags(xmlNode *localroot);
  bool beginsWith(UString const &str1, UString const &str2) const;
  bool endsWith(UString const &str1, UString const &str2) const;
  UString tolower(UString const &str) const;
  UString tags(UString const &str) const;
  UString readWord(InputFile& in);
  UString readBlank(InputFile& in);
  UString readUntil(InputFile& in, int const symbol) const;
  void applyWord(UString const &word_str);
  void applyRule();
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
  bool gettingLemmaFromWord(UString attr);
  UString combineWblanks(UString wblank_current, UString wblank_to_add);

public:
  Postchunk();
  ~Postchunk();

  void read(string const &transferfile, string const &datafile);
  void postchunk(InputFile& in, UFILE* out);
  bool getNullFlush(void);
  void setNullFlush(bool null_flush);
  void setTrace(bool trace);
};

#endif
