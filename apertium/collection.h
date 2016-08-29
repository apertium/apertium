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
#ifndef __COLLECTION_H
#define __COLLECTION_H

#include <cstdio>
#include <map>
#include <set>
#include <vector>

using namespace std;

/** Collection
 *  Is an indexed set.
 */ 
class Collection {
  map <set<int>, int> index;
  vector <const set<int> *> element;
public:
  /** Returns the collection's size. 
   */
  int size (void);

  /** Checks whether or not the collection has the element received as
   *  a parameter.  
   *  @param t element @return true if t is not in the
   *  collection
   */
  bool has_not (const set<int>& t);

  /** @param n position in the collection
   *  @return the element at the n-th position
   */
  const set<int>& operator[] (int n);

  /** If the element received as a parameter does not appear in the
   *  collection, it is added at the end.  
   *  @param t an element @return
   *  the position in which t appears in the collection.
   */
  int& operator[] (const set<int>& t);

  /** Adds an element to the collection
   *  @param t the element to be added
   */  
  int& add(const set<int>& t);

  /** 
   *  Write the collection contents to an output stream
   *  @param output the output stream
   */
  void write(FILE *output);

  /**
   *  Reads the collection contents from an input stream
   *  @param input the input stream
   */
  void read(FILE *input);

  void serialise(std::ostream &serialised) const;
  void deserialise(std::istream &serialised);
};


#endif
