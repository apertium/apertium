<?xml version="1.0" encoding="UTF-8"?> <!-- -*- nxml -*- -->
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
 along with this program; if not, see <https://www.gnu.org/licenses/>.
-->
<xsl:stylesheet version="1.0"
                xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
  <xsl:output method="text" encoding="UTF-8"/>

<xsl:param name="mode"/>

<xsl:template name="replaceString">
  <xsl:param name="haystack"/>
  <xsl:param name="needle"/>
  <xsl:param name="replacement"/>
  <xsl:choose>
    <xsl:when test="contains($haystack, $needle)">
      <xsl:value-of select="substring-before($haystack, $needle)"/>
      <xsl:value-of select="$replacement"/>
      <xsl:call-template name="replaceString">
	<xsl:with-param name="haystack"
			select="substring-after($haystack, $needle)"/>
	<xsl:with-param name="needle" select="$needle"/>
	<xsl:with-param name="replacement" select="$replacement"/>
      </xsl:call-template>
    </xsl:when>
    <xsl:otherwise>
      <xsl:value-of select="$haystack"/>
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>


<xsl:template name="format-rule-apertium">
  <xsl:choose>
    <xsl:when test="count(./begin) = 0">
      <xsl:value-of select="./tag/@regexp"/>
      <xsl:value-of select="string('&#x9;{&#xA;')"/>
      <xsl:if test="./@eos = string('yes')">
	<xsl:value-of select="string('  isDot = true;&#xA;')"/>
      </xsl:if>
      <xsl:if test="./@eoh = string('yes')">
	<xsl:value-of select="string('  isEoh = true;&#xA;')"/>
      </xsl:if>
      <xsl:value-of select="string('  bufferAppend(buffer, yytext);&#xA;}&#xA;')"/>
    </xsl:when>
    <xsl:otherwise>
      <xsl:variable name="thisnode" select="."/>
      <xsl:value-of select="./begin/@regexp"/>
      <xsl:value-of select="string('&#x9;{&#xA;')"/>
      <xsl:if test="./@eos = string('yes')">
	<xsl:value-of select="string('  isDot = true;&#xA;')"/>
      </xsl:if>
      <xsl:if test="./@eoh = string('yes')">
	<xsl:value-of select="string('  isEoh = true;&#xA;')"/>
      </xsl:if>
      <xsl:value-of select="string('  bufferAppend(buffer, yytext);&#xA;  yy_push_state(C')"/>
      <xsl:for-each select="/format/rules/format-rule/begin ">
	<xsl:if test="./@regexp = $thisnode/begin/@regexp">
          <xsl:value-of select="position()"/>
	</xsl:if>
      </xsl:for-each>
      <xsl:value-of select="string(');&#xA;}&#xA;')"/>
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>

<xsl:template name="format-rule-matxin">
  <xsl:choose>
    <xsl:when test="./@type = 'open'">
      <xsl:value-of select="./tag/@regexp"/>
      <xsl:value-of select="string('&#x9;{&#xA;')"/>
      <xsl:if test="./@eos = string('yes')">
        <xsl:value-of select="string('  isDot = true;&#xA;')"/>
      </xsl:if>
      <xsl:if test="./@eoh = string('yes')">
	<xsl:value-of select="string('  isEoh = true;&#xA;')"/>
      </xsl:if>
      <xsl:value-of select="string('  printBuffer();&#xA;')"/>

      <xsl:value-of select="string('  if (hasWrite_white) {&#xA;    fputs(&quot; &quot;, yyout);&#xA;')"/>
      <xsl:value-of select="string('    offset++;&#xA;    hasWrite_white = false;&#xA;  }&#xA;')"/>

      <xsl:value-of select="string('  current++;&#xA;  orders.push_back(current);&#xA;')"/>
      <xsl:value-of select="string('  last=&quot;open_tag&quot;;&#xA;  offsets.push_back(offset);&#xA;')"/>
      <xsl:value-of select="string('  tags.push_back(to_ustring(yytext));&#xA;}&#xA;')"/>
    </xsl:when>
    <xsl:when test="./@type = 'close'">
      <xsl:value-of select="./tag/@regexp"/>
      <xsl:value-of select="string('&#x9;{&#xA;')"/>
      <xsl:if test="./@eos = string('yes')">
        <xsl:value-of select="string('  isDot = true;&#xA;')"/>
      </xsl:if>
      <xsl:if test="./@eoh = string('yes')">
	<xsl:value-of select="string('  isEoh = true;&#xA;')"/>
      </xsl:if>
      <xsl:value-of select="string('  int ind=get_index(yytext);&#xA;')"/>
      <xsl:value-of select="string('  printBuffer(ind, yytext);&#xA;}&#xA;')"/>
    </xsl:when>
    <xsl:when test="./@type = 'comment'">
      <xsl:variable name="thisnode" select="."/>
      <xsl:value-of select="./begin/@regexp"/>
      <xsl:value-of select="string('&#x9;{&#xA;')"/>
      <xsl:if test="./@eos = string('yes')">
        <xsl:value-of select="string('  isDot = true;&#xA;')"/>
      </xsl:if>
      <xsl:if test="./@eoh = string('yes')">
	<xsl:value-of select="string('  isEoh = true;&#xA;')"/>
      </xsl:if>
      <xsl:value-of select="string('  last = &quot;buffer&quot;;&#xA;')"/>
      <xsl:value-of select="string('  bufferAppend(buffer, yytext);&#xA;  yy_push_state(C')"/>
      <xsl:for-each select="/format/rules/format-rule[@type='comment']">
        <xsl:if test="./begin/@regexp = $thisnode/begin/@regexp">
          <xsl:value-of select="position()"/>
        </xsl:if>
      </xsl:for-each>
      <xsl:value-of select="string(');&#xA;}&#xA;')"/>
    </xsl:when>
    <xsl:otherwise>
      <xsl:value-of select="./tag/@regexp"/>
      <xsl:value-of select="string('&#x9;{&#xA;')"/>
      <xsl:if test="./@eos = string('yes')">
        <xsl:value-of select="string('  isDot = true;&#xA;')"/>
      </xsl:if>
      <xsl:if test="./@eoh = string('yes')">
	<xsl:value-of select="string('  isEoh = true;&#xA;')"/>
      </xsl:if>
      <xsl:value-of select="string('  last = &quot;buffer&quot;;&#xA;')"/>
      <xsl:value-of select="string('  bufferAppend(buffer, yytext);&#xA;}&#xA;')"/>
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>

<xsl:template match="format">

%{

#include &lt;cstdlib&gt;
#include &lt;iostream&gt;
#include &lt;map&gt;
#include &lt;string&gt;
#include &lt;vector&gt;

extern "C" {
#if !defined(__STDC__)
# define __STDC__ 1
#endif
#include &lt;regex.h&gt;
}

#include &lt;lttoolbox/lt_locale.h&gt;
#include &lt;lttoolbox/ustring.h&gt;
#ifndef GENFORMAT
#include "apertium_config.h"
#endif
#include &lt;utf8.h&gt;
#include &lt;apertium/unlocked_cstdio.h&gt;

using namespace std;

UString buffer;
string symbuf;
bool isDot, isEoh, hasWrite_dot, hasWrite_white;
bool eosIncond;
bool noDot;
bool markEoh;
UFILE* formatfile;
string last;
int current;
long int offset;


vector&lt;long int&gt; offsets;
vector&lt;UString&gt; tags;
vector&lt;int&gt; orders;

regex_t escape_chars;
regex_t names_regexp;

void bufferAppend(UString &amp;buf, string const &amp;str)
{
  buf += to_ustring(str.c_str());
}

void put(const UString&amp; str, FILE* f)
{
  string temp;
  utf8::utf16to8(str.begin(), str.end(), std::back_inserter(temp));
  fputs_unlocked(temp.c_str(), f);
}

void init_escape()
{
  if(regcomp(&amp;escape_chars, "<xsl:call-template name="replaceString">
      <xsl:with-param name="haystack"
		      select="/format/options/escape-chars/@regexp"/>
      <xsl:with-param name="needle" select="string('\')"/>
      <xsl:with-param name="replacement" select="string('\\')"/>
    </xsl:call-template>", REG_EXTENDED))
  {
    cerr &lt;&lt; "ERROR: Illegal regular expression for escape characters" &lt;&lt; endl;
    exit(EXIT_FAILURE);
  }
}

void init_tagNames()
{
  if(regcomp(&amp;names_regexp, "<xsl:call-template name="replaceString">
      <xsl:with-param name="haystack"
		      select="/format/options/tag-name/@regexp"/>
      <xsl:with-param name="needle" select="string('\')"/>
      <xsl:with-param name="replacement" select="string('\\')"/>
    </xsl:call-template>", REG_EXTENDED))
  {
    cerr &lt;&lt; "ERROR: Illegal regular expression for tag-names" &lt;&lt; endl;
    exit(EXIT_FAILURE);
  }
}

string backslash(string const &amp;str)
{
  string new_str;

  for(unsigned int i = 0; i &lt; str.size(); i++)
  {
    if(str[i] == '\\')
    {
      new_str += str[i];
    }
    new_str += str[i];
  }

  return new_str;
}


UString escape(string const &amp;str)
{
  regmatch_t pmatch;

  char const *mystring = str.c_str();
  int base = 0;
  UString result;

  while(!regexec(&amp;escape_chars, mystring + base, 1, &amp;pmatch, 0))
  {
    bufferAppend(result, str.substr(base, pmatch.rm_so));
    result += '\\';
    const char *mb = str.c_str() + base + pmatch.rm_so;
    UChar32 micaracter = utf8::next(mb, mb+4);

    result += micaracter;
    base += pmatch.rm_eo;
  }

  bufferAppend(result, str.substr(base));
  return result;
}

UString escape(UString const &amp;str)
{
  string dest;
  utf8::utf16to8(str.begin(), str.end(), std::back_inserter(dest));
  return escape(dest);
}

string get_tagName(string tag){
  regmatch_t pmatch;

  char const *mystring = tag.c_str();
  string result;
  if(!regexec(&amp;names_regexp, mystring, 1, &amp;pmatch, 0))
  {
    result=tag.substr(pmatch.rm_so, pmatch.rm_eo - pmatch.rm_so);
    return result;
  }

  return "";
}


<xsl:for-each select="./rules/replacement-rule">
  <xsl:variable name="varname"
		select="concat(concat(string('S'),position()),string('_substitution'))"/>
  <xsl:value-of select="string('map&lt;string, UString&gt; S')"/>
  <xsl:value-of select="position()"/>
  <xsl:value-of select="string('_substitution;&#xA;&#xA;void S')"/>
  <xsl:value-of select="position()"/>
  <xsl:value-of select="string('_init()&#xA;{')"/>

  <xsl:for-each select="./replace">
    <xsl:value-of select="string('&#xA;  ')"/>
    <xsl:value-of select="$varname"/>
    <xsl:value-of select="string('[&quot;')"/>
    <xsl:value-of select="./@source"/>
    <xsl:value-of select="string('&quot;] = &quot;')"/>
    <xsl:value-of select="./@target"/>
    <xsl:value-of select="string('&quot;_u;')"/>
  </xsl:for-each>

  <xsl:value-of select="string('&#xA;}&#xA;')"/>
</xsl:for-each>

<xsl:if test="$mode=string('matxin')">
int get_index(string end_tag){
  string new_end_tag;
  size_t pos;

  for (int i=tags.size()-1; i >= 0; i--) {
    new_end_tag.clear();
    utf8::utf16to8(tags[i].begin(), tags[i].end(), std::back_inserter(new_end_tag));

    if (get_tagName(end_tag) == get_tagName(new_end_tag))
      return i;
  }

  return -1;
}

void print_emptyTags() {
  for (size_t i=0; i &lt; tags.size(); i++) {
    u_fprintf(formatfile, "&lt;format-tag offset=\"%d\" order= \"%d\"&gt;&lt;![CDATA[%S]&gt;&lt;/format-tag&gt;\n", offsets[i], orders[i], tags[i].c_str());
  }
}
</xsl:if>

<xsl:choose>
  <xsl:when test="$mode=string('matxin')">
void printBuffer(int ind=-1, string end_tag="")
{
  UString etiketa;
  UString wend_tag = to_ustring(end_tag.c_str());
  size_t pos;
  int num;

  if (ind != -1 &amp;&amp; ind == tags.size()-1 &amp;&amp;
      offsets[ind] == offset &amp;&amp; orders[ind] == current)
  {
    last = "buffer";
    buffer = tags.back() + buffer + wend_tag;
    tags.pop_back();
    offsets.pop_back();
    orders.pop_back();
  }
  else if (ind == -1 &amp;&amp; !wend_tag.empty())
  {
    last = "buffer";
    buffer = buffer + wend_tag;
  }
  else
  {
    // isEoh handling TODO matxin format
    if (hasWrite_dot &amp;&amp; isDot)
    {
	  u_fprintf(formatfile, "&lt;empty-tag offset=\"%d\"/&gt;\n", offset+1);

      fputs(" .\n", yyout);
      offset += 2;
      hasWrite_dot = false;
    }

    isDot = false;

    if ((buffer.size() == 1 &amp;&amp; buffer[0] != ' ') || buffer.size() &gt; 1)
    {
      if (hasWrite_white)
      {
        fputs(" ", yyout);
        offset++;
        hasWrite_white = false;
      }

      current++;

	  u_fprintf(formatfile, "&lt;format-tag offset=\"%d\" order=\"%d\"&gt;&lt;![CDATA[", offset, current);
      while ((pos = buffer.find("]]&gt;")) != UString::npos)
        buffer.replace(pos, 3, "\\]\\]\\&gt;"_u);
      write(buffer, formatfile);
	  u_fprintf(formatfile, "]]&gt;&lt;/format-tag&gt;\n");
    }
    else
    {
	  put(buffer, yyout);
      offset += buffer.size();
    }


    if (ind != -1)
    {
      if (hasWrite_white)
      {
	    fputc(' ', yyout);
        offset++;
        hasWrite_white = false;
      }

	  u_fprintf(formatfile, "&lt;open-close-tag&gt;\n");
	  u_fprintf(formatfile, "&lt;open-tag offset=\"%d\"order=\"%d\"&gt;&lt;![CDATA[", offsets[ind], orders[ind]);
      etiketa = tags[ind];
      while ((pos = etiketa.find("]]&gt;"_u)) != UString::npos)
        etiketa.replace(pos, 3, "\\]\\]\\&gt;"_u);
      write(etiketa, formatfile);

      current++;

      u_fprintf(formatfile, "]]&gt;&lt;/open-tag&gt;\n");
      u_fprintf(formatfile, "&lt;close-tag offset=\"%d\" order=\"%d\"&gt;&lt;![CDATA[", offset, current);
      while ((pos = wend_tag.find("]]&gt;"_u)) != UString::npos)
        wend_tag.replace(pos, 3, "\\]\\]\\&gt;"_u);
      write(wend_tag, formatfile);
      u_fprintf(formatfile, "]]&gt;&lt;/close-tag&gt;\n");
      u_fprintf(formatfile, "&lt;/open-close-tag&gt;\n");

      tags.erase(tags.begin() + ind);
      offsets.erase(offsets.begin() + ind);
      orders.erase(orders.begin() + ind);
    }


    last = "buffer";
    buffer.clear();
  }

}
  </xsl:when>
  <xsl:otherwise>

void preDot()
{
  if(eosIncond)
  {
    if(noDot)
    {
      fputs_unlocked("[]", yyout);
    }
    else
    {
      fputs_unlocked(".[]", yyout);
    }
  }
}

void printBuffer()
{
  if(isEoh &amp;&amp; markEoh)
  {
	put(u"[]\u2761", yyout);
    isEoh = false;
  }
  if(isDot &amp;&amp; !eosIncond)
  {
    if(noDot)
    {
      fputs_unlocked("[]", yyout);
    }
    else
    {
      fputs_unlocked(".[]", yyout);
    }
    isDot = false;
  }
  if(buffer.size() &gt; <xsl:value-of select="/format/options/largeblocks/@size"/>)
  {
    string filename = tmpnam(NULL);
    UFILE *largeblock = u_fopen(filename.c_str(), "wb", NULL, NULL);
    write(buffer, largeblock);
    u_fclose(largeblock);
    preDot();
    fputc_unlocked('[', yyout);
    fputc_unlocked('@', yyout);
    fputs_unlocked(filename.c_str(), yyout);
    fputc_unlocked(']', yyout);
  }
  else if(buffer.size() &gt; 1)
  {
    preDot();
    fputc_unlocked('[', yyout);
    UString const tmp = escape(buffer);
    if(tmp[0] == '@')
    {
      fputc_unlocked('\\', yyout);
    }
	put(tmp, yyout);
    fputc_unlocked(']', yyout);
  }
  else if(buffer.size() == 1 &amp;&amp; buffer[0] != ' ')
  {
    preDot();
    fputc_unlocked('[', yyout);
    UString const tmp = escape(buffer);
    if(tmp[0] == '@')
    {
      fputc_unlocked('\\', yyout);
    }
    put(tmp, yyout);

    fputc_unlocked(']', yyout);
  }
  else
  {
    put(buffer, yyout);
  }

  buffer.clear();
}
  </xsl:otherwise>
</xsl:choose>
%}

<xsl:if test="count(./rules/format-rule[@type='comment']) &gt; 1">
<xsl:value-of select="string('%x')"/>
<xsl:for-each select="./rules/format-rule[@type='comment']">
  <xsl:value-of select="string(' C')"/>
  <xsl:value-of select="position()"/>
</xsl:for-each>
</xsl:if>
%option nounput
%option noyywrap<xsl:if test="./options/case-sensitive/@value=string('no')">
%option caseless</xsl:if>
%option stack

%%

<xsl:for-each select="./rules/format-rule[@type='comment']">
  <xsl:variable name="sc"
                select="concat(string('C'), position())"/>
  <xsl:variable name="thisnode" select="."/>
&lt;<xsl:value-of select="$sc"/>&gt;{

<xsl:for-each select="/format/rules/format-rule">
  <xsl:sort select="./@priority" data-type="number" order="ascending"/>
  <xsl:choose>
    <xsl:when test="$thisnode/@priority &gt; ./@priority">
      <xsl:value-of select="string('&#x9;')"/>
      <xsl:choose>
        <xsl:when test="$mode=string('matxin')">
          <xsl:call-template name="format-rule-matxin"/>
        </xsl:when>
        <xsl:otherwise>
          <xsl:call-template name="format-rule-apertium"/>
        </xsl:otherwise>
      </xsl:choose>
    </xsl:when>
    <xsl:otherwise/>
  </xsl:choose>
</xsl:for-each>

<xsl:value-of select="string('&#x9;')"/><xsl:value-of select="./end/@regexp"/>&#x9;{
  last = "buffer";
  bufferAppend(buffer, yytext);
  yy_pop_state();
}

&#x9;\n|.&#x9;{
  last = "buffer";
  bufferAppend(buffer, yytext);
}

}
</xsl:for-each>

<xsl:for-each select="./rules/format-rule">
  <xsl:choose>
    <xsl:when test="$mode=string('matxin')">
      <xsl:call-template name="format-rule-matxin"/>
    </xsl:when>
    <xsl:otherwise>
      <xsl:call-template name="format-rule-apertium"/>
    </xsl:otherwise>
  </xsl:choose>
</xsl:for-each>


<xsl:for-each select="./rules/replacement-rule">
  <xsl:variable name="varname"
		select="concat(concat(string('S'),position()),string('_substitution'))"/>
  <xsl:value-of select="string('&#xA;')"/>
  <xsl:value-of select="./@regexp"/>
  <xsl:value-of select="string('&#x9;{&#xA;  if(')"/>
  <xsl:value-of select="$varname"/>
  <xsl:value-of select="string('.find(yytext) != ')"/>
  <xsl:value-of select="$varname"/>
  <xsl:value-of select="string('.end())&#xA;  {&#xA;    printBuffer();&#xA;    put(')"/>
  <xsl:value-of select="$varname"/>
  <xsl:value-of select="string('[yytext], yyout);&#xA;    offset+=')"/>
  <xsl:value-of select="$varname"/>
  <xsl:value-of select="string('[yytext].size();&#xA;')"/>
  <xsl:value-of select="string('    hasWrite_dot = hasWrite_white = true;&#xA;  }&#xA;  else&#xA;  {&#xA;')"/>
  <xsl:value-of select="string('    last=&quot;buffer&quot;;&#xA;    bufferAppend(buffer, yytext);&#xA;  }&#xA;}&#xA;')"/>
</xsl:for-each>

<xsl:value-of select="./options/space-chars/@regexp"/>&#x9;{
  if (last == "open_tag")
    bufferAppend(tags.back(), yytext);
  else
    bufferAppend(buffer, yytext);

}

<xsl:value-of select="./options/escape-chars/@regexp"/>&#x9;{
  printBuffer();
  fputc_unlocked('\\', yyout);
  offset++;
  const char *mb = yytext;
  UChar32 symbol = utf8::next(mb, mb+4);
  put(UString(1, symbol), yyout);
  offset++;
  hasWrite_dot = hasWrite_white = true;

}

.&#x9;{
  printBuffer();
  symbuf += yytext;

  if (utf8::is_valid(symbuf.begin(), symbuf.end())) {
    const char *mb = symbuf.c_str();
    UChar32 symbol = utf8::next(mb, mb+4);
    symbuf.clear();
	put(UString(1, symbol), yyout);
    offset++;
    hasWrite_dot = hasWrite_white = true;
  }
}

&lt;&lt;EOF&gt;&gt;&#x9;{
  isDot = true;

  preDot();
  printBuffer();
  return 0;
}
%%



void usage(string const &amp;progname)
{
<xsl:choose>
  <xsl:when test="$mode=string('matxin')">
  cerr &lt;&lt; "USAGE: " &lt;&lt; progname &lt;&lt; " format_file [input_file [output_file]" &lt;&lt; ']' &lt;&lt; endl;
  </xsl:when>
  <xsl:otherwise>
  cerr &lt;&lt; "USAGE: " &lt;&lt; progname &lt;&lt; " [ -h | -o | -i | -n ] [input_file [output_file]" &lt;&lt; ']' &lt;&lt; endl;
  </xsl:otherwise>
</xsl:choose>
  cerr &lt;&lt; "<xsl:value-of select="./@name"/> format processor " &lt;&lt; endl;
  exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[])
{
  LtLocale::tryToSetLocale();
  int base = 0;
  eosIncond = false;

  if(argc &gt;= 2)
  {
    while(1+base &lt; argc)
    {
      if(!strcmp(argv[1+base],"-i"))
      {
        eosIncond = true;
        base++;
      }
      else if(!strcmp(argv[1+base],"-n"))
      {
        noDot = true;
        base++;
      }
      else if(!strcmp(argv[1+base],"-o"))
      {
        markEoh = true;
        base++;
      }
      else
      {
        break;
      }
    }
  }
<xsl:choose>
  <xsl:when test="$mode=string('matxin')">
  if(argc &gt; 4 || argc &lt; 2)
  {
    usage(argv[0]);
  }

  switch(argc-base)
  {
    case 4:
      yyout = fopen(argv[3+base], "wb");
      if(!yyout)
      {
        usage(argv[0]);
      }
    case 3:
      yyin = fopen(argv[2+base], "rb");
      if(!yyin)
      {
        usage(argv[0]);
      }
    case 2:
      formatfile = u_fopen(argv[1+base], "wb", NULL, NULL);
      if(!formatfile)
      {
        usage(argv[0]);
      }
      break;
    default:
      break;
  }
  </xsl:when>
  <xsl:otherwise>
  if((argc-base) &gt; 4) {
    usage(argv[0]);
  }
  if ((argc - base) == 3) {
    yyout = fopen(argv[2 + base], "wb");
	if (!yyout) {
	  usage(argv[0]);
	}
  }
  if ((argc - base) >= 2) {
    yyin = fopen(argv[1 + base], "rb");
    if (!yyin) {
      usage(argv[0]);
    }
  }
  </xsl:otherwise>
</xsl:choose>
  // prevent warning message
  yy_push_state(1);
  yy_top_state();
  yy_pop_state();

<xsl:for-each select="./rules/replacement-rule">
  <xsl:value-of select="string('  S')"/>
  <xsl:value-of select="position()"/>
  <xsl:value-of select="string('_init();&#xA;')"/>
</xsl:for-each>

<xsl:if test="$mode=string('matxin')">
  write("&lt;?xml version=\&quot;1.0\&quot; encoding=\&quot;UTF-8\&quot; ?>\n"_u, formatfile);
  write("&lt;format&gt;\n"_u, formatfile);
</xsl:if>

  last.clear();
  buffer.clear();
  isEoh = isDot = hasWrite_dot = hasWrite_white = false;
  current=0;
  offset = 0;
  init_escape();
  init_tagNames();
  yylex();

<xsl:if test="$mode=string('matxin')">
  print_emptyTags();
  write("&lt;/format&gt;"_u, formatfile);
  fclose(formatfile);
</xsl:if>
  fclose(yyin);
  fclose(yyout);
}
</xsl:template>
</xsl:stylesheet>
