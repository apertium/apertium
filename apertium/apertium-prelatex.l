

%{



#include <cstdlib>
#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <apertium/latex_accentsmap.h>
#include <lttoolbox/string_utils.h>
#include <apertium/string_to_wostream.h>

extern "C" {
#if !defined(__STDC__)
# define __STDC__ 1
#endif
#include <regex.h>
}

#include <lttoolbox/lt_locale.h>
#include <apertium/unlocked_cstdio.h>
#ifdef _WIN32
#include <io.h>
#include <fcntl.h>
#endif

using namespace std;

AccentsMap accentsMap(false);
UString closesym;
//For german babel detection
bool ngermanbabel = false;



%}


%option nounput
%option noyywrap
%option stack

%x mathenv
%x readbrackets

%%





\\t\{..\}	{ //This information is lost
    fwrite(yytext+3, 1, yyleng-4, yyout);
}
\\l	{
	fputs("ł", yyout);
}

\"[oOaAuUsS]	{ //When usepackage[ngerman]{babel} is present (not checked).
	if(!ngermanbabel) {
        fputs(yytext, yyout);
    } else {
		switch(yytext[1]){
			case 'o': fputs("ö", yyout); break;
			case 'O': fputs("Ö", yyout); break;
			case 'a': fputs("ä", yyout); break;
			case 'A': fputs("Ä", yyout); break;
			case 'u': fputs("ü", yyout); break;
			case 'U': fputs("Ü", yyout); break;
			case 's': fputs("ß", yyout); break;
			case 'S': fputs("ß", yyout); break;
		}
	}
}



\\[\^\"\'`]((\{\\[ij]\})|(\\[ij]))	{
	switch(yytext[1]){
	case '^':
		if(yytext[4]=='i')
			fputs("î", yyout);
		else
			fputs("ĵ",yyout);
		break;
	case '\"':
		if(yytext[4]=='i')
			fputs("ï",yyout);
		else
			fputs("j",yyout); //should actually be j with umlaut
		break;
	case '\'':
		if(yytext[4]=='i')
			fputs("í",yyout);
		else
			fputs("j",yyout); //should actually be j with accent
		break;
	case '`':
		if(yytext[4]=='i')
			fputs("ì",yyout);
		else
			fputs("k",yyout); //should actually be j with accent
		break;
	}
}

\{\\oe\}	{
	fputs("œ",yyout);
}

\{\\OE\}	{
	fputs("Œ",yyout);
}

\{\\ae\}	{
	fputs("æ",yyout);
}

\{\\AE\}	{
	fputs("Æ",yyout);
}

\{\\aa\}	{
	fputs("å",yyout);
}

\{\\AA\}	{
	fputs("Å",yyout);
}

\{\\o\}	{
	fputs("ø",yyout);
}

\{\\O\}	{
	fputs("Ø",yyout);
}

\{\\ss\}	{
	fputs("ß",yyout);
}

\\#[0-9]+	{
        fputs("<HASH_", yyout);
        fwrite(yytext+2, 1, yyleng-2, yyout);
        fputs("/>", yyout);
}

\\#		{
        fputs("<HASH/>", yyout);
}

\\[`'\^\"H~ck=b.druv]((\{.\})|(.))	{
    UString ws = to_ustring(yytext);
    UString key;
    key += ws[1];
    key += (yyleng == 3) ? ws[2] : ws[3];

	UString result = accentsMap.get(key);
    if (result.empty()) {
      fputc('<', yyout);
      fwrite(yytext+1, 1, yyleng-1, yyout);
      fputs("/>", yyout);
    } else {
      fputus(result, yyout);
    }
}

\\\\	{
	fputs("<BR/>",yyout);
}

\%.*	{
    fputs("<COMMENT>", yyout);
	if (yytext[yyleng-1] == '\r') {
      fwrite(yytext+1, 1, yyleng-2, yyout);
      fputs("</COMMENT>\r", yyout);
	} else {
      fwrite(yytext+1, 1, yyleng-1, yyout);
      fputs("</COMMENT>", yyout);
    }
}

\\usepackage\[[^\]]*\]	{
  fputs("<usepackage/><PARAM>", yyout);
  // this is maybe wrong, but hopefully no one puts non-ASCII
  // characters in their package names
  UString ws = to_ustring(yytext).substr(12, yyleng-13);
  fputus(ws, yyout);
  fputs("</PARAM>", yyout);
  if(ws.find("ngerman"_u) != UString::npos)
    ngermanbabel = true;
}

\[[^\]]*\]	{
  fputs("<PARAM>", yyout);
  fwrite(yytext+1, 1, yyleng-2, yyout);
  fputs("</PARAM>", yyout);
}

\\begin[^a-zA-Z0-9_]	{
	BEGIN(readbrackets);
	closesym = ""_u;
}

\\end[^a-zA-Z0-9_]	{
	BEGIN(readbrackets);
	closesym = "/"_u;
}



<readbrackets>[ \n\r\t]*\{?[ \n\r\t]*	{
  UString ws = to_ustring(yytext);
  int i = ws.find('{');
  if (i >= 0) {
    ws = ws.substr(0, i) + ws.substr(i+1);
  }
  fputus(ws, yyout);
}

<readbrackets>[a-zA-Z0-9]+\*	{
  fputc('<', yyout);
  fputus(closesym, yyout);
  fwrite(yytext+0, 1, yyleng-1, yyout);
  fputs("_STAR>", yyout);
}

<readbrackets>[a-zA-Z0-9]+	{
  fputc('<', yyout);
  fputus(closesym, yyout);
  fputs(yytext, yyout);
  fputc('>', yyout);
}

<readbrackets>[ \n\r\t]*\}[ \n\r\t]*	{
	BEGIN(0);
    UString ws = to_ustring(yytext);
    int i = ws.find('}');
    if (i >= 0) {
      ws = ws.substr(0, i) + ws.substr(i+1);
    }
    fputus(ws, yyout);
}


\\[A-Za-z]+\*	{
  fputc('<', yyout);
  fwrite(yytext+1, 1, yyleng-2, yyout);
  fputs("_STAR/>", yyout);
}

\\[A-Za-z]+	{
  fputc('<', yyout);
  fwrite(yytext+1, 1, yyleng-1, yyout);
  fputs("/>", yyout);
}

\\\{	{
        fputs("<LEFTESCAPEDBRACE/>", yyout);
        }

\\\{	{
        fputs("<RIGHTESCAPEDBRACE/>", yyout);
        }

\\\%	{
        fputs("<ESCAPEDPERCENT/>", yyout);
        }

\{	{
	fputs("<CONTENTS>",yyout);
}

\}	{
	fputs("</CONTENTS>", yyout);
}

~	{
	fputs("&NBSP;",yyout);
}

\$\$	{
	BEGIN(mathenv);
	fputs("<MATH_DOLLARS>",yyout);
}

<mathenv>\$\$	{
	fputs("</MATH_DOLLARS>",yyout);
	BEGIN(0);
}

\$	{
	BEGIN(mathenv);
	fputs("<MATH_DOLLAR>",yyout);
}

<mathenv>\$	{
	fputs("</MATH_DOLLAR>",yyout);
	BEGIN(0);
}

\\verb[|][^|]+[|]	{
        fputs("<VERB>", yyout);
        fwrite(yytext+5, 1, yyleng-5, yyout);
        fputs("</VERB>", yyout);
}

\\verb[!][^!]+[!]	{
        fputs("<VERB>",yyout);
        fwrite(yytext+5, 1, yyleng-5, yyout);
        fputs("</VERB>", yyout);
}

\\verb[?][^?]+[?]	{
        fputs("<VERB>",yyout);
        fwrite(yytext+5, 1, yyleng-5, yyout);
        fputs("</VERB>", yyout);
}

\\verb[/][^/]+[/]	{
        fputs("<VERB>",yyout);
        fwrite(yytext+5, 1, yyleng-5, yyout);
        fputs("</VERB>", yyout);
}

\\verb[#][^#]+[#]	{
        fputs("<VERB>",yyout);
        fwrite(yytext+5, 1, yyleng-5, yyout);
        fputs("</VERB>", yyout);
}

\\verb[+][^+]+[+]	{
        fputs("<VERB>",yyout);
        fwrite(yytext+5, 1, yyleng-5, yyout);
        fputs("</VERB>", yyout);
}

\\\(	{
	fputs("<MATH_PAR>",yyout);
}

\\\)	{
	fputs("</MATH_PAR>",yyout);
}

\\\[	{
	fputs("<MATH_BRA>",yyout);
}

\\\]	{
	fputs("</MATH_BRA>",yyout);
}

\?`	{
	fputs("¿",yyout);
}

!`	{
	fputs("¡",yyout);
}

\"	{
	fputs("&quot;",yyout);
}
\'	{
	fputs("&apos;",yyout);
}
\<	{
	fputs("&lt;",yyout);
}
\>	{
	fputs("&gt;",yyout);
}
\\\&	{
	fputs("&amp;",yyout);
}
\&	{
	fputs("<AMP/>",yyout);
}





(.|\n|\r)	{
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

  cerr << "LaTeX format preprocessor " << endl;
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
