#include <lttoolbox/ustring.h>
#include <lttoolbox/lt_locale.h>
#include <lttoolbox/input_file.h>
#include <lttoolbox/file_utils.h>
#include <lttoolbox/cli.h>
#include <unicode/normalizer2.h>
#include "filesystem.h"
#include <iostream>
#include <unicode/errorcode.h>
#include <unicode/putil.h>

#ifdef __MINGW32__
#include <windows.h>
#endif

int main(int argc, char* argv[])
{
  LtLocale::tryToSetLocale();
  CLI cli("Normalize text");
  cli.add_file_arg("data_file", false);
  cli.add_file_arg("input_file");
  cli.add_file_arg("output_file");
  cli.add_bool_arg('z', "null-flush", "flush output on the null character");
  cli.add_bool_arg('h', "help", "print this message and exit");
  cli.parse_args(argc, argv);

  UErrorCode err = U_ZERO_ERROR;
  auto pth = fs::absolute(cli.get_files()[0]);

  u_setDataDirectory(pth.parent_path().c_str());

  auto norm = icu::Normalizer2::getInstance("", pth.stem().c_str(),
					    UNORM2_COMPOSE, err);

  if (U_FAILURE(err)) {
    std::cerr << "Unable to load normalizer." << std::endl;
    std::cerr << u_errorName(err) << std::endl;
    return EXIT_FAILURE;
  }

  InputFile input;
  if (!cli.get_files()[1].empty()) {
    input.open_or_exit(cli.get_files()[1].c_str());
  }
  UFILE* output = openOutTextFile(cli.get_files()[2]);

  while (!input.eof()) {
    UString buf;
    UChar32 c = input.get();
    while (c != '\0' && c != '\\' && c != '[' && c != U_EOF) {
      buf += c;
      if (norm->hasBoundaryAfter(c)) {
	break;
      }
      c = input.get();
    }
    if (!buf.empty()) {
      auto s = norm->normalize(buf.c_str(), err);
      if (U_FAILURE(err)) {
	std::cerr << "Unable to normalize " << buf << std::endl;
	std::cerr << u_errorName(err) << std::endl;
	return EXIT_FAILURE;
      }
      u_fprintf(output, "%.*S", s.length(), s.getBuffer());
    }
    switch (c) {
    case '\0':
      u_fputc(c, output);
      u_fflush(output);
      break;
    case '\\':
      u_fputc(c, output);
      u_fputc(input.get(), output);
      break;
    case U_EOF:
      break;
    case '[':
      input.unget(c);
      buf = input.readBlank(true);
      write(buf, output);
      break;
    }
  }

  u_fclose(output);
  return EXIT_SUCCESS;
}
