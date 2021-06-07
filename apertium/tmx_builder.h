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
#ifndef _TMXBUILDER_
#define _TMXBUILDER_

#include <apertium/transfer_data.h>
#include <string>
#include <cstdio>
#include <lttoolbox/input_file.h>

using namespace std;

class TMXBuilder
{
private:
  UString lang1;
  UString lang2;
  unsigned int max_edit;
  unsigned int diagonal_width;
  unsigned int window_size;
  unsigned int step;
  double percent;
  double edit_distance_percent;
  unsigned int low_limit;
  FILE *freference;

  static UString nextTU(InputFile& input);
  static UString restOfBlank(InputFile& input);
  static UString nextBlank(InputFile& input);
  static UString xmlize(UString const &str);
  static bool compatible(InputFile& input, InputFile& output, bool lazy = false);
  void generateTMX(InputFile& f1, InputFile& f2, UFILE* output);
  void outputTU(InputFile& f1, InputFile& f2, UFILE* output);
  static vector<UString> reverseList(vector<UString> const &v);
  static vector<UString> sentenceList(InputFile& file);
  static int argmin(int nw, int n, int w);
  static int * levenshteinTable(vector<UString> &l1, vector<UString> &l2,
				unsigned int diagonal_width, unsigned int max_edit);
  void printTU(UFILE* output, UString const &tu1, UString const &tu2) const;
  static UString filter(UString const &s);
  static int weight(UString const &s);
  static void printTable(int *table, unsigned int nrows, unsigned int ncols);
  static int editDistance(UString const &s1, UString const &s2, unsigned int max_edit);
  static int min3(int i1, int i2, int i3);
  static int min2(int i1, int i2);
  void printTUCond(UFILE* output, UString const &s1, UString const &s2, bool secure_zone);
  static vector<UString> extractFragment(vector<UString> const &text, unsigned int base,
					 unsigned int width);

  static bool isRemovablePunct(wchar_t const &c);
  bool similar(UString const &s1, UString const &s2);

  void splitAndMove(InputFile& file, string const &filename);
public:
  TMXBuilder(UString const &l1, UString const &l2);
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
