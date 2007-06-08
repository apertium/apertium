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
      <xsl:value-of select="string('  buffer.append(yytext);&#xA;}&#xA;')"/>
    </xsl:when>
    <xsl:otherwise>
      <xsl:variable name="thisnode" select="."/>
      <xsl:value-of select="./begin/@regexp"/>
      <xsl:value-of select="string('&#x9;{&#xA;')"/>
      <xsl:if test="./@eos = string('yes')">
	<xsl:value-of select="string('  isDot = true;&#xA;')"/>
      </xsl:if>
      <xsl:value-of select="string('  buffer.append(yytext);&#xA;  yy_push_state(C')"/>
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
      <xsl:value-of select="string('  current++;&#xA;  orders.push_back(current);&#xA;')"/>
      <xsl:value-of select="string('  last=&quot;open_tag&quot;;&#xA;  offsets.push_back(offset);&#xA;')"/>
      <xsl:value-of select="string('  tags.push_back(yytext);&#xA;}&#xA;')"/>
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
      <xsl:value-of select="string('  buffer.append(yytext);&#xA;  yy_push_state(C')"/>
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
      <xsl:value-of select="string('  buffer.append(yytext);&#xA;}&#xA;')"/>
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>

<xsl:template match="format">

%{

#include &lt;cstdlib&gt;
#include &lt;iostream&gt;
#include &lt;map&gt;
#include &lt;vector&gt;
#include &lt;regex.h&gt;
#include &lt;string&gt;


using namespace std;

string buffer;
bool isDot, hasWrite_dot, hasWrite_white;
FILE *formatfile;
string last;
int current;
long int offset;


vector&lt;long int&gt; offsets;
vector&lt;string&gt; tags;
vector&lt;int&gt; orders;

regex_t escape_chars;
regex_t names_regexp;

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
  string new_str = "";

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

string escape(string const &amp;str)
{
  regmatch_t pmatch;
  
  char const *mystring = str.c_str();
  int base = 0;
  string result = "";
  while(!regexec(&amp;escape_chars, mystring + base, 1, &amp;pmatch, 0))
  {
    result.append(str.substr(base, pmatch.rm_so));
    result += '\\';
    result += str[base + pmatch.rm_so];
    base += pmatch.rm_eo;
  }

  return result + str.substr(base);
}

string get_tagName(string tag){
  regmatch_t pmatch;
  
  char const *mystring = tag.c_str();
  string result = "";
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
  <xsl:value-of select="string('map&lt;string, string&gt; S')"/>
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
    <xsl:value-of select="string('&quot;;')"/>
  </xsl:for-each>

  <xsl:value-of select="string('&#xA;}&#xA;')"/>
</xsl:for-each>

<xsl:if test="$mode=string('matxin')">
int get_index(string end_tag){
  for (int i=tags.size()-1; i >= 0; i--) {
    if (get_tagName(end_tag) == get_tagName(tags[i]))
      return i;
  }
  return -1;
}

void print_emptyTags(){
  char tag[250];

  for (int i=0; i &lt; tags.size(); i++) {
    sprintf(tag, "&lt;format-tag offset=\"%d\" order= \"%d\"&gt;&lt;![CDATA[", offsets[i], orders[i]);
    fputs_unlocked(tag, formatfile);
    fputs_unlocked(tags[i].c_str(), formatfile); 
    fputc_unlocked(']', formatfile);
    sprintf(tag, "]&gt;&lt;/format-tag&gt;\n");
    fputs_unlocked(tag, formatfile);
  }
}
</xsl:if>

<xsl:choose>
  <xsl:when test="$mode=string('matxin')">
void printBuffer(int ind=-1, string end_tag="")
{
  char tag[250];

  if (ind != -1 &amp;&amp; ind == tags.size()-1 &amp;&amp; offsets[ind] == offset) {
    last = "buffer";
    buffer = tags.back() + buffer + end_tag;
    tags.pop_back();
    offsets.pop_back();
    orders.pop_back();
  }
  else if (ind == -1 &amp;&amp; end_tag != "") {
    last = "buffer";
    buffer = buffer + end_tag;    
  }
  else {
    if (hasWrite_dot &amp;&amp; isDot) {
      sprintf(tag, "&lt;empty-tag offset=\"%d\"/&gt;\n", offset);
      fputs_unlocked(tag, formatfile);

      fputs_unlocked(".", yyout);
      offset++;
      hasWrite_dot = false;
    }

    isDot = false;

    if(ind != -1) {
      if (hasWrite_white) {
        fputs_unlocked(" ", yyout);
        offset++;
        hasWrite_white = false;
      }

      sprintf(tag, "&lt;open-close-tag&gt;\n");
      sprintf(tag, "%s&lt;open-tag offset=\"%d\" order=\"%d\"&gt;&lt;![CDATA[", tag, offsets[ind], orders[ind]);
      fputs_unlocked(tag, formatfile);
      fputs_unlocked(tags[ind].c_str(), formatfile);

      current++;

      sprintf(tag, "]&gt;&lt;/open-tag&gt;\n");
      sprintf(tag, "]%s&lt;close-tag offset=\"%d\" order=\"%d\"&gt;&lt;![CDATA[", tag, offset, current);
      fputs_unlocked(tag, formatfile);
      fputs_unlocked(end_tag.c_str(), formatfile);
      sprintf(tag, "]&gt;&lt;/close-tag&gt;\n");
      sprintf(tag, "]%s&lt;/open-close-tag&gt;\n", tag);
      fputs_unlocked(tag, formatfile);

      tags.erase(tags.begin() + ind);
      offsets.erase(offsets.begin() + ind);
      orders.erase(orders.begin() + ind);
    }

    if(buffer.size() &gt; 1) {
      if (hasWrite_white) {
        fputs_unlocked(" ", yyout);
        offset++;
        hasWrite_white = false;
      }

      current++;

      sprintf(tag, "&lt;format-tag offset=\"%d\" order=\"%d\"&gt;&lt;![CDATA[", offset, current);
      fputs_unlocked(tag, formatfile);
      fputs_unlocked(buffer.c_str(), formatfile);
      sprintf(tag, "]&gt;&lt;/format-tag&gt;\n"); 
      fputc_unlocked(']', formatfile);
      fputs_unlocked(tag, formatfile);
    }
    else if(buffer.size() == 1 &amp;&amp; buffer[0] != ' ') {
      if (hasWrite_white) {
        fputs_unlocked(" ", yyout);
        offset++;
        hasWrite_white = false;
      }

      current++;

      sprintf(tag, "&lt;format-tag offset=\"%d\" order=\"%d\"&gt;&lt;![CDATA[", offset, current);
      fputs_unlocked(tag, formatfile);
      fputs_unlocked(buffer.c_str(), formatfile);
      sprintf(tag, "]&gt;&lt;/format-tag&gt;\n");
      fputc_unlocked(']', formatfile);
      fputs_unlocked(tag, formatfile);
    }
    else {
      fputs_unlocked(buffer.c_str(), yyout);
      offset += buffer.size();
      hasWrite_white = false;
    }


    last = "buffer";
    buffer = "";
  }
}
  </xsl:when>
  <xsl:otherwise>
void printBuffer()
{
  if(isDot)
  {
    fputs_unlocked(".[]", yyout);
    isDot = false;
  }
  if(buffer.size() &gt; <xsl:value-of select="/format/options/largeblocks/@size"/>)
  {
    string filename = tmpnam(NULL);
    FILE *largeblock = fopen(filename.c_str(), "w");
    fputs_unlocked(buffer.c_str(), largeblock);
    fclose(largeblock);
    fputc_unlocked('[', yyout);
    fputc_unlocked('@', yyout);
    fputs_unlocked(filename.c_str(), yyout);
    fputc_unlocked(']', yyout);
  }
  else if(buffer.size() &gt; 1)
  {
    fputc_unlocked('[', yyout);
    string const tmp = escape(buffer);
    if(tmp[0] == '@')
    {
      fputc_unlocked('\\', yyout);
    }
    fputs_unlocked(tmp.c_str(), yyout);
    fputc_unlocked(']', yyout);
  }
  else if(buffer.size() == 1 &amp;&amp; buffer[0] != ' ')
  {
    fputc_unlocked('[', yyout);
    string const tmp = escape(buffer);
    if(tmp[0] == '@')
    {
      fputc_unlocked('\\', yyout);
    }
    fputs_unlocked(tmp.c_str(), yyout);

    fputc_unlocked(']', yyout);
  }     
  else
  {
    fputs_unlocked(buffer.c_str(), yyout);
  }

  buffer = "";
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
  buffer.append(yytext);
  yy_pop_state();
}

&#x9;\n|.&#x9;{
  last = "buffer";
  buffer += yytext[0];
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
  <xsl:value-of select="string('.end())&#xA;  {&#xA;    printBuffer();&#xA;    fputs_unlocked(')"/>
  <xsl:value-of select="$varname"/>
  <xsl:value-of select="string('[yytext].c_str(), yyout);&#xA;    offset+=')"/>
  <xsl:value-of select="$varname"/>
  <xsl:value-of select="string('[yytext].size();&#xA;')"/>
  <xsl:value-of select="string('    hasWrite_dot = hasWrite_white = true;&#xA;  }&#xA;  else&#xA;  {&#xA;')"/>
  <xsl:value-of select="string('    last=&quot;buffer&quot;;&#xA;    buffer.append(yytext);&#xA;  }&#xA;}&#xA;')"/>  
</xsl:for-each>

<xsl:value-of select="./options/space-chars/@regexp"/>&#x9;{
  if (last == "open_tag") 
    tags.back() += yytext[0];
  else
    buffer += yytext[0];
    
}

<xsl:value-of select="./options/escape-chars/@regexp"/>&#x9;{
  printBuffer();
  fputc_unlocked('\\', yyout);
  offset++;
  fputc_unlocked(yytext[0], yyout);
  offset++;
  hasWrite_dot = hasWrite_white = true;

}

.&#x9;{
  printBuffer();
  fputc_unlocked(yytext[0], yyout);
  offset++;
  hasWrite_dot = hasWrite_white = true;

}

&lt;&lt;EOF&gt;&gt;&#x9;{
  isDot = true;
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
<xsl:choose>
  <xsl:when test="$mode=string('matxin')">
  if(argc &gt; 4 || argc &lt; 2)
  {
    usage(argv[0]);
  }
 
  switch(argc)
  {
    case 4:
      yyout = fopen(argv[3], "w");
      if(!yyout)
      {
        usage(argv[0]);
      }
    case 3:
      yyin = fopen(argv[2], "r");
      if(!yyin)
      {
        usage(argv[0]);
      }
    case 2:
      formatfile = fopen(argv[1], "w");
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
  fputs_unlocked("&lt;?xml version=\&quot;1.0\&quot; encoding=\&quot;ISO-8859-1\&quot;?>\n", formatfile);
  fputs_unlocked("&lt;format&gt;\n", formatfile);
</xsl:if>

  last = buffer = "";
  isDot = hasWrite_dot = hasWrite_white = false;
  current=0;
  offset = 0;
  init_escape();
  init_tagNames();
  yylex();

<xsl:if test="$mode=string('matxin')">
  print_emptyTags();
  fputs_unlocked("&lt;/format&gt;", formatfile);
  fclose(formatfile);
</xsl:if>
  fclose(yyin);
  fclose(yyout);
}
</xsl:template>
</xsl:stylesheet>
