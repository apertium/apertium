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
#ifndef _TMXBUILDER_
#define _TMXBUILDER_

#include <apertium/transfer_data.h>
#include <string>
#include <cstdio>

using namespace std;

class TMXBuilder
{
private:
  wstring lang1;
  wstring lang2;
  unsigned int max_edit;
  unsigned int diagonal_width;
  unsigned int window_size;
  unsigned int step;
  double percent;
  double edit_distance_percent;
  unsigned int low_limit;
  FILE *freference;

  static wstring nextTU(FILE *input);
  static wstring restOfBlank(FILE *input);
  static wstring nextBlank(FILE *input);
  static wstring xmlize(wstring const &str);
  static bool compatible(FILE *input, FILE *output, bool lazy = false);
  void generateTMX(FILE *f1, FILE *f2, FILE *output);
  void outputTU(FILE *f1, FILE *f2, FILE *output);
  static vector<wstring> reverseList(vector<wstring> const &v);
  static vector<wstring> sentenceList(FILE *file);
  static int argmin(int nw, int n, int w);
  static int * levenshteinTable(vector<wstring> &l1, vector<wstring> &l2, 
				unsigned int diagonal_width, unsigned int max_edit);
  void printTU(FILE *output, wstring const &tu1, wstring const &tu2) const;
  static wstring filter(wstring const &s);
  static int weight(wstring const &s);  
  static void printTable(int *table, unsigned int nrows, unsigned int ncols);
  static int editDistance(wstring const &s1, wstring const &s2, unsigned int max_edit);
  static int min3(int i1, int i2, int i3);
  static int min2(int i1, int i2);
  void printTUCond(FILE *output, wstring const &s1, wstring const &s2, bool secure_zone);
  static vector<wstring> extractFragment(vector<wstring> const &text, unsigned int base, 
					 unsigned int width);

  static bool isRemovablePunct(wchar_t const &c);
  bool similar(wstring const &s1, wstring const &s2);

  void splitAndMove(FILE *file, string const &filename);
public:
  TMXBuilder(wstring const &l1, wstring const &l2);
  ~TMXBuilder();
  static bool check(string const &file1, string const &file2, bool lazy = false);
  void generate(string const &file1, string const &file2, 
                string const &outfile="");
                
  void setMaxEdit(int me);
  void setDiagonalWidth(int dw);
  void setWindowSize(int ws);
  void setStep(int s);
  void setPercent(double p);
  void setLowLimit(int l);
  void setEditDistancePercent(double e);
  void setTranslation(string const &filename);
};

#endif
