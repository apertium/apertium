#include <apertium/exception.h>
#include <apertium/shell_utils.h>
#include <i18n.h>

#ifdef _MSC_VER
#include <fcntl.h>
#include <io.h>
#endif // _MSC_VER

namespace Apertium {
namespace ShellUtils {

void expect_file_arguments(int actual, int lower, int upper) {
  if (actual < lower || actual >= upper) {
    std::stringstream what_;
    for (int i=lower;i<upper;i++) {
      what_ << i;
      if (i < upper - 1) {
        what_ << ", ";
      }
      if (i == upper - 2) {
        what_ << "or ";
      }
    }
    icu::UnicodeString msg = I18n(APER_I18N_DATA, "apertium").format("APER1099",
      {"expected", "actual"}, {what_.str().c_str(),actual});
    throw Exception::Shell::UnexpectedFileArgumentCount(msg);
  }
}

void expect_file_arguments(int actual, int exactly) {
  expect_file_arguments(actual, exactly, exactly + 1);
}

template <typename T>
void try_open_fstream(const char *metavar, const char *filename,
                      T &stream) {
  stream.open(filename);
  if (stream.fail()) {
    std::stringstream what_;
    what_ << "can't open " << metavar << " file \"" << filename << "\"";
    throw Exception::Shell::StreamOpenError(what_);
  }
}

template
void
try_open_fstream(const char *metavar, const char *filename,
                 std::ifstream &stream);

template
void
try_open_fstream(const char *metavar, const char *filename,
                 std::ofstream &stream);

template
void
try_open_fstream(const char *metavar, const char *filename,
                 std::wifstream &stream);

template
void
try_open_fstream(const char *metavar, const char *filename,
                 std::wofstream &stream);

FILE *try_open_file(const char *metavar, const char *filename,
                           const char *flags) {
  FILE *f = std::fopen(filename, flags);
  if (f == NULL) {
    throw Exception::Shell::FopenError(I18n(APER_I18N_DATA, "apertium").format("APER1100",
      {"metavar", "filename"}, {metavar, filename}));
  }
  return f;
}

UFILE* try_open_file_utf8(const char *metavar, const char *filename,
                                       const char *flags) {
  UFILE* f = u_fopen(filename, flags, NULL, NULL);
  if (f == NULL) {
    std::stringstream what_;
    throw Exception::Shell::FopenError(I18n(APER_I18N_DATA, "apertium").format("APER1100",
      {"metavar", "filename"}, {metavar, filename}));
  }
  return f;
}

void try_close_file(const char *metavar, const char *filename,
                           FILE *file) {
  if (std::fclose(file) != 0) {
    std::stringstream what_;
    throw Exception::Shell::FcloseError(I18n(APER_I18N_DATA, "apertium").format("APER1100",
      {"metavar", "filename"}, {metavar, filename}));
  }
}
}
}
