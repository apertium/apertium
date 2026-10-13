#include <apertium/tagger_exe.h>
#include <lttoolbox/lt_locale.h>

#include <iostream>

int main(int argc, char** argv)
{
  LtLocale::tryToSetLocale();
  TaggerExe t;
  InputFile input;
  UFILE* output = u_finit(stdout, NULL, NULL);
  FILE* tg = fopen(argv[1], "rb");
  t.load(tg);
  t.tag_hmm(input, output);
  return EXIT_SUCCESS;
}
