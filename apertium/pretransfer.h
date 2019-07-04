#ifndef PRETRANSFER_H
#define PRETRANSFER_H

#include <iostream>
#include <string>

#include <lttoolbox/lt_locale.h>
#include <apertium/apertium_config.h>
#include <apertium/unlocked_cstdio.h>
#include <apertium/string_utils.h>

void readAndWriteUntil(FILE *input, FILE *output, int const charcode);
void procWord(FILE *input, FILE *output, bool surface_forms, bool compound_sep);
void processStream(FILE *input, FILE *output, bool null_flush, bool surface_forms, bool compound_sep);

#endif
