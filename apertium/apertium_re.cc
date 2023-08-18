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
#include <apertium/apertium_re.h>
#include <lttoolbox/compression.h>
#include <iostream>
#include <cstdlib>
#include <lttoolbox/string_utils.h>

using namespace std;
using namespace icu;

ApertiumRE::ApertiumRE() {}

ApertiumRE::~ApertiumRE()
{
  if (re != nullptr) {
    delete re;
  }
}

void
ApertiumRE::read(FILE *input)
{
  unsigned int size = Compression::multibyte_read(input);
  if (fseek(input, size, SEEK_CUR) != 0) {
    I18n(APER_I18N_DATA, "apertium").error("APER1017", {}, {}, true);
  }
}

void
ApertiumRE::compile(UString const &str)
{
  if (re != nullptr) {
    delete re;
  }
  UnicodeString s = str.c_str();
  UErrorCode err = U_ZERO_ERROR;
  re = RegexPattern::compile(s, UREGEX_DOTALL|UREGEX_CASE_INSENSITIVE, err);
  if(U_FAILURE(err)) {
    I18n(APER_I18N_DATA, "apertium").error("APER1018", {"exp", "error"},
                                                       {icu::UnicodeString(str.data()), u_errorName(err)}, true);
  }
}

void
ApertiumRE::write(FILE *output) const
{
  if (re == nullptr) {
    I18n(APER_I18N_DATA, "apertium").error("APER1019", {}, {}, true);
  }
  // for backwards compatibility, write empty binary form
  Compression::multibyte_write(0, output);
}

UString
ApertiumRE::match(UString const &str) const
{
  if(re == nullptr) {
    return ""_u;
  }

  UnicodeString s = str.c_str();
  UErrorCode err = U_ZERO_ERROR;
  RegexMatcher* m = re->matcher(s, err);

  if (U_FAILURE(err)) {
    I18n(APER_I18N_DATA, "apertium").error("APER1020", {"error"},
                                                       {u_errorName(err)}, true);
  }

  if (!m->find()) {
    delete m;
    return ""_u;
  }

  UString ret = m->group(err).getTerminatedBuffer();
  if (U_FAILURE(err)) {
    I18n(APER_I18N_DATA, "apertium").error("APER1021", {"error"},
                                                       {u_errorName(err)}, true);
  }

  delete m;

  return ret;
}

// Return true if something was replaced and false otherwise
bool
ApertiumRE::replace(UString &str, UString const &value) const
{
  if(re == nullptr) {
    return false;
  }

  UnicodeString s = str.c_str();
  UErrorCode err = U_ZERO_ERROR;
  RegexMatcher* m = re->matcher(s, err);

  if (U_FAILURE(err)) {
    I18n(APER_I18N_DATA, "apertium").error("APER1020", {"error"},
                                                       {u_errorName(err)}, true);
  }

  // do this manually rather than call m->replaceFirst()
  // because we want to know that a match happened
  if (!m->find()) {
    delete m;
    return false;
  }
  UString res = str.substr(0, m->start(err));
  res.append(value);
  res.append(str.substr(m->end(err)));
  res.swap(str);
  delete m;
  return true;
}
