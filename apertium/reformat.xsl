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
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 02111-1307, USA.
-->
<xsl:stylesheet version="1.0" 
                xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
  <xsl:output method="text" encoding="ISO-8859-1"/>

<xsl:template match="format">

%{

#include &lt;apertium/apertium_config.h&gt;

#if !HAVE_DECL_FPUTS_UNLOCKED
#define fputs_unlocked fputs
#endif

#if !HAVE_DECL_FPUTC_UNLOCKED
#define fputc_unlocked fputc
#endif

#include &lt;cstdlib&gt;
#include &lt;iostream&gt;
#include &lt;libgen.h&gt;
#include &lt;map&gt;
#include &lt;string&gt;
#include &lt;unistd.h&gt;


using namespace std;

<xsl:for-each select="./rules/replacement-rule">
  <xsl:variable name="varname" 
		select="concat(concat(string('S'),position()),string('_substitution'))"/>
  <xsl:value-of select="string('map&lt;string, string&gt; S')"/>
  <xsl:value-of select="position()"/>
  <xsl:value-of select="string('_substitution;&#xA;&#xA;void S')"/>
  <xsl:value-of select="position()"/>
  <xsl:value-of select="string('_init()&#xA;{')"/>

  <xsl:for-each select="./replace">
    <xsl:if test="./@prefer = string('yes')">
      <xsl:value-of select="string('&#xA;  ')"/>
      <xsl:value-of select="$varname"/>
      <xsl:value-of select="string('[&quot;')"/>
      <xsl:value-of select="./@target"/>
      <xsl:value-of select="string('&quot;] = &quot;')"/>
      <xsl:value-of select="./@source"/>
      <xsl:value-of select="string('&quot;;')"/>
    </xsl:if>
  </xsl:for-each>

  <xsl:value-of select="string('&#xA;}&#xA;')"/>
</xsl:for-each>

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
  FILE *temp = fopen(filename.c_str(), "r");
  int mychar;

  if(!temp)
  {
    cerr &lt;&lt; "ERROR: File '" &lt;&lt; filename &lt;&lt;"' not found." &lt;&lt; endl;
    exit(EXIT_FAILURE);
  }
  while((mychar = fgetc_unlocked(temp)) != EOF)
  {
    fputc_unlocked(mychar, yyout);
  }
  fclose(temp);
  unlink(filename.c_str());
}

"[\\@"&#x9;{
  fputc_unlocked('@', yyout);
}

".[]"&#x9;{
  // do nothing
}

"\\"<xsl:value-of select="/format/options/escape-chars/@regexp"/>&#x9;{
  fputc_unlocked(yytext[1], yyout);
}

 

.|\n&#x9;{
<xsl:for-each select="./rules/replacement-rule">
  <xsl:variable name="varname" 
		select="concat(concat(string('S'),position()),string('_substitution'))"/>

  <xsl:value-of select="string('  ')"/>
  <xsl:if test="not(position()=1)">
    <xsl:value-of select="string('else ')"/>
  </xsl:if>
  <xsl:value-of select="string('if(')"/>
  <xsl:value-of select="$varname"/>
  <xsl:value-of select="string('.find(yytext) != ')"/>
  <xsl:value-of select="$varname"/>
  <xsl:value-of select="string('.end())&#xA;  {&#xA;')"/>
  <xsl:value-of select="string('    fputs_unlocked(')"/>
  <xsl:value-of select="$varname"/>
  <xsl:value-of select="string('[yytext].c_str(), yyout);')"/>
  <xsl:value-of select="string('&#xA;  }&#xA;')"/>
</xsl:for-each>   

<xsl:if test="not(count(./rules/replacement-rule)=0)">
  <xsl:value-of select="string('  else&#xA;  {&#xA;  ')"/>
</xsl:if>
<xsl:value-of select="string('  fputc_unlocked(yytext[0], yyout);&#xA;')"/>
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
  cerr &lt;&lt; "USAGE: " &lt;&lt; progname &lt;&lt; " [input_file [output_file]" &lt;&lt; ']' &lt;&lt; endl;
  cerr &lt;&lt; "<xsl:value-of select="./@name"/> format processor " &lt;&lt; endl;
  exit(EXIT_SUCCESS);  
}

int main(int argc, char *argv[])
{
  if(argc &gt; 3)
  {
    usage(argv[0]);
  }
 
  switch(argc)
  {
    case 3:
      yyout = fopen(argv[2], "w");
      if(!yyout)
      {
        usage(argv[0]);
      }
    case 2:
      yyin = fopen(argv[1], "r");
      if(!yyin)
      {
        usage(argv[0]);
      }
      break;
    default:
      break;
  }

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
