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
#ifndef _TRANSFER_
#define _TRANSFER_

#include <apertium/transfer_base.h>

#include <apertium/transfer_token.h>
#include <apertium/transfer_word.h>
#include <lttoolbox/buffer.h>
#include <lttoolbox/fst_processor.h>

#include <cstdio>
#include <vector>
#include <queue>

using namespace std;

class Transfer : TransferBase
{
private:

  TransferWord **word;
  queue <UString> blank_queue;
  int lword;
  int last_lword;
  Buffer<TransferToken> input_buffer;
  vector<UString *> tmpword;
  vector<UString *> tmpblank;
  
  bool in_out;
  bool in_lu;
  
  bool in_wblank;
  UString out_wblank;
  map <UString, UString> var_out_wblank;
  
  FSTProcessor fstp;
  FSTProcessor extended;
  bool isExtended;
  UFILE *output;

  xmlNode *lastrule;
  unsigned int nwords;

  enum OutputType{lu,chunk};

  OutputType defaultAttrs;
  bool preBilingual;
  bool useBilingual;
  bool trace_att;
  UString emptyblank;

  void readBil(string const &filename);

  void processLet(xmlNode *localroot);
  void processAppend(xmlNode *localroot);
  void processOut(xmlNode *localroot);
  void processCallMacro(xmlNode *localroot);
  void processModifyCase(xmlNode *localroot);
  bool processLogical(xmlNode *localroot);
  bool processTest(xmlNode *localroot);
  bool processAnd(xmlNode *localroot);
  bool processOr(xmlNode *localroot);
  bool processEqual(xmlNode *localroot);
  bool processBeginsWith(xmlNode *localroot);
  bool processBeginsWithList(xmlNode *localroot);
  bool processEndsWith(xmlNode *localroot);
  bool processEndsWithList(xmlNode *local);
  bool processContainsSubstring(xmlNode *localroot);
  bool processNot(xmlNode *localroot);
  bool processIn(xmlNode *localroot);
  int processRule(xmlNode *localroot);
  UString evalString(xmlNode *localroot);
  int processInstruction(xmlNode *localroot);
  int processChoose(xmlNode *localroot);
  UString processChunk(xmlNode *localroot);
  UString processTags(xmlNode *localroot);

  UString readWord(InputFile& in);
  UString readBlank(InputFile& in);
  UString readUntil(InputFile& in, int const symbol) const;
  void applyWord(UString const &word_str);
  int applyRule();
  TransferToken & readToken(InputFile& in);
  bool checkIndex(xmlNode *element, int index, int limit);
  void transfer_wrapper_null_flush(InputFile& in, UFILE* out);
  void tmp_clear();
public:
  Transfer();
  ~Transfer();

  void read(string const &transferfile, string const &datafile,
	    string const &fstfile = "");
  void transfer(InputFile& in, UFILE* out);
  void setUseBilingual(bool value);
  bool getUseBilingual(void) const;
  void setPreBilingual(bool value);
  bool getPreBilingual(void) const;
  void setExtendedDictionary(string const &fstfile);
  void setCaseSensitiveness(bool value);
  void setTraceATT(bool trace);
};

#endif
