#include <apertium/apertium_re.h>
#include <lttoolbox/compression.h>
#include <iostream>
#include <cstdlib>
#include <apertium/string_utils.h>

using namespace Apertium;
using namespace std;

ApertiumRE::ApertiumRE()
{
  empty = true;
}

ApertiumRE::~ApertiumRE()
{
  if(!empty)
  {
    pcre_free(re);
  }
  empty = true;
}

void
ApertiumRE::read(FILE *input)
{
  unsigned int size = Compression::multibyte_read(input);
  re = static_cast<pcre *>(pcre_malloc(size));
  if(size != fread(re, 1, size, input))
  {
    cerr << L"Error reading regexp" << endl;
    exit(EXIT_FAILURE);
  }
  
  empty = false;
}

void
ApertiumRE::compile(string const &str)
{
  const char *error;
  int erroroffset;
  re = pcre_compile(str.c_str(), PCRE_CASELESS|PCRE_EXTENDED|PCRE_UTF8,
	            &error, &erroroffset, NULL);
  if(re == NULL)
  {
    wcerr << L"Error: pcre_compile ";
    cerr << error << endl;
    exit(EXIT_FAILURE);
  }
  
  empty = false;
}

void 
ApertiumRE::write(FILE *output) const
{
  if(empty)
  {
    cerr << L"Error, cannot write empty regexp" << endl;
    exit(EXIT_FAILURE);
  }
  
  size_t size;
  int rc = pcre_fullinfo(re, NULL, PCRE_INFO_SIZE, &size);
  if(rc < 0)
  {
    wcerr << L"Error calling pcre_fullinfo()\n" << endl;
    exit(EXIT_FAILURE);
  }
  
  Compression::multibyte_write(size, output);
  
  size_t rc2 = fwrite(re, 1, size, output);
  if(rc2 != size)
  {
    wcerr << L"Error writing precompiled regex\n" << endl;
    exit(EXIT_FAILURE);
  }                
}

string
ApertiumRE::match(string const &str) const
{
  if(empty)
  {
    return "";
  }
  
  int result[3];
  int workspace[4096];
//  int rc = pcre_exec(re, NULL, str.c_str(), str.size(), 0, PCRE_NO_UTF8_CHECK, result, 3);
  int rc = pcre_dfa_exec(re, NULL, str.c_str(), str.size(), 0, PCRE_NO_UTF8_CHECK, result, 3, workspace, 4096);

  if(rc < 0)
  {
    switch(rc)
    {
      case PCRE_ERROR_NOMATCH:
	return "";

      default:
	wcerr << L"Error: Unknown error matching regexp (code " << rc << L")" << endl;
	exit(EXIT_FAILURE);
    }
  }
  
  return str.substr(result[0], result[1]-result[0]);
}

void
ApertiumRE::replace(string &str, string const &value) const
{
  if(empty)
  {
    return;
  }
  
  int result[3];
  int workspace[4096];
  // int rc = pcre_exec(re, NULL, str.c_str(), str.size(), 0, PCRE_NO_UTF8_CHECK, result, 3);
  int rc = pcre_dfa_exec(re, NULL, str.c_str(), str.size(), 0, PCRE_NO_UTF8_CHECK, result, 3, workspace, 4096);
  if(rc < 0)
  {
    switch(rc)
    {
      case PCRE_ERROR_NOMATCH:
	return;
      
      default:
	wcerr << L"Error: Unknown error matching regexp (code " << rc << L")" << endl;
	exit(EXIT_FAILURE);
    }
  }

  string res = str.substr(0, result[0]);
  res.append(value);
  res.append(str.substr(result[1]));
  str = res;
}
