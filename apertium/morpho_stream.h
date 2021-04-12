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
/**
 *  Word class and MorphoStream class definitions
 *
 *  @author	Felipe Sánchez-Martínez
 */

#ifndef __MORPHOSTREAM_H
#define __MORPHOSTREAM_H

#include <apertium/constant_manager.h>
#include <lttoolbox/match_exe.h>
#include <lttoolbox/match_state.h>
#include <apertium/tagger_data.h>
#include <apertium/tagger_word.h>

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
class MorphoStream {
public:
   virtual ~MorphoStream();
   /** Get next word in the input stream
    *  @return  A pointer to the next word in the input stream
    */
   virtual TaggerWord* get_next_word() = 0;

   /**
    * Set up the flag to detect '\0' characters
    * @param nf the null_flush value
    */
   virtual void setNullFlush(bool nf) = 0;

   /**
    * Return true if the last reading is end of file of '\0' when null_flush
    * is true
    * @returns the value of end_of_file
    */
   virtual bool getEndOfFile(void) = 0;

   /**
    * Sets a new value for the end_of_file_flag
    * @param eof the new value for end_of_file
    */
   virtual void setEndOfFile(bool eof) = 0;

   /**
    * Rewind the underlying FILE* and unset the end_of_file flag
    */
   virtual void rewind() = 0;
};

#endif
