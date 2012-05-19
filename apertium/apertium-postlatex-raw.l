

%{



#include <cstdlib>
#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <apertium/latex_accentsmap.h>

extern "C" {
#if !defined(__STDC__)
# define __STDC__ 1
#endif
#include <regex.h>
}

#include <string>
#include <lttoolbox/lt_locale.h>
#include <lttoolbox/ltstr.h>
#ifndef GENFORMAT
#include "apertium_config.h"
#endif
#include <apertium/unlocked_cstdio.h>
#ifdef _MSC_VER
#include <io.h>
#include <fcntl.h>
#endif

using namespace std;

AccentsMap accentsMap(true);
wstring closesym = L"";
string memconv = "";

wstring convertir(string const &multibyte, int const length)
{
  memconv.append(multibyte.c_str(), length);
  int tam = memconv.size();
  wchar_t *retval = new wchar_t[tam+1];
  size_t l = mbstowcs(retval, memconv.c_str(), tam);

  if(l == ((size_t) -1))
  {
    delete[] retval;
    if(memconv.size() >= 4)
    { 
      wcerr << L"Warning: wrong encoding" << endl;
    }
    return L"";
  }
  else
  {   
    memconv = "";
    retval[l] = 0;
    wstring ret = retval;
    delete[] retval;
    return ret;
  }
}  




%}


%option nounput
%option noyywrap
%option stack

%x mathenv
%x readbrackets

%%



&quot;	{
	fputws(L"\"",yyout);
}
&apos;	{
	fputws(L"\'",yyout);
}
&lt;	{
	fputws(L"<",yyout);
}
&gt;	{
	fputws(L">",yyout);
}
&amp;	{
	fputws(L"\\&",yyout);
}
\<AMP\/\>	{
	fputws(L"&",yyout);
}

\<LEFTESCAPEDBRACE\/\>	{
        fputws(L"\\{", yyout);
}

\<RIGHTESCAPEDBRACE\/\>	{
        fputws(L"\\}", yyout);
}

\<ESCAPEDPERCENT\/\>	{
        fputws(L"\\%", yyout);
}



¿	{
	fputws(L"?`",yyout);
}

¡	{
	fputws(L"!`",yyout);
}



\<MATH_DOLLARS\>	{
	BEGIN(mathenv);
	fputws(L"$$",yyout);
}

<mathenv>\<\/MATH_DOLLARS\>	{
	fputws(L"$$",yyout);
	BEGIN(0);
}


\<MATH_DOLLAR\>	{
	BEGIN(mathenv);
	fputws(L"$",yyout);
}

<mathenv>\<\/MATH_DOLLAR\>	{
	fputws(L"$",yyout);
	BEGIN(0);
}

\<MATH_PAR\>	{
	fputws(L"\\(",yyout);
}

\<\/MATH_PAR\>	{
	fputws(L"\\)",yyout);
}

\<MATH_BRA\>	{
	fputws(L"\\[",yyout);
}

\<\/MATH_BRA\>	{
	fputws(L"\\]",yyout);
}


\<CONTENTS\>	{
	fputws(L"{",yyout);
}

\<\/CONTENTS\>	{
	fputws(L"}",yyout);
}

&NBSP;	{
	fputws(L"~",yyout);
}



\<BR\/\>	{
	fputws(L"\\\\",yyout);
}

\<COMMENT\>[^\<]*	{
	fputws((wstring(L"\%")+convertir(yytext+9,yyleng-9)).c_str(),yyout);
}

\<\/COMMENT\>	{
}


\<PARAM\>[^\<]*	{
	fputws((wstring(L"[")+convertir(yytext+7,yyleng-7)).c_str(),yyout);
}
\<\/PARAM\>	{
	fputws(L"]", yyout);
}

\<VERB\>	{
        fputws(L"\\verb", yyout);
}

\<\/VERB\>	{
        ;
}


\<[a-zA-Z0-9]+\>	{
	fputws((wstring(L"\\begin{")+convertir(yytext+1,yyleng-2)+wstring(L"}")).c_str(),yyout);
}

\<[a-zA-Z0-9]+_STAR\>	{
	fputws((wstring(L"\\begin{")+convertir(yytext+1,yyleng-7)+wstring(L"*}")).c_str(),yyout);
}

\<\/[a-zA-Z0-9]+\>	{
	fputws((wstring(L"\\end{")+convertir(yytext+2,yyleng-3)+wstring(L"}")).c_str(),yyout);
}

\<\/[a-zA-Z0-9]+_STAR\>	{
	fputws((wstring(L"\\end{")+convertir(yytext+2,yyleng-8)+wstring(L"*}")).c_str(),yyout);
}

\<[a-zA-Z0-9]+\/\>	{
	fputws((wstring(L"\\")+convertir(yytext+1,yyleng-3)).c_str(),yyout);
}

\<[a-zA-Z0-9]+_STAR\/\>	{
	fputws((wstring(L"\\")+convertir(yytext+1,yyleng-8)+wstring(L"*")).c_str(),yyout);
}

\#	{
        fputws(L"\\#", yyout);
}


(.|\n)	{
	fputws(convertir(yytext,yyleng).c_str(),yyout);
}

<mathenv>(.|\n)	{
	fputws(convertir(yytext,yyleng).c_str(),yyout);
}


<<EOF>>	{
	return 0;
}
%%



void usage(string const &progname)
{

  cerr << "USAGE: " << progname << " [input_file [output_file]" << ']' << endl;
  
  cerr << "LaTeX format postprocessor " << endl;
  exit(EXIT_SUCCESS);  
}

int main(int argc, char *argv[])
{
  LtLocale::tryToSetLocale();
  size_t base = 0;
  
  if(argc >= 2 && !strcmp(argv[1],"-i"))
  {
    base++;
  }

 if((argc-base) > 4)
  {
    usage(argv[0]);
  }

  switch(argc-base)
  {
    case 3:
      yyout = fopen(argv[2+base], "w");
      if(!yyout)
      {
        usage(argv[0]);
      }
    case 2:
      yyin = fopen(argv[1+base], "r");
      if(!yyin)
      {
        usage(argv[0]);
      }
      break;
    default:
      break;
  }
  
#ifdef _MSC_VER
  _setmode(_fileno(yyin), _O_U8TEXT);
  _setmode(_fileno(yyout), _O_U8TEXT);
#endif 
  // prevent warning message
  yy_push_state(1);
  yy_top_state();
  yy_pop_state();

  yylex();

  fclose(yyin);
  fclose(yyout);
}
