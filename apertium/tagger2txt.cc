#include <lttoolbox/lt_locale.h>
#include <apertium/tagger_data_exe.h>
#include <lttoolbox/ustring.h>
#include <iostream>
#include <cstdio>

using namespace std;

void out(StringWriter& sw, str_int* arr, uint64_t ct, const char* name)
{
  if (ct) {
    cout << name << " {\n";
    for (uint64_t i = 0; i < ct; i++) {
      cout << sw.get(arr[i].s) << "\t" << arr[i].i << endl;
    }
    cout << "}\n";
  }
}

void out(StringWriter& sw, str_str_int* arr, uint64_t ct, const char* name)
{
  if (ct) {
    cout << name << " {\n";
    for (uint64_t i = 0; i < ct; i++) {
      cout << sw.get(arr[i].s1) << "\t" << sw.get(arr[i].s2) << "\t";
      cout << arr[i].i << endl;
    }
    cout << "}\n";
  }
}

int main(int argc, char* argv[])
{
  LtLocale::tryToSetLocale();

  if (argc != 3) {
    cerr << "USAGE: apertium-tagger2txt [ h | l | 1 | 2 | 3 | x ] prob_file\n";
    exit(EXIT_FAILURE);
  }

  TaggerDataExe tde;
  FILE* fin = fopen(argv[2], "rb");
  if (!fin) {
    cerr << "Unable to open '" << argv[2] << "' for reading.\n";
    exit(EXIT_FAILURE);
  }
  switch (argv[1][0]) {
  case 'h':
    tde.read_compressed_hmm_lsw(fin, true);
    cout << "Type: HMM\n";
    break;
  case 'l':
    tde.read_compressed_hmm_lsw(fin, false);
    cout << "Type: LSW\n";
    break;
  case '1':
    tde.read_compressed_unigram1(fin);
    cout << "Type: Unigram1\n";
    break;
  case '2':
    tde.read_compressed_unigram2(fin);
    cout << "Type: Unigram2\n";
    break;
  case '3':
    tde.read_compressed_unigram3(fin);
    cout << "Type: Unigram3\n";
    break;
  case 'x':
  default:
    cerr << "Unrecognized prob type '" << argv[1] << "'\n";
    exit(EXIT_FAILURE);
  }

  out(tde.str_write, tde.uni1, tde.uni1_count, "Data");
  out(tde.str_write, tde.uni2, tde.uni2_count, "Data");
  out(tde.str_write, tde.uni3_l_t, tde.uni3_l_t_count, "Data_l_t");
  out(tde.str_write, tde.uni3_cl_ct, tde.uni3_cl_ct_count, "Data_cl_ct");
  out(tde.str_write, tde.uni3_ct_cl, tde.uni3_ct_cl_count, "Data_ct_cl");
  return 0;
}
