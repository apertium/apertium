#include <apertium/exception.h>
#include <apertium/shell_utils.h>

#ifdef _MSC_VER
#include <fcntl.h>
#include <io.h>
#endif // _MSC_VER

namespace Apertium {
namespace ShellUtils {

void expect_file_arguments(int actual, int lower, int upper) {
  if (actual < lower || actual >= upper) {
    std::stringstream what_;
    what_ << "expected ";
    for (int i=lower;i<upper;i++) {
      what_ << i;
      if (i < upper - 1) {
        what_ << ", ";
      }
      if (i == upper - 2) {
        what_ << "or ";
      }
    }
    what_ << " file arguments, got " << actual;
    throw Exception::Shell::UnexpectedFileArgumentCount(what_);
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
    std::stringstream what_;
    what_ << "can't open " << metavar << " file \"" << filename << "\"";
    throw Exception::Shell::FopenError(what_);
  }
  return f;
}

FILE *try_open_file_utf8(const char *metavar, const char *filename,
                                       const char *flags) {
  FILE *f = try_open_file(metavar, filename, flags);
#ifdef _MSC_VER
  _setmode(_fileno(f), _O_U8TEXT);
#endif // _MSC_VER
  return f;
}

void try_close_file(const char *metavar, const char *filename,
                           FILE *file) {
  if (std::fclose(file) != 0) {
    std::stringstream what_;
    what_ << "can't close " << metavar << " file \"" << filename << "\"";
    throw Exception::Shell::FcloseError(what_);
  }
}
}
}
