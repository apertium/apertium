

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

#include <lttoolbox/lt_locale.h>
#ifndef GENFORMAT
#include "apertium_config.h"
#endif
#include <apertium/unlocked_cstdio.h>
#include <utf8/utf8.h>
#ifdef _WIN32
#include <io.h>
#include <fcntl.h>
#endif

using namespace std;

AccentsMap accentsMap(true);

%}


%option nounput
%option noyywrap
%option stack

%x mathenv
%x readbrackets

%%



&quot;	{
	fputs("\"",yyout);
}
&apos;	{
	fputs("\'",yyout);
}
&lt;	{
	fputs("<",yyout);
}
&gt;	{
	fputs(">",yyout);
}
&amp;	{
	fputs("\\&",yyout);
}
\<AMP\/\>	{
	fputs("&",yyout);
}

\<LEFTESCAPEDBRACE\/\>	{
        fputs("\\{", yyout);
}

\<RIGHTESCAPEDBRACE\/\>	{
        fputs("\\}", yyout);
}

\<ESCAPEDPERCENT\/\>	{
        fputs("\\%", yyout);
}

¿	{
	fputs("?`",yyout);
}

¡	{
	fputs("!`",yyout);
}



\<MATH_DOLLARS\>	{
	BEGIN(mathenv);
	fputs("$$",yyout);
}

<mathenv>\<\/MATH_DOLLARS\>	{
	fputs("$$",yyout);
	BEGIN(0);
}


\<MATH_DOLLAR\>	{
	BEGIN(mathenv);
	fputs("$",yyout);
}

<mathenv>\<\/MATH_DOLLAR\>	{
	fputs("$",yyout);
	BEGIN(0);
}

\<MATH_PAR\>	{
	fputs("\\(",yyout);
}

\<\/MATH_PAR\>	{
	fputs("\\)",yyout);
}

\<MATH_BRA\>	{
	fputs("\\[",yyout);
}

\<\/MATH_BRA\>	{
	fputs("\\]",yyout);
}


\<CONTENTS\>	{
	fputs("{",yyout);
}

\<\/CONTENTS\>	{
	fputs("}",yyout);
}

&NBSP;	{
	fputs("~",yyout);
}



\<BR\/\>	{
	fputs("\\\\",yyout);
}

\<COMMENT\>[^\<]*	{
    fputs("\%", yyout);
    fputrange(yytext, 9, yyleng-9, yyout);
}

\<\/COMMENT\>	{
}


\<PARAM\>[^\<]*	{
    fputc('[', yyout);
    fputrange(yytext, 7, yyleng-7, yyout);
}
\<\/PARAM\>	{
	fputs("]", yyout);
}

\<VERB\>	{
        fputs("\\verb", yyout);
}

\<\/VERB\>	{
        ;
}



ł	{
	fputs("\\l", yyout);
}


œ	{
	fputs("{\\oe}",yyout);
}

Œ	{
	fputs("{\\OE}",yyout);
}

æ	{
	fputs("{\\ae}",yyout);
}

Æ	{
	fputs("{\\AE}",yyout);
}

å	{
	fputs("{\\aa}",yyout);
}

Å	{
	fputs("{\\AA}",yyout);
}

ø	{
	fputs("{\\o}",yyout);
}

Ø	{
	fputs("{\\O}",yyout);
}

ß	{
	fputs("{\\ss}",yyout);
}

\<[a-zA-Z0-9]+\>	{
    fputs("\\begin{", yyout);
    fputrange(yytext, 1, yyleng-2, yyout);
    fputc('}', yyout);
}

\<HASH_[0-9]+\/\>	{
    fputs("\\#", yyout);
    fputrange(yytext, 6, yyleng-8, yyout);
}

\<HASH\/\>		{
        fputs("\\#", yyout);
}

\<[a-zA-Z0-9]+_STAR\>	{
    fputs("\\begin{", yyout);
    fputrange(yytext, 1, yyleng-7, yyout);
    fputs("*}", yyout);
}

\<\/[a-zA-Z0-9]+\>	{
    fputs("\\end{", yyout);
    fputrange(yytext, 2, yyleng-3, yyout);
    fputc('}', yyout);
}

\<\/[a-zA-Z0-9]+_STAR\>	{
    fputs("\\end{", yyout);
    fputrange(yytext, 2, yyleng-8, yyout);
    fputs("*}", yyout);
}

\<[a-zA-Z0-9]+\/\>	{
    fputc('\\', yyout);
    fputrange(yytext, 1, yyleng-3, yyout);
}

\<[a-zA-Z0-9]+_STAR\/\>	{
    fputc('\\', yyout);
    fputrange(yytext, 1, yyleng-8, yyout);
    fputc('*', yyout);
}
 /*NO ENTIENDO ESTA REGLA
 \#	{
        fputs("\\#", yyout);
 }*/


[^A-Za-z\n]	{
    UString wt = to_ustring(yytext);
	UString wa = accentsMap.get(wt);
	if (wa.empty()) {
		fputus(wt, yyout);
 	} else {
        UString tmp;
        tmp += '\\';
        tmp += wa[0];
        tmp += '{';
        tmp += wa.substr(1);
        tmp += '}';
        fputus(tmp, yyout);
    }
}


(.|\n)	{
	fputs(yytext, yyout);
}

<mathenv>(.|\n)	{
	fputs(yytext, yyout);
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

  // prevent warning message
  yy_push_state(1);
  yy_top_state();
  yy_pop_state();

  yylex();

  fclose(yyin);
  fclose(yyout);
}
