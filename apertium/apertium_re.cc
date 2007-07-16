#include <apertium/apertium_re.h>
#include <lttoolbox/compression.h>
#include <iostream>
#include <cstdlib>

using namespace std;

ApertiumRE::ApertiumRE()
{
  extra = NULL;
  empty = true;
}

ApertiumRE::~ApertiumRE()
{
  if(!empty)
  {
    delete reinterpret_cast<char *>(re);
  }
  empty = true;
}

void
ApertiumRE::read(FILE *input)
{
  unsigned int size = Compression::multibyte_read(input);
  re = reinterpret_cast<pcre *>(new char[size]);
  if(size != fread(re, 1, size, input))
  {
    cerr << L"Error reading regexp" << endl;
    exit(EXIT_FAILURE);
  }
  
  size = Compression::multibyte_read(input);
  if(size == 0)
  {
    extra = NULL;
  }
  else
  {
    extra = new pcre_extra;
    extra->study_data = (void *) new char[size];
    if(size != fread(extra->study_data, 1, size, input))
    {
      cerr << L"Error reading study data" << endl;
      exit(EXIT_FAILURE);
    }
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
  
  extra = pcre_study(re, PCRE_CASELESS|PCRE_EXTENDED|PCRE_UTF8, &error);
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
  
  int size;
  int rc = pcre_fullinfo(re, extra, PCRE_INFO_SIZE, &size);
  if(rc < 0)
  {
    wcerr << L"Error calling pcre_fullinfo()\n" << endl;
    exit(EXIT_FAILURE);
  }
  
  Compression::multibyte_write(size, output);
  
  rc = fwrite(re, 1, size, output);
  if(rc != size)
  {
    wcerr << L"Error writing precompiled regex\n" << endl;
    exit(EXIT_FAILURE);
  }              
  
  if(extra == NULL)
  {
    Compression::multibyte_write(0, output);
  }
  else
  {
    rc = pcre_fullinfo(re, extra, PCRE_INFO_STUDYSIZE, &size);
    if(rc < 0)
    {  
      wcerr << L"Error calling pcre_fullinfo()\n" << endl;
      exit(EXIT_FAILURE);
    }
  
    Compression::multibyte_write(size, output);
    rc = fwrite(extra->study_data, 1, size, output);
    if(rc != size)
    {
      wcerr << L"Error writing precompiled regex\n" << endl;
      exit(EXIT_FAILURE);
    }              
  }
}

string
ApertiumRE::match(string const &str) const
{
  int result[3];
  int workspace[4096];
//  int rc = pcre_exec(re, extra, str.c_str(), str.size(), 0, PCRE_NO_UTF8_CHECK, result, 3);
  int rc = pcre_dfa_exec(re, extra, str.c_str(), str.size(), 0, PCRE_NO_UTF8_CHECK, result, 3, workspace, 4096);

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
  int result[3];
  int workspace[4096];
  // int rc = pcre_exec(re, extra, str.c_str(), str.size(), 0, PCRE_NO_UTF8_CHECK, result, 3);
  int rc = pcre_dfa_exec(re, extra, str.c_str(), str.size(), 0, PCRE_NO_UTF8_CHECK, result, 3, workspace, 4096);
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
