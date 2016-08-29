#ifndef SHELL_UTILS_H
#define SHELL_UTILS_H

#include <fstream>
#include <cstdio>

namespace Apertium {
namespace ShellUtils {

void
expect_file_arguments(int actual, int lower, int upper);

void
expect_file_arguments(int actual, int exactly);

template <typename T>
void
try_open_fstream(const char *metavar, const char *filename,
                      T &stream);

FILE*
try_open_file(const char *metavar, const char *filename,
              const char *flags);

FILE*
try_open_file_utf8(const char *metavar, const char *filename,
                   const char *flags);

void try_close_file(const char *metavar, const char *filename,
                    FILE *file);

}
}

#endif
