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
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */
/** 
 *  Word class and MorphoStream class definitions
 *
 *  @author	Felipe Sánchez-Martínez 
 */

#ifndef __FILEMORPHOSTREAM_H
#define __FILEMORPHOSTREAM_H

#include <apertium/constant_manager.h>
#include <lttoolbox/match_exe.h>
#include <lttoolbox/match_state.h>
#include <apertium/tagger_data.h>
#include <apertium/tagger_word.h>
#include <apertium/morpho_stream.h>

#include <cstdio>
#include <deque>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>

using namespace std;

/** Class MorphoStream.  
 *  This class processes the output of class  yyFlexLexer (lex.yy.cc), and 
 *  builds the TaggerWord objects managed by the tagger 
 */
class FileMorphoStream : public MorphoStream {
private:
  bool foundEOF;
  wstring last_string_tag;
  bool debug;
  FILE *input;
  int ca_any_char;
  int ca_any_tag;
  int ca_kignorar;
  int ca_kbarra;
  int ca_kdollar;
  int ca_kbegin;
  int ca_kmot;
  int ca_kmas;
  int ca_kunknown;
  int ca_tag_keof;
  int ca_tag_kundef;

  vector<TaggerWord *> vwords; //Vector used to implement a buffer
                             //to treat ambiguous multiword units

  MatchExe *me;
  TaggerData *td;
  Alphabet alphabet;
  MatchState ms;

  bool null_flush;
  bool end_of_file;

  void readRestOfWord(int &ivwords);
  void lrlmClassify(wstring const &str, int &ivwords);
public:

   /** Constructor 
    *  @param is the input stream.
    */
   FileMorphoStream(FILE *ftxt, bool d, TaggerData *t);
  
   /** 
    *  Destructor 
    */
   ~FileMorphoStream();
  
   /** See interface */
   TaggerWord* get_next_word();  
   void setNullFlush(bool nf);
   bool getEndOfFile(void);
   void setEndOfFile(bool eof);
   void rewind();
};

#endif
