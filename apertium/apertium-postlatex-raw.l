

%{



#include <cstdlib>
#include <iostream>
#include <string>

extern "C" {
#if !defined(__STDC__)
# define __STDC__ 1
#endif
#include <regex.h>
}

#include <lttoolbox/lt_locale.h>
#include <apertium/unlocked_cstdio.h>

using namespace std;

%}


%option nounput
%option noyywrap
%option stack

%x mathenv
%x readbrackets

%%



&quot;	{
	fputs_unlocked("\"",yyout);
}
&apos;	{
	fputs_unlocked("\'",yyout);
}
&lt;	{
	fputs_unlocked("<",yyout);
}
&gt;	{
	fputs_unlocked(">",yyout);
}
&amp;	{
	fputs_unlocked("\\&",yyout);
}
\<AMP\/\>	{
	fputs_unlocked("&",yyout);
}

\<LEFTESCAPEDBRACE\/\>	{
        fputs_unlocked("\\{", yyout);
}

\<RIGHTESCAPEDBRACE\/\>	{
        fputs_unlocked("\\}", yyout);
}

\<ESCAPEDPERCENT\/\>	{
        fputs_unlocked("\\%", yyout);
}



¿	{
	fputs_unlocked("?`",yyout);
}

¡	{
	fputs_unlocked("!`",yyout);
}



\<MATH_DOLLARS\>	{
	BEGIN(mathenv);
	fputs_unlocked("$$",yyout);
}

<mathenv>\<\/MATH_DOLLARS\>	{
	fputs_unlocked("$$",yyout);
	BEGIN(0);
}


\<MATH_DOLLAR\>	{
	BEGIN(mathenv);
	fputs_unlocked("$",yyout);
}

<mathenv>\<\/MATH_DOLLAR\>	{
	fputs_unlocked("$",yyout);
	BEGIN(0);
}

\<MATH_PAR\>	{
	fputs_unlocked("\\(",yyout);
}

\<\/MATH_PAR\>	{
	fputs_unlocked("\\)",yyout);
}

\<MATH_BRA\>	{
	fputs_unlocked("\\[",yyout);
}

\<\/MATH_BRA\>	{
	fputs_unlocked("\\]",yyout);
}


\<CONTENTS\>	{
	fputs_unlocked("{",yyout);
}

\<\/CONTENTS\>	{
	fputs_unlocked("}",yyout);
}

&NBSP;	{
	fputs_unlocked("~",yyout);
}



\<BR\/\>	{
	fputs_unlocked("\\\\",yyout);
}

\<COMMENT\>[^\<]*	{
	fputs_unlocked("\%", yyout);
	fwrite(yytext+9, 1, yyleng-9, yyout);
}

\<\/COMMENT\>	{
}


\<PARAM\>[^\<]*	{
	fputs_unlocked("[", yyout);
	fwrite(yytext+7, 1, yyleng-7, yyout);
}
\<\/PARAM\>	{
	fputs_unlocked("]", yyout);
}

\<VERB\>	{
        fputs_unlocked("\\verb", yyout);
}

\<\/VERB\>	{
        ;
}


\<[a-zA-Z0-9]+\>	{
	fputs_unlocked("\\begin{", yyout);
	fwrite(yytext+1, 1, yyleng-2, yyout);
	fputs_unlocked("}", yyout);
}

\<[a-zA-Z0-9]+_STAR\>	{
	fputs_unlocked("\\begin{", yyout);
	fwrite(yytext+1, 1, yyleng-7, yyout);
	fputs_unlocked("*}", yyout);
}

\<\/[a-zA-Z0-9]+\>	{
	fputs_unlocked("\\end{", yyout);
	fwrite(yytext+2, 1, yyleng-3, yyout);
	fputs_unlocked("}", yyout);
}

\<\/[a-zA-Z0-9]+_STAR\>	{
	fputs_unlocked("\\end{", yyout);
	fwrite(yytext+2, 1, yyleng-8, yyout);
	fputs_unlocked("*}", yyout);
}

\<[a-zA-Z0-9]+\/\>	{
	fputs_unlocked("\\", yyout);
	fwrite(yytext+1, 1, yyleng-3, yyout);
}

\<[a-zA-Z0-9]+_STAR\/\>	{
  fputc_unlocked('\\', yyout);
  fwrite(yytext+1, 1, yyleng-8, yyout);
  fputc_unlocked('*', yyout);
}

\#	{
        fputs_unlocked("\\#", yyout);
}


(.|\n)	{
	fwrite(yytext, 1, yyleng, yyout);
}

<mathenv>(.|\n)	{
	fwrite(yytext, 1, yyleng, yyout);
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
