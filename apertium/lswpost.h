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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */
/**
 *  Light Sliding-Window Part of Speech Tagger (LSWPoST) implementation (header)
 *
 *  @author   Gang Chen - pkuchengang@gmail.com
 */

#ifndef __LSWPOST_H
#define __LSWPOST_H

#include <cstdio>
#include <fstream>
#include <math.h>
#include <string>
#include <vector>
#include <set>
#include <map>
#include <cfloat>
#include <cstring>

#include <apertium/collection.h>
#include <apertium/constant_manager.h>
#include <apertium/morpho_stream.h>
#include <apertium/tagger_data_lsw.h>
#include <apertium/tagger_utils.h>
#include <apertium/tagger_word.h>


#define ZERO 1e-10

/** LSWPoST
 *  Light Sliding-Window Part of Speech Tagger
 */
class LSWPoST {
private:
  TaggerDataLSW * tdlsw;
  TTag eos; // end-of-sentence tag
  bool debug; //If true, print error messages when tagging input text
  bool show_sf; //If true, print superficial forms when tagging input text
  bool null_flush; //If true, flush on '\0'

public:
   /** Constructor
    */
   LSWPoST(TaggerDataLSW *t);

   /** Destructor
    */
   ~LSWPoST();

   /** Used to set the end-of-sentence tag
    *  @param t the end-of-sentence tag
    */
   void set_eos(TTag t);
   
   /** Used to set the debug flag
    */
   void set_debug(bool d);

   /** Used to set the show superficial forms flag 
    */
   void set_show_sf(bool sf);

   /** Used to set the null_flush flag 
    */
   void setNullFlush(bool nf);

   /** It reads the expanded dictionary received as a parameter and calculates
    *  the set of ambiguity classes that the tagger will manage.
    *  @param fdic the input stream with the expanded dictionary to read
    */
   void read_dictionary(FILE *fdic);

   /** Whether a tag sequence is valid, according to the forbid and enforce rules
    */
   bool is_valid_seq(TTag left, TTag mid, TTag right);

   /** Init probabilities
    *  It applies the forbid and enforce rules found in tagger specification.
    *  To do so, the joint probability of a tag sequence that contains a forbid
    *  rule, or doesn't satisfy a enforce rule, is set to 0.
    */
   void init_probabilities(FILE *ftxt);

   /** Unsupervised training algorithm (Baum-Welch implementation).
    *  @param ftxt the input stream with the untagged corpus to process
    */
   void train (FILE *ftxt);

   /** Prints the para matrix.
    */
   void print_para_matrix();

   /** Do the tagging
    */
   void tagger(FILE *in, FILE *out, bool show_all_good_first);

   /** This method returns a known ambiguity class that is a subset of 
    *  the one received as a parameter. This is useful when a new
    *  ambiguity class is found because of changes in the morphological
    *  dictionary used by the MT system.
    *  @param c set of tags (ambiguity class) 
    *  @return a known ambiguity class    
    */ 
   set<TTag> find_similar_ambiguity_class(set<TTag> c);
};
#endif
