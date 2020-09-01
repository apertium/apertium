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
 *  First order hidden Markov model (HMM) implementation (header)
 *
 *  @author   Felipe Sánchez-Martínez - fsanchez@dlsi.ua.es
 */

#ifndef __HMM_H
#define __HMM_H

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
#include <apertium/tagger_data_hmm.h>
#include <apertium/tagger_utils.h>
#include <apertium/tagger_word.h>

using namespace std;

#define ZERO 1e-10

/** HMM
 *  first-order hidden Markov Model
 */
class HMM : public Apertium::FILE_Tagger {
private:
   TaggerDataHMM tdhmm;
   TTag eos; // end-of-sentence tag

   /** It allocs memory for the transition (a) and the emission (b) matrices.
    *  Before calling this method the number of ambiguity classes must be known.
    *  This methos is called within read_ambiguity_classes and scan_for_ambiguity_classes.
    *  @see: read_ambiguity_classes, scan_for_ambiguity_classes
    */
   void init();
protected:
   void post_ambg_class_scan();
public:
   TaggerData& get_tagger_data();
   void deserialise(FILE *Serialised_FILE_Tagger);
   std::vector<std::wstring> &getArrayTags();
   void serialise(FILE *Stream_);
   void deserialise(const TaggerData &Deserialised_FILE_Tagger);
   void init_probabilities_from_tagged_text_(MorphoStream &stream_tagged,
                                             MorphoStream &stream_untagged);
   void init_probabilities_kupiec_(MorphoStream &lexmorfo);
   void train(MorphoStream &morpho_stream, unsigned long count);
   HMM();
   HMM(Apertium::TaggerFlags& Flags_);
   HMM(TaggerDataHMM *tdhmm);

   /** Constructor
    */
   HMM(TaggerDataHMM tdhmm);

   /** Destructor
    */
   ~HMM();

   /** Used to set the end-of-sentence tag
    *  @param t the end-of-sentence tag
    */
   void set_eos(TTag t);

   /** It reads the ambiguity classes from the stream received as
    *  input
    *  @param is the input stream
    */
   void read_ambiguity_classes(FILE *in);

   /** It writes the ambiguity classes to the stream received as
    *  a parameter
    *  @param iosthe output stream
    */
   void write_ambiguity_classes(FILE *out);

   /** It reads the probabilities (matrices a and b) from the stream
    *  received as a parameter
    *  @param is the input stream
    */
   void read_probabilities(FILE *in);

   /** It writes the probabilities (matrices a and b) to the stream
    *  received as a parameter
    *  @param os the output stream
    */
   void write_probabilities(FILE *out);

   /** It initializes the transtion (a) and emission (b) probabilities
    *  from an untagged input text by means of Kupiec's method
    *  @param is the input stream with the untagged corpus to process
    */
   void init_probabilities_kupiec(MorphoStream &lexmorfo);

   /** It initializes the transtion (a) and emission (b) probabilities
    *  from a tagged input text by means of the expected-likelihood
    *  estimate (ELE) method
    *  @param ftagged the input stream with the tagged corpus to process
    *  @param funtagged the same corpus to process but untagged
    */
   void init_probabilities_from_tagged_text(MorphoStream &stream_tagged,
                                            MorphoStream &stream_untagged);

   /** It applies the forbid and enforce rules found in tagger specification.
    *  To do so the transition matrix is modified by introducing null probabilities
    *  in the involved transitions.
    */
   void apply_rules();

   /** Unsupervised training algorithm (Baum-Welch implementation).
    *  @param morpho_stream the input stream with the untagged corpus to process
    */
   void train(MorphoStream &morpho_stream);

   /** Tagging algorithm (Viterbi implementation).
    *  @param in the input stream with the untagged text to tag
    *  @param out the output stream with the tagged text
    */
   void tagger(MorphoStream &morpho_stream, FILE *Output);

   /** Prints the A matrix.
    */
   void print_A();

   /** Prints the B matrix.
    */
   void print_B();

   /** Prints the ambiguity classes.
    */
   void print_ambiguity_classes();

   void filter_ambiguity_classes(FILE *in, FILE *out);
};

#endif
