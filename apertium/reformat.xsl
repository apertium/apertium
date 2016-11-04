<?xml version="1.0" encoding="ISO-8859-1"?> <!-- -*- nxml -*- -->
<!--
 Copyright (C) 2005 Universitat d'Alacant / Universidad de Alicante

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License as
 published by the Free Software Foundation; either version 2 of the
 License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful, but
 WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, see <http://www.gnu.org/licenses/>.
-->
<xsl:stylesheet version="1.0"
                xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
  <xsl:output method="text" encoding="UTF-8"/>

<xsl:template match="format">

%{

#ifndef GENFORMAT
#include "apertium_config.h"
#endif
#include &lt;utf8/utf8.h&gt;
#include &lt;apertium/unlocked_cstdio.h&gt;

#include &lt;cstdlib&gt;
#include &lt;iostream&gt;
#include &lt;libgen.h&gt;
#include &lt;map&gt;
#include &lt;string&gt;
#include &lt;unistd.h&gt;
#include &lt;lttoolbox/lt_locale.h&gt;
#include &lt;lttoolbox/ltstr.h&gt;
#include &lt;apertium/string_to_wostream.h&gt;
#include &lt;wchar.h&gt;
#ifdef _WIN32
#include &lt;io.h&gt;
#include &lt;fcntl.h&gt;
#define utf8to32 utf8to16
#define utf32to8 utf16to8
#endif

using namespace std;

<xsl:for-each select="./rules/replacement-rule">
  <xsl:variable name="varname"
		select="concat(concat(string('S'),position()),string('_substitution'))"/>
  <xsl:value-of select="string('map&lt;wstring, wstring, Ltstr&gt; S')"/>
  <xsl:value-of select="position()"/>
  <xsl:value-of select="string('_substitution;&#xA;&#xA;void S')"/>
  <xsl:value-of select="position()"/>
  <xsl:value-of select="string('_init()&#xA;{')"/>

  <xsl:for-each select="./replace">
    <xsl:if test="./@prefer = string('yes')">
      <xsl:value-of select="string('&#xA;  ')"/>
      <xsl:value-of select="$varname"/>
      <xsl:value-of select="string('[L&quot;')"/>
      <xsl:value-of select="./@target"/>
      <xsl:value-of select="string('&quot;] = L&quot;')"/>
      <xsl:value-of select="./@source"/>
      <xsl:value-of select="string('&quot;;')"/>
    </xsl:if>
  </xsl:for-each>

  <xsl:value-of select="string('&#xA;}&#xA;')"/>
</xsl:for-each>

string memconv;

wstring convertir(char const *multibyte, int const length)
{
  std::wstring rv;
  memconv.append(multibyte, length);
  if (utf8::is_valid(memconv.begin(), memconv.end())) {
  	utf8::utf8to32(memconv.begin(), memconv.end(), std::back_inserter(rv));
  	memconv.clear();
  }
  return rv;
}

%}

%option nounput
%option noyywrap<xsl:if test="./options/case-sensitive/@value=string('no')">
%option caseless</xsl:if>

%%

"["|"]"&#x9;{
  // do nothing
}

"[@"[^]]+"]"&#x9;{
  string filename = yytext;
  filename = filename.substr(2, filename.size()-3);
  FILE *temp = fopen(filename.c_str(), "rb");
  wint_t mychar;
#ifdef _MSC_VER
  _setmode(_fileno(temp), _O_U8TEXT);
#endif

  if(!temp)
  {
    wcerr &lt;&lt; "ERROR: File '" &lt;&lt; filename &lt;&lt;"' not found." &lt;&lt; endl;
    exit(EXIT_FAILURE);
  }
  while(static_cast&lt;int&gt;(mychar = fgetwc_unlocked(temp)) != EOF)
  {
    fputwc_unlocked(mychar, yyout);
  }
  fclose(temp);
  unlink(filename.c_str());
}

"[\\@"&#x9;{
  fputwc_unlocked(L'@', yyout);
}

".[]"&#x9;{
  // do nothing
}

"\\"<xsl:value-of select="/format/options/escape-chars/@regexp"/>&#x9;{
  fputws_unlocked(convertir(yytext+1, yyleng-1).c_str(), yyout);
}



.|\n&#x9;{
  wstring yytext_conv = convertir(yytext, yyleng);
<xsl:for-each select="./rules/replacement-rule">
  <xsl:variable name="varname"
		select="concat(concat(string('S'),position()),string('_substitution'))"/>

  <xsl:value-of select="string('  ')"/>
  <xsl:if test="not(position()=1)">
    <xsl:value-of select="string('else ')"/>
  </xsl:if>
  <xsl:value-of select="string('if(')"/>
  <xsl:value-of select="$varname"/>
  <xsl:value-of select="string('.find(yytext_conv) != ')"/>
  <xsl:value-of select="$varname"/>
  <xsl:value-of select="string('.end())&#xA;  {&#xA;')"/>
  <xsl:value-of select="string('    fputws_unlocked(')"/>
  <xsl:value-of select="$varname"/>
  <xsl:value-of select="string('[yytext_conv].c_str(), yyout);')"/>
  <xsl:value-of select="string('&#xA;  }&#xA;')"/>
</xsl:for-each>

<xsl:if test="not(count(./rules/replacement-rule)=0)">
  <xsl:value-of select="string('  else&#xA;  {&#xA;  ')"/>
</xsl:if>
<xsl:value-of select="string('  fputws_unlocked(yytext_conv.c_str(), yyout);&#xA;')"/>
<xsl:if test="not(count(./rules/replacement-rule)=0)">
  <xsl:value-of select="string('  }')"/>
</xsl:if>
}

&lt;&lt;EOF&gt;&gt;&#x9;{
  return 0;
}

%%

void usage(string const &amp;progname)
{
  wcerr &lt;&lt; "USAGE: " &lt;&lt; progname &lt;&lt; " [input_file [output_file]" &lt;&lt; ']' &lt;&lt; endl;
  wcerr &lt;&lt; "<xsl:value-of select="./@name"/> format processor " &lt;&lt; endl;
  exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[])
{
  LtLocale::tryToSetLocale();

  if(argc &gt; 3)
  {
    usage(argv[0]);
  }

  switch(argc)
  {
    case 3:
      yyout = fopen(argv[2], "wb");
      if(!yyout)
      {
        usage(argv[0]);
      }
    case 2:
      yyin = fopen(argv[1], "rb");
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

<xsl:for-each select="./rules/replacement-rule">
  <xsl:value-of select="string('  S')"/>
  <xsl:value-of select="position()"/>
  <xsl:value-of select="string('_init();&#xA;')"/>
</xsl:for-each>

  yylex();
  fclose(yyin);
  fclose(yyout);
}
</xsl:template>
</xsl:stylesheet>
