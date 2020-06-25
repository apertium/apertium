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

#ifndef _TRANSFERWORD_
#define _TRANSFERWORD_

#include <map>
#include <apertium/apertium_re.h>
#include <string>
#include <cstdlib>

using namespace std;

/**
 * Word type for transfer modules
 */
class TransferWord
{
private:
  /**
   * Source language word
   */
  string s_str;

  /**
   * Target language word
   */
  string t_str;

  /**
   * Reference word
   */
  string r_str;
  
  /**
   * Wordbound blank
   */
  string b_str;

  /**
   * Queue length
   */
  int queue_length;

  /**
   * Copy method
   * @param o the object to be copied
   */
  void copy(TransferWord const &o);

  /**
   * Destroy method
   */
  void destroy();

  /**
   * Accesses the source/target/reference side of a word using the specified part
   * @param str typically s_str or t_str or r_str
   * @param part regular expression to match/access
   * @return reference to matched/accessed string
   */
  string access(string const &str, ApertiumRE const &part);

  /**
   * Assings a value to the source/target/reference side of a word using the
   * specified part
   * @param str typically s_str or t_str or r_str
   * @param part regular expression to match/access
   * @param value the string to be assigned
   */
  void assign(string &str, ApertiumRE const &part, string const &value);

public:
  /**
   * Non-parametric constructor
   */
  TransferWord();
  /**
   * Destructor
   */
  ~TransferWord();

  /**
   * Copy constructor
   * @param o the object to be copied
   */
  TransferWord(TransferWord const &o);

  /**
   * Parametric constructor calling init()
   * @param src source word
   * @param tgt target word
   * @param ref reference word
   * @param blank wordbound blank
   * @param queue queue lenght
   */
  TransferWord(string const &src, string const &tgt, string const &ref, string const &blank, int queue = 0);

  /**
   * Assignment operator
   * @param o the object to be assigned
   * @return reference to left part of assignment
   */
  TransferWord & operator =(TransferWord const &o);

  /**
   * Sets a tri-word (a source language word, its counterpart in target
   * language, and a reference if one exists
   * @param src source word
   * @param tgt target word
   * @param ref reference word
   * @param blank wordbound blank
   */
  void init(string const &src, string const &tgt, string const &ref, string const &blank);

  /**
   * Reference a source language word part
   * @param part regular expression to match
   * @param with_queue access taking into account the queue
   * @returns reference to the part of string matched
   */
  string source(ApertiumRE const &part, bool with_queue = true);

  /**
   * Reference a target language word part
   * @param part regular expression to match
   * @param with_queue access taking into account the queue
   * @returns reference to the part of string matched
   */
  string target(ApertiumRE const &part, bool with_queue = true);

  /**
   * Reference the reference word part
   * @param part regular expression to match
   * @param with_queue access taking into account the queue
   * @returns reference to the part of string matched
   */
  string reference(ApertiumRE const &part, bool with_queue = true);
  
  /**
   * Reference the wordbound blank part
   * @param part regular expression to match
   * @param with_queue access taking into account the queue
   * @returns reference to the part of string matched
   */
  string blank(ApertiumRE const &part, bool with_queue = true);

  /**
   * Sets a value for a source language word part
   * @param part regular expression to match
   * @param value the new value for the given part
   * @param with_queue access taking or not into account the queue
   * @returns whether part matched
   */
  bool setSource(ApertiumRE const &part, string const &value,
		 bool with_queue = true);

  /**
   * Sets a value for a target language word part
   * @param part regular expression to match
   * @param value the new value for the given part
   * @param with_queue access taking or not into account the queue
   * @returns whether part matched
   */
  bool setTarget(ApertiumRE const &part, string const &value,
		 bool with_queue = true);

  /**
   * Sets a value for a reference word part
   * @param part regular expression to match
   * @param value the new value for the given part
   * @param with_queue access taking or not into account the queue
   * @returns whether part matched
   */
  bool setReference(ApertiumRE const &part, string const &value,
     bool with_queue = true);
};

#endif
