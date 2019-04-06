/*
 * CLExec.cpp
 *
 *  Created on: Jun 21, 2018
 *      Author: aboelhamd
 */

#include <iostream>
#include <stdexcept>
#include <stdio.h>
#include <string>
#include <unistd.h>
#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <algorithm>
#include <cfloat>
#include <sstream>
#include <apertium/string_utils.h>
#include <apertium/utf_converter.h>

#include "case_handler.h"

using namespace std;

string
CaseHandler::toLowerCase (string s)
{
  wstring ws = UtfConverter::fromUtf8 (s);
  ws = StringUtils::tolower (ws);
  s = UtfConverter::toUtf8 (ws);

  return s;
}

string
CaseHandler::toUpperCase (string s)
{
  wstring ws = UtfConverter::fromUtf8 (s);
  ws = StringUtils::toupper (ws);
  s = UtfConverter::toUtf8 (ws);

  return s;
}

string
CaseHandler::FirLetUpperCase (string s)
{
  wstring ws = UtfConverter::fromUtf8 (s);

  ws[0] = (wchar_t) towupper (ws[0]);

  s = UtfConverter::toUtf8 (ws);

  return s;
}

// The result of bitwise character comparison: 0 if this contains
// the same characters as text, -1 if the characters in this are
// bitwise less than the characters in text, +1 if the characters
// in this are bitwise greater than the characters in text.
int
CaseHandler::compare (string s1, string s2)
{
  wstring ws1 = UtfConverter::fromUtf8 (s1);
  wstring ws2 = UtfConverter::fromUtf8 (s2);

  return ws1.compare (ws2);
}

int
CaseHandler::compareCaseless (string s1, string s2)
{
  wstring ws1 = UtfConverter::fromUtf8 (s1);
  ws1 = StringUtils::tolower (ws1);
  wstring ws2 = UtfConverter::fromUtf8 (s2);
  ws2 = StringUtils::tolower (ws2);

  return ws1.compare (ws2);
}
