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
 *  Light Sliding-Window Part of Speech Tagger (LSWPoST) implementation (header)
 *
 *  @author   Gang Chen - pkuchengang@gmail.com
 */

#ifndef __LSWPOST_H
#define __LSWPOST_H

#include "file_tagger.h"

#include <cstdio>
#include <fstream>
#include <cmath>
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
class LSWPoST : public Apertium::FILE_Tagger {
private:
  TaggerDataLSW tdlsw;
  TTag eos; // end-of-sentence tag
protected:
  void post_ambg_class_scan();
public:
  TaggerData& get_tagger_data();
  void deserialise(FILE *Serialised_FILE_Tagger);
  std::vector<std::wstring> &getArrayTags();
  void serialise(FILE *Stream_);
  void deserialise(const TaggerData &Deserialised_FILE_Tagger);
  void init_probabilities_from_tagged_text_(MorphoStream &, MorphoStream &);
  void init_probabilities_kupiec_(MorphoStream &lexmorfo);
  void train(MorphoStream &morpho_stream, unsigned long count);
  LSWPoST();
  LSWPoST(TaggerDataLSW *tdlsw);

   /** Constructor
    */
   LSWPoST(TaggerDataLSW t);

   /** Destructor
    */
   ~LSWPoST();

   /** Used to set the end-of-sentence tag
    *  @param t the end-of-sentence tag
    */
   void set_eos(TTag t);

   /** Whether a tag sequence is valid, according to the forbid and enforce rules
    */
   bool is_valid_seq(TTag left, TTag mid, TTag right);

   /** Init probabilities
    *  It applies the forbid and enforce rules found in tagger specification.
    *  To do so, the joint probability of a tag sequence that contains a forbid
    *  rule, or doesn't satisfy a enforce rule, is set to 0.
    */
   void init_probabilities(MorphoStream &morpho_stream);

   /** Unsupervised training algorithm (Baum-Welch implementation).
    *  @param ftxt the input stream with the untagged corpus to process
    */
   void train(MorphoStream &morpho_stream);

   /** Prints the para matrix.
    */
   void print_para_matrix();

   /** Do the tagging
    */
   void tagger(MorphoStream &morpho_stream, FILE *Output,
               const bool &First = false);
};
#endif
