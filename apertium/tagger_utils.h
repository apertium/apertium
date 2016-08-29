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
#ifndef __TAGGERUTILS_H
#define __TAGGERUTILS_H

#include <map>
#include <string>
#include <set>
#include <fstream>
#include <iostream>
#include <vector>
#include <apertium/ttag.h>
#include <cstdlib>
#include <apertium/tagger_data.h>
#include <apertium/tagger_word.h>
#include <apertium/morpho_stream.h>

using namespace std;

namespace tagger_utils
{
/** Print a fatal error message
 *  @param s the error message to print
 */
void fatal_error (wstring const &s);

/** Print a fatal error message related to a file
 *  @param s the file name to be printted in the error message
 */
void file_name_error (string const &s);

/** Convert from int to string
 *  @param i the int value to convert
 *  @return an string representing the number recived as input
 */
char *itoa(int i);

/** Make all array positions equal to zero
 *  @param a the array
 *  @param l length of the array a
 */
void clear_array_double(double a[], int l);

/** Clear all vectors stored in array v
 *  @param v array of vectors
 *  @param l length of the array v
 */
void clear_array_vector(vector<TTag> v[], int l);

/** Return the number of tokens in the multiword unit
 */
 int ntokens_multiword(wstring const &s);
 
/** Devuelve el nº de guiones que contiene la cadena pasada como argumento
  */
int nguiones_fs(wstring const &cadena);

/** Reads the expanded dictionary received as a parameter puts the resulting
 *  ambiguity classes that the tagger will manage.
 *  @param fdic the input stream with the expanded dictionary to read
 *  @param td the tagger data instance to mutate
 */
void scan_for_ambg_classes(FILE *fdic, TaggerData &td);
void scan_for_ambg_classes(Collection &output, MorphoStream &morpho_stream);

void add_neccesary_ambg_classes(TaggerData &td);

void init_ambg_class_freq(TaggerData &td);

void count_ambg_class_freq(FILE *fdic, TaggerData &td);
void count_ambg_class_freq(Collection &output, vector<unsigned int> &acc,
                           MorphoStream &morpho_stream);

/** This method returns a known ambiguity class that is a subset of
*  the one received as a parameter. This is useful when a new
*  ambiguity class is found because of changes in the morphological
*  dictionary used by the MT system.
*  @param c set of tags (ambiguity class)
*  @return a known ambiguity class
*/
set<TTag> & find_similar_ambiguity_class(TaggerData &td, set<TTag> &c);

/** Dies with an error message if the tags aren't in the tagger data */
void require_ambiguity_class(TaggerData &td, set<TTag> &tags, TaggerWord &word, int nw);

/** As with find_similar_ambiguity_class, but returns tags if it's already fine
 * & prints a warning if warn */
set<TTag> & require_similar_ambiguity_class(TaggerData &td, set<TTag> &tags, TaggerWord &word, bool warn);
set<TTag> & require_similar_ambiguity_class(TaggerData &td, set<TTag> &tags);

/** Just prints a warning if warn */
void warn_absent_ambiguity_class(TaggerData &td, set<TTag> &tags, TaggerWord &word, bool warn);

wstring trim(wstring s);

};

template <class T>
ostream& operator<< (ostream& os, const map <int, T> & f);
template <class T>
istream& operator>> (istream& is, map <int, T> & f);
template <class T>
ostream& operator<< (ostream& os, const set<T>& s);

#endif
