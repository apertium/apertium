

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

AccentsMap accentsMap(false);
wstring closesym = L"";
string memconv = "";
//For german babel detection
bool ngermanbabel = false;

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





\\t\{..\}	{ //This information is lost
	fputws(convertir(yytext+3,yyleng-4).c_str(),yyout);
}
\\l	{
	fputws(L"ł", yyout);
}

\"[oOaAuUsS]	{ //When usepackage[ngerman]{babel} is present (not checked).
	if(!ngermanbabel)
		fputws(convertir(yytext,yyleng).c_str(),yyout);
	else {
		switch(yytext[1]){
			case 'o': fputws(L"ö", yyout); break;
			case 'O': fputws(L"Ö", yyout); break;
			case 'a': fputws(L"ä", yyout); break;
			case 'A': fputws(L"Ä", yyout); break;
			case 'u': fputws(L"ü", yyout); break;
			case 'U': fputws(L"Ü", yyout); break;
			case 's': fputws(L"ß", yyout); break;
			case 'S': fputws(L"ß", yyout); break;
		}
	}
}



\\[\^\"\'`]((\{\\[ij]\})|(\\[ij]))	{
	switch(yytext[1]){
	case '^':
		if(yytext[4]=='i')
			fputws(L"î", yyout);
		else
			fputws(L"ĵ",yyout);
		break;
	case '\"':
		if(yytext[4]=='i')
			fputws(L"ï",yyout);
		else
			fputws(L"j",yyout); //should actually be j with umlaut
		break;
	case '\'':
		if(yytext[4]=='i')
			fputws(L"í",yyout);
		else
			fputws(L"j",yyout); //should actually be j with accent
		break;
	case '`':
		if(yytext[4]=='i')
			fputws(L"ì",yyout);
		else
			fputws(L"k",yyout); //should actually be j with accent
		break;
	}
}

\{\\oe\}	{
	fputws(L"œ",yyout);
}

\{\\OE\}	{
	fputws(L"Œ",yyout);
}

\{\\ae\}	{
	fputws(L"æ",yyout);
}

\{\\AE\}	{
	fputws(L"Æ",yyout);
}

\{\\aa\}	{
	fputws(L"å",yyout);
}

\{\\AA\}	{
	fputws(L"Å",yyout);
}

\{\\o\}	{
	fputws(L"ø",yyout);
}

\{\\O\}	{
	fputws(L"Ø",yyout);
}

\{\\ss\}	{
	fputws(L"ß",yyout);
}

\\#[0-9]+	{
        fputws((wstring(L"<HASH_")+convertir(yytext+2,yyleng-2)+wstring(L"/>")).c_str(),yyout);
}

\\#		{
        fputws(L"<HASH/>", yyout);
}

\\[`'\^\"H~ck=b.druv]((\{.\})|(.))	{
	wstring ws = convertir(yytext,yyleng).c_str();
	
	wstring result = accentsMap.get(
	  L""+ws.substr(1,1)+ (
	    (yyleng==3)? ws.substr(2,1) : ws.substr(3,1)
	  ));
	  
        if(result == L"")
        {
	  fputws((wstring(L"<")+convertir(yytext+1,yyleng)+wstring(L"/>")).c_str(),yyout);          
        }
        else
        {
  	  fputws(result.c_str(), yyout);
        }
}

\\\\	{
	fputws(L"<BR/>",yyout);
}

\%.*	{
	if(yytext[yyleng-1]=='\r')
		fputws((wstring(L"<COMMENT>")+convertir(yytext+1,yyleng-2)+wstring(L"</COMMENT>\r")).c_str(),yyout);
	else		
		fputws((wstring(L"<COMMENT>")+convertir(yytext+1,yyleng-1)+wstring(L"</COMMENT>")).c_str(),yyout);
}

\\usepackage\[[^\]]*\]	{
	wstring ws = convertir(yytext+12,yyleng-13);
	fputws((wstring(L"<usepackage/><PARAM>")+ws+wstring(L"</PARAM>")).c_str(), yyout);
	if(ws.find(L"ngerman") >= 0)
		ngermanbabel = true;
}

\[[^\]]*\]	{
	fputws((wstring(L"<PARAM>")+convertir(yytext+1,yyleng-2)+wstring(L"</PARAM>")).c_str(), yyout);
}

\\begin[^a-zA-Z0-9_]	{
	BEGIN(readbrackets);
	closesym = L"";
}

\\end[^a-zA-Z0-9_]	{
	BEGIN(readbrackets);
	closesym = L"/";
}



<readbrackets>[ \n\r\t]*\{?[ \n\r\t]*	{
	wstring ws = convertir(yytext,yyleng);
	int i = ws.find(L'{'); //remove it
	if(i>=0)
		ws = ws.substr(0,i)+ws.substr(i+1);
	fputws(ws.c_str(),yyout);
}

<readbrackets>[a-zA-Z0-9]+\*	{
	fputws((wstring(L"<")+closesym+convertir(yytext,yyleng-1)+wstring(L"_STAR>")).c_str(),yyout);
}

<readbrackets>[a-zA-Z0-9]+	{
	fputws((wstring(L"<")+closesym+convertir(yytext,yyleng)+wstring(L">")).c_str(),yyout);
}

<readbrackets>[ \n\r\t]*\}[ \n\r\t]*	{
	BEGIN(0);
	wstring ws = convertir(yytext,yyleng);
	int i = ws.find(L'}'); //remove it
	if(i>=0)
		ws = ws.substr(0,i)+ws.substr(i+1);
	fputws(ws.c_str(),yyout);
}


\\[A-Za-z]+\*	{ 
	fputws((wstring(L"<")+convertir(yytext+1,yyleng-2)+wstring(L"_STAR/>")).c_str(),yyout);
}

\\[A-Za-z]+	{ 
	fputws((wstring(L"<")+convertir(yytext+1,yyleng)+wstring(L"/>")).c_str(),yyout);
}

\\\{	{
        fputws(L"<LEFTESCAPEDBRACE/>", yyout);
        }
       
\\\{	{
        fputws(L"<RIGHTESCAPEDBRACE/>", yyout);
        }

\\\%	{
        fputws(L"<ESCAPEDPERCENT/>", yyout);
        }

\{	{
	fputws(L"<CONTENTS>",yyout);
}

\}	{
	fputws((wstring(L"</CONTENTS>")).c_str(),yyout);
}

~	{
	fputws(L"&NBSP;",yyout);
}

\$\$	{
	BEGIN(mathenv);
	fputws(L"<MATH_DOLLARS>",yyout);
}

<mathenv>\$\$	{
	fputws(L"</MATH_DOLLARS>",yyout);
	BEGIN(0);
}

\$	{
	BEGIN(mathenv);
	fputws(L"<MATH_DOLLAR>",yyout);
}

<mathenv>\$	{
	fputws(L"</MATH_DOLLAR>",yyout);
	BEGIN(0);
}

\\verb[|][^|]+[|]	{
        fputws(L"<VERB>",yyout);
        wstring ws = convertir(yytext, yyleng);
        fputws(ws.substr(5, ws.size()-5).c_str(), yyout);
        fputws(L"</VERB>", yyout);
}        

\\verb[!][^!]+[!]	{
        fputws(L"<VERB>",yyout);
        wstring ws = convertir(yytext, yyleng);
        fputws(ws.substr(5, ws.size()-5).c_str(), yyout);
        fputws(L"</VERB>", yyout);
}        

\\verb[?][^?]+[?]	{
        fputws(L"<VERB>",yyout);
        wstring ws = convertir(yytext, yyleng);
        fputws(ws.substr(5, ws.size()-5).c_str(), yyout);
        fputws(L"</VERB>", yyout);
}        

\\verb[/][^/]+[/]	{
        fputws(L"<VERB>",yyout);
        wstring ws = convertir(yytext, yyleng);
        fputws(ws.substr(5, ws.size()-5).c_str(), yyout);
        fputws(L"</VERB>", yyout);
}        

\\verb[#][^#]+[#]	{
        fputws(L"<VERB>",yyout);
        wstring ws = convertir(yytext, yyleng);
        fputws(ws.substr(5, ws.size()-5).c_str(), yyout);
        fputws(L"</VERB>", yyout);
}        

\\verb[+][^+]+[+]	{
        fputws(L"<VERB>",yyout);
        wstring ws = convertir(yytext, yyleng);
        fputws(ws.substr(5, ws.size()-5).c_str(), yyout);
        fputws(L"</VERB>", yyout);
}        

\\\(	{
	fputws(L"<MATH_PAR>",yyout);
}

\\\)	{
	fputws(L"</MATH_PAR>",yyout);
}

\\\[	{
	fputws(L"<MATH_BRA>",yyout);
}

\\\]	{
	fputws(L"</MATH_BRA>",yyout);
}

\?`	{
	fputws(L"¿",yyout);
}

!`	{
	fputws(L"¡",yyout);
}

\"	{
	fputws(L"&quot;",yyout);
}
\'	{
	fputws(L"&apos;",yyout);
}
\<	{
	fputws(L"&lt;",yyout);
}
\>	{
	fputws(L"&gt;",yyout);
}
\\\&	{
	fputws(L"&amp;",yyout);
}
\&	{
	fputws(L"<AMP/>",yyout);
}





(.|\n|\r)	{
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
