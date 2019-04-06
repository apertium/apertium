/*
 * CLExec.h
 *
 *  Created on: Jun 21, 2018
 *      Author: aboelhamd
 */

#ifndef SRC_CASE_HANDLER_H_
#define SRC_CASE_HANDLER_H_

#include <string>
#include <vector>

#include "ambiguous_chunker.h"

using namespace std;

class CaseHandler
{
public:

  static string
  toLowerCase (string s);

  static string
  toUpperCase (string s);

  static string
  FirLetUpperCase (string s);

  static int
  compare (string s1, string s2);

  static int
  compareCaseless (string s1, string s2);

};

#endif /* SRC_CASE_HANDLER_H_ */
