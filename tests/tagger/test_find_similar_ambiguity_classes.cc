#include "apertium/utf_converter.h"
#include "apertium/tagger_utils.h"
#include "apertium/tagger_data_hmm.h"
#include "apertium/tagger_data.h"
#include <iostream>
#include <iostream>
#include <sstream>
#include <algorithm>

void print_ambiguity_class(const vector<wstring> &array_tags, const set<TTag> &abgset)
{
  unsigned int j;
  set<TTag>::const_iterator abgseti;
  for (abgseti=abgset.begin(), j=0; abgseti!=abgset.end(); abgseti++, j++) {
    wcout << array_tags[*abgseti];
    if (j < abgset.size() - 1) {
      wcout << " ";
    }
  }
}

void find_similar_ambiguity_class_io(TaggerData &td)
{
  vector<wstring> &array_tags = td.getArrayTags();
  wstring line = L"";
  getline(wcin, line, L'\n');

  wstringstream line_stream(line);
  set<TTag> ambiguity_class;
  wstring tag_name;
  while (line_stream >> tag_name) {
    vector<wstring>::iterator it;
    it = find(array_tags.begin(), array_tags.end(), tag_name);
    if (it == array_tags.end()) {
        wcerr << L"Tag not in model: " << tag_name << L'\n';
        exit(-3);
    }
    ambiguity_class.insert(it - array_tags.begin());
  }
  set<TTag> similar_ambiguity_class = tagger_utils::find_similar_ambiguity_class(td, ambiguity_class);
  print_ambiguity_class(array_tags, similar_ambiguity_class);
}

int main(int argc, char *argv[])
{
  if (argc < 2) {
    cerr<<"Usage: "<<argv[0]<<" <probfile>\n";
    exit(-1);
  }
  char* probfile = argv[1];
  TaggerDataHMM tagger_data_hmm;
  FILE* fin = fopen(probfile, "r");
  if (!fin) {
    cerr<<"Error: cannot open file '"<<probfile<<"'\n";
    exit(-2);
  }
  tagger_data_hmm.read(fin);
  fclose(fin);

  find_similar_ambiguity_class_io((TaggerData&)tagger_data_hmm);
}
