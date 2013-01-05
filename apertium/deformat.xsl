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
      <xsl:value-of select="string('  bufferAppend(buffer, yytext);&#xA;}&#xA;')"/>
    </xsl:when>
    <xsl:otherwise>
      <xsl:variable name="thisnode" select="."/>
      <xsl:value-of select="./begin/@regexp"/>
      <xsl:value-of select="string('&#x9;{&#xA;')"/>
      <xsl:if test="./@eos = string('yes')">
	<xsl:value-of select="string('  isDot = true;&#xA;')"/>
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
      <xsl:value-of select="string('  printBuffer();&#xA;')"/>

      <xsl:value-of select="string('  if (hasWrite_white) {&#xA;    fputws(L&quot; &quot;, yyout);&#xA;')"/>
      <xsl:value-of select="string('    offset++;&#xA;    hasWrite_white = false;&#xA;  }&#xA;')"/>

      <xsl:value-of select="string('  current++;&#xA;  orders.push_back(current);&#xA;')"/>
      <xsl:value-of select="string('  last=&quot;open_tag&quot;;&#xA;  offsets.push_back(offset);&#xA;')"/>
      <xsl:value-of select="string('  wchar_t* symbol = new wchar_t[strlen(yytext) + 1];&#xA;')"/>
      <xsl:value-of select="string('  mbstowcs(symbol, yytext, strlen(yytext));&#xA;')"/>
      <xsl:value-of select="string('  symbol[strlen(yytext)] = (char) 0;&#xA;')"/>
      <xsl:value-of select="string('  tags.push_back(symbol);&#xA;  delete[] symbol;&#xA;}&#xA;')"/>
    </xsl:when>
    <xsl:when test="./@type = 'close'">
      <xsl:value-of select="./tag/@regexp"/>
      <xsl:value-of select="string('&#x9;{&#xA;')"/>
      <xsl:if test="./@eos = string('yes')">
        <xsl:value-of select="string('  isDot = true;&#xA;')"/>
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

#include &lt;string&gt;
#include &lt;lttoolbox/lt_locale.h&gt;
#include &lt;lttoolbox/ltstr.h&gt;
#ifndef GENFORMAT
#include "apertium_config.h"
#endif
#include &lt;apertium/unlocked_cstdio.h&gt;
#ifdef _MSC_VER
#include &lt;io.h&gt;
#include &lt;fcntl.h&gt;
#endif

using namespace std;

wstring buffer;
string symbuf;
bool isDot, hasWrite_dot, hasWrite_white;
bool eosIncond;
FILE *formatfile;
string last;
int current;
long int offset;


vector&lt;long int&gt; offsets;
vector&lt;wstring&gt; tags;
vector&lt;int&gt; orders;

regex_t escape_chars;
regex_t names_regexp;

void bufferAppend(wstring &amp;buf, string const &amp;str)
{
  symbuf.append(str);

  for(size_t i = 0, limit = symbuf.size(); i &lt; limit;)
  {
    wchar_t symbol;
    int gap = mbtowc(&amp;symbol, symbuf.c_str() + i, MB_CUR_MAX);
    if(gap == -1)
    {
      if(i + MB_CUR_MAX &lt; limit)
      {
        buf += L'?';
        gap = 1;
      }
      else
      { 
        symbuf = symbuf.substr(i);
        return;
      }
    }
    else 
    { 
      buf += symbol;
    }

    i += gap;
  }

  symbuf = "";
  return;
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


wstring escape(string const &amp;str)
{
  regmatch_t pmatch;
  
  char const *mystring = str.c_str();
  int base = 0;
  wstring result;
  
  while(!regexec(&amp;escape_chars, mystring + base, 1, &amp;pmatch, 0))
  {
    bufferAppend(result, str.substr(base, pmatch.rm_so));
    result += L'\\';
    wchar_t micaracter;
    int pos = mbtowc(&amp;micaracter, str.c_str() + base + pmatch.rm_so, MB_CUR_MAX);
    if(pos == -1)
    {
      wcerr &lt;&lt; L"Uno" &lt;&lt; endl;
      wcerr &lt;&lt; L"Encoding error." &lt;&lt; endl;
      exit(EXIT_FAILURE);      
    }
    
    result += micaracter;
    base += pmatch.rm_eo;
  }

  bufferAppend(result, str.substr(base));
  return result;
}

wstring escape(wstring const &amp;str)
{
  string dest;
  
  for(size_t i = 0, limit = str.size(); i &lt; limit; i++)
  {
#ifdef __GNUC__
    char symbol[MB_CUR_MAX+1];
#else 
    std::string _symbol(MB_CUR_MAX+1, 0);
    char *symbol = &amp;_symbol[0];
#endif
    int pos = wctomb(symbol, str[i]);
    if(pos == -1)
    {
      symbol[0]='?';
      pos = 1;
    }
    symbol[pos] = 0;
    dest.append(symbol);
  }
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
  <xsl:value-of select="string('map&lt;string, wstring, Ltstr&gt; S')"/>
  <xsl:value-of select="position()"/>
  <xsl:value-of select="string('_substitution;&#xA;&#xA;void S')"/>
  <xsl:value-of select="position()"/>
  <xsl:value-of select="string('_init()&#xA;{')"/>

  <xsl:for-each select="./replace">
    <xsl:value-of select="string('&#xA;  ')"/>
    <xsl:value-of select="$varname"/>
    <xsl:value-of select="string('[&quot;')"/>
    <xsl:value-of select="./@source"/>
    <xsl:value-of select="string('&quot;] = L&quot;')"/>
    <xsl:value-of select="./@target"/>
    <xsl:value-of select="string('&quot;;')"/>
  </xsl:for-each>

  <xsl:value-of select="string('&#xA;}&#xA;')"/>
</xsl:for-each>

<xsl:if test="$mode=string('matxin')">
int get_index(string end_tag){
  string new_end_tag;
  size_t pos;

  for (int i=tags.size()-1; i >= 0; i--) {
    // a wchar to char conversion can be up to 4 times larger
    char *tmp = new char (sizeof(char)*((tags[i].size()+1) * 4));
    // Keep the existing memset. Better safe than sorry.
    memset(tmp, '\0', tags[i].size() + 1);

    pos = wcstombs(tmp, tags[i].c_str(), tags[i].size());
    if (pos == (size_t)-1)
    {
      wcerr &lt;&lt; L"Encoding error." &lt;&lt; endl;
      exit(EXIT_FAILURE);
    }
    new_end_tag = tmp;
    delete[] tmp;

    if (get_tagName(end_tag) == get_tagName(new_end_tag))
      return i;
  }

  return -1;
}

void print_emptyTags() {
  wchar_t tag[250];

  for (size_t i=0; i &lt; tags.size(); i++) {
    swprintf(tag, 250, L"&lt;format-tag offset=\"%d\" order= \"%d\"&gt;&lt;![CDATA[", offsets[i], orders[i]);
    fputws(tag, formatfile);
    fputws(tags[i].c_str(), formatfile);
    fputwc(L']', formatfile);
    swprintf(tag, 250, L"]&gt;&lt;/format-tag&gt;\n");
    fputws(tag, formatfile);
  }
}
</xsl:if>

<xsl:choose>
  <xsl:when test="$mode=string('matxin')">
void printBuffer(int ind=-1, string end_tag="")
{
  wchar_t tag[250];
  wstring etiketa;
  wstring wend_tag;
  size_t pos;
  int num;
  wchar_t result[end_tag.size() + 1];

  // Convert end_tag to wstring
  pos = mbstowcs(result, end_tag.c_str(), end_tag.size());
  if (pos == (size_t) -1)
  {
    wcerr &lt;&lt; L"Encoding error." &lt;&lt; endl;
    exit(EXIT_FAILURE);
  }
  result[pos] = L'\0';
  wend_tag = result;

  if (ind != -1 &amp;&amp; ind == tags.size()-1 &amp;&amp;
      offsets[ind] == offset &amp;&amp; orders[ind] == current)
  {
    last = "buffer";
    buffer = tags.back() + buffer + wend_tag;
    tags.pop_back();
    offsets.pop_back();
    orders.pop_back();
  }
  else if (ind == -1 &amp;&amp; wend_tag != L"")
  {
    last = "buffer";
    buffer = buffer + wend_tag;
  }
  else
  {
    if (hasWrite_dot &amp;&amp; isDot)
    {
      swprintf(tag, 250, L"&lt;empty-tag offset=\"%d\"/&gt;\n", offset+1);
      fputws(tag, formatfile);

      fputws(L" .\n", yyout);
      offset += 2;
      hasWrite_dot = false;
    }

    isDot = false;

    if ((buffer.size() == 1 &amp;&amp; buffer[0] != ' ') || buffer.size() &gt; 1)
    {
      if (hasWrite_white)
      {
        fputws(L" ", yyout);
        offset++;
        hasWrite_white = false;
      }

      current++;

      swprintf(tag, 250, L"&lt;format-tag offset=\"%d\" order=\"%d\"&gt;&lt;![CDATA[", offset, current);
      fputws(tag, formatfile);
      while ((pos = buffer.find(L"]]&gt;")) != wstring::npos)
        buffer.replace(pos, 3, L"\\]\\]\\&gt;");
      fputws(buffer.c_str(), formatfile);
      swprintf(tag, 250, L"]]&gt;&lt;/format-tag&gt;\n");
      fputws(tag, formatfile);
    }
    else
    {
      fputws(buffer.c_str(), yyout);
      offset += buffer.size();
    }


    if (ind != -1)
    {
      if (hasWrite_white)
      {
        fputws(L" ", yyout);
        offset++;
        hasWrite_white = false;
      }

      num = swprintf(tag, 250, L"&lt;open-close-tag&gt;\n");
      swprintf(tag + num, 250 - num, L"&lt;open-tag offset=\"%d\" order=\"%d\"&gt;&lt;![CDATA[", offsets[ind], orders[ind]);
      fputws(tag, formatfile);
      etiketa = tags[ind];
      while ((pos = etiketa.find(L"]]&gt;")) != wstring::npos)
        etiketa.replace(pos, 3, L"\\]\\]\\&gt;");
      fputws(etiketa.c_str(), formatfile);

      current++;

      num = swprintf(tag, 250, L"]]&gt;&lt;/open-tag&gt;\n");
      swprintf(tag + num, 250 - num, L"&lt;close-tag offset=\"%d\" order=\"%d\"&gt;&lt;![CDATA[", offset, current);
      fputws(tag, formatfile);
      while ((pos = wend_tag.find(L"]]&gt;")) != wstring::npos)
        wend_tag.replace(pos, 3, L"\\]\\]\\&gt;");
      fputws(wend_tag.c_str(), formatfile);
      num = swprintf(tag, 250, L"]]&gt;&lt;/close-tag&gt;\n");
      swprintf(tag + num, 250 - num, L"&lt;/open-close-tag&gt;\n");
      fputws(tag, formatfile);

      tags.erase(tags.begin() + ind);
      offsets.erase(offsets.begin() + ind);
      orders.erase(orders.begin() + ind);
    }


    last = "buffer";
    buffer = L"";
  }

}
  </xsl:when>
  <xsl:otherwise>

void preDot()
{
  if(eosIncond)
  {
    fputws_unlocked(L".[]", yyout);
  }
}

void printBuffer()
{
  if(isDot &amp;&amp; !eosIncond)
  {
    fputws_unlocked(L".[]", yyout);
    isDot = false;
  }
  if(buffer.size() &gt; <xsl:value-of select="/format/options/largeblocks/@size"/>)
  {
    string filename = tmpnam(NULL);
    FILE *largeblock = fopen(filename.c_str(), "w");
    fputws_unlocked(buffer.c_str(), largeblock);
    fclose(largeblock);
    preDot();
    fputwc_unlocked(L'[', yyout);
    fputwc_unlocked(L'@', yyout);
    wchar_t cad[filename.size()];
    size_t pos = mbstowcs(cad, filename.c_str(), filename.size());
    if(pos == (size_t) -1)
    {
      wcerr &lt;&lt; L"Tres" &lt;&lt; endl;

      wcerr &lt;&lt; L"Encoding error." &lt;&lt; endl;
      exit(EXIT_FAILURE);
    }
    cad[pos] = 0;
    fputws_unlocked(cad, yyout);
    fputwc_unlocked(L']', yyout);
  }
  else if(buffer.size() &gt; 1)
  {
    preDot();
    fputwc_unlocked(L'[', yyout);
    wstring const tmp = escape(buffer);
    if(tmp[0] == L'@')
    {
      fputwc_unlocked(L'\\', yyout);
    }
    fputws_unlocked(tmp.c_str(), yyout);
    fputwc_unlocked(L']', yyout);
  }
  else if(buffer.size() == 1 &amp;&amp; buffer[0] != L' ')
  {
    preDot();
    fputwc_unlocked(L'[', yyout);
    wstring const tmp = escape(buffer);
    if(tmp[0] == L'@')
    {
      fputwc_unlocked(L'\\', yyout);
    }
    fputws_unlocked(tmp.c_str(), yyout);

    fputwc_unlocked(L']', yyout);
  }     
  else
  {
    fputws_unlocked(buffer.c_str(), yyout);
  }

  buffer = L"";
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
  <xsl:value-of select="string('.end())&#xA;  {&#xA;    printBuffer();&#xA;    fputws_unlocked(')"/>
  <xsl:value-of select="$varname"/>
  <xsl:value-of select="string('[yytext].c_str(), yyout);&#xA;    offset+=')"/>
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
  fputwc_unlocked(L'\\', yyout);
  offset++;
  wchar_t symbol;
  int pos = mbtowc(&amp;symbol, yytext, MB_CUR_MAX);
  if(pos == -1)
  {
      wcerr &lt;&lt; L"Cuatro" &lt;&lt; endl;

    wcerr &lt;&lt; L"Encoding error." &lt;&lt; endl;
    exit(EXIT_FAILURE);
  }

  fputwc_unlocked(symbol, yyout);
  offset++;
  hasWrite_dot = hasWrite_white = true;

}

.&#x9;{
  printBuffer();
  symbuf += yytext;
  wchar_t symbol;
  int pos = mbtowc(&amp;symbol, symbuf.c_str(), MB_CUR_MAX);
  if(pos == -1)
  {
    if(symbuf.size() > (size_t) MB_CUR_MAX)
    {
      // unknown character
      symbuf = "";
      fputwc_unlocked(L'?', yyout);
      offset++;
      hasWrite_dot = hasWrite_white = true;
    }
  }
  else
  {
    symbuf = "";
    fputwc_unlocked(symbol, yyout);
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
  cerr &lt;&lt; "USAGE: " &lt;&lt; progname &lt;&lt; " [input_file [output_file]" &lt;&lt; ']' &lt;&lt; endl;
  </xsl:otherwise>
</xsl:choose>
  cerr &lt;&lt; "<xsl:value-of select="./@name"/> format processor " &lt;&lt; endl;
  exit(EXIT_SUCCESS);  
}

int main(int argc, char *argv[])
{
  LtLocale::tryToSetLocale();
  size_t base = 0;
  eosIncond = false;
  
  if(argc &gt;= 2 &amp;&amp; !strcmp(argv[1],"-i"))
  {
    eosIncond = true;
    base++;
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
      yyout = fopen(argv[3+base], "w");
      if(!yyout)
      {
        usage(argv[0]);
      }
    case 3:
      yyin = fopen(argv[2+base], "r");
      if(!yyin)
      {
        usage(argv[0]);
      }
    case 2:
      formatfile = fopen(argv[1+base], "w");
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
 if((argc-base) &gt; 4)
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
  </xsl:otherwise>
</xsl:choose>
#ifdef _MSC_VER
  _setmode(_fileno(yyin), _O_U8TEXT);
  _setmode(_fileno(yyout), _O_U8TEXT);
#endif 
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
  fputws(L"&lt;?xml version=\&quot;1.0\&quot; encoding=\&quot;UTF-8\&quot; ?>\n", formatfile);
  fputws(L"&lt;format&gt;\n", formatfile);
</xsl:if>

  last = "";
  buffer = L"";
  isDot = hasWrite_dot = hasWrite_white = false;
  current=0;
  offset = 0;
  init_escape();
  init_tagNames();
  yylex();

<xsl:if test="$mode=string('matxin')">
  print_emptyTags();
  fputws(L"&lt;/format&gt;", formatfile);
  fclose(formatfile);
</xsl:if>
  fclose(yyin);
  fclose(yyout);
}
</xsl:template>
</xsl:stylesheet>
