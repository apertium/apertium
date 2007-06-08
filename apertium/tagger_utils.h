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
#ifndef __TAGGERUTILS_H
#define __TAGGERUTILS_H

#include <map>
#include <string>
#include <set>
#include <fstream>
#include <iostream>
#include <vector>
#include <apertium/ttag.h>

using namespace std;

/** Print a fatal error message
 *  @param s the error message to print
 */
void fatal_error (wstring const &s);

/** Print a fatal error message related to a file
 *  @param s the file name to be printted in the error message
 */
void file_name_error (string s);

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
 int ntokens_multiword(string s);
 
/** Devuelve el nº de guiones que contiene la cadena pasada como argumento
  */
int nguiones_fs(string cadena);

string trim(string s);

template <class T>
ostream& operator<< (ostream& os, const map <int, T> & f) {
  typename map <int, T>::const_iterator it;
  os<<f.size();
  for (it=f.begin(); it!=f.end(); it++) 
    os<<' '<<it->first<<' '<<it->second;
  return os;
}

template <class T>
istream& operator>> (istream& is, map <int, T> & f) {
  int n, i, k;
  f.clear();
  is>>n; 
  for (k=0; k<n; k++) {
    is>>i;     // warning: does not work if both
    is>>f[i];  // lines merged in a single one
  }
  if (is.bad()) fatal_error(L"reading map");
  return is;
}

template <class T>
ostream& operator<< (ostream& os, const set<T>& s) {
  typename set<T>::iterator it = s.begin();
  os<<'{';
  if (it!=s.end()) {
    os<<*it;
    while (++it!=s.end()) os<<','<<*it;
  }
  os<<'}';
  return os;
}

#endif
