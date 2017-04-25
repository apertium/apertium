<?xml version="1.0" encoding="UTF-8"?> <!-- -*- nxml -*- -->
<!--
Copyright (C) 2016 Universitat d'Alacant / Universidad de Alicante

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
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
  <xsl:output method="xml" encoding="UTF-8"/>

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

  <xsl:template match="mode[not(@gendebug='yes')]">
    <!-- Output these unchanged; don't apply templates here -->
    <xsl:copy-of select="."/>
  </xsl:template>

  <xsl:template match="mode[@gendebug='yes']">
    <xsl:comment>
      <xsl:text> **************** </xsl:text>
      <xsl:value-of select="./@name"/>
      <xsl:text>: **************** </xsl:text>
    </xsl:comment>
    <xsl:copy-of select="."/>
    <xsl:apply-templates select="./pipeline/program"/>
  </xsl:template>

  <xsl:template name="debugSuffix">
    <xsl:param name="progname"/>
    <xsl:param name="suff-attr"/>
    <xsl:variable name="p" select="normalize-space($progname)"/>
    <!-- TODO: We also need to know what names have been used already
         to make them unique! Might be easier to do uniquifying
         outside XSLT -->
    <xsl:choose>
      <xsl:when test="$suff-attr != ''">
        <xsl:text>-</xsl:text><xsl:value-of select="$suff-attr"/>
      </xsl:when>
      <xsl:when test="starts-with($p, 'cg-proc')">
        <xsl:text>-disam</xsl:text>
      </xsl:when>
      <xsl:when test="starts-with($p, 'apertium-tagger')">
        <xsl:text>-tagger</xsl:text>
      </xsl:when>
      <xsl:when test="starts-with($p, 'apertium-pretransfer')">
        <xsl:text>-pretransfer</xsl:text>
      </xsl:when>
      <xsl:when test="starts-with($p, 'lrx-proc')">
        <xsl:text>-lex</xsl:text>
      </xsl:when>
      <xsl:when test="starts-with($p, 'apertium-transfer')">
        <xsl:text>-chunker</xsl:text>
      </xsl:when>
      <xsl:when test="starts-with($p, 'apertium-interchunk')">
        <xsl:text>-interchunk</xsl:text>
      </xsl:when>
      <xsl:when test="starts-with($p, 'apertium-postchunk')">
        <xsl:text>-postchunk</xsl:text>
      </xsl:when>
      <xsl:when test="contains($p, '$1')">
        <xsl:text>-dgen</xsl:text>
      </xsl:when>
      <xsl:when test="starts-with($p, 'lt-proc') and contains($p, ' -b')">
        <xsl:text>-biltrans</xsl:text>
      </xsl:when>
      <xsl:when test="starts-with($p, 'lt-proc') and contains($p, ' -p')">
        <xsl:text>-pgen</xsl:text>
      </xsl:when>
      <xsl:when test="starts-with($p, 'lt-proc')">
        <xsl:text>-morph</xsl:text>
      </xsl:when>
      <xsl:when test="starts-with($p, 'hfst-proc')">
        <xsl:text>-morph</xsl:text>
      </xsl:when>
      <xsl:otherwise>
        <xsl:text>-NAMEME</xsl:text>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <xsl:template name="traceOpt">
    <xsl:param name="progname"/>
    <xsl:variable name="p" select="normalize-space($progname)"/>
    <xsl:choose>
      <xsl:when test="starts-with($p, 'cg-proc')">
        <xsl:text> -t</xsl:text>
      </xsl:when>
      <xsl:when test="starts-with($p, 'lrx-proc')">
        <xsl:text> -t</xsl:text>
      </xsl:when>
      <xsl:when test="starts-with($p, 'apertium-transfer')">
        <xsl:text> -t</xsl:text>
      </xsl:when>
      <xsl:when test="starts-with($p, 'apertium-interchunk')">
        <xsl:text> -t</xsl:text>
      </xsl:when>
      <xsl:when test="starts-with($p, 'apertium-postchunk')">
        <xsl:text> -t</xsl:text>
      </xsl:when>
      <xsl:otherwise>
        <xsl:text></xsl:text>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <xsl:template match="program">
    <!-- Regular debug modes: -->
    <mode install="no">
      <xsl:attribute name="name">
        <xsl:value-of select="../../@name"/>
        <xsl:call-template name="debugSuffix">
          <xsl:with-param name="progname" select="./@name"/>
          <xsl:with-param name="suff-attr" select="./@debug-suff"/>
        </xsl:call-template>
      </xsl:attribute>
      <pipeline>
        <xsl:copy-of select="./preceding-sibling::*"/>
        <program>
          <xsl:attribute name="name">
            <xsl:call-template name="replaceString">
              <xsl:with-param name="haystack" select="./@name"/>
              <xsl:with-param name="needle" select="'$1'"/>
              <xsl:with-param name="replacement" select="'-d'"/>
            </xsl:call-template>
            <xsl:call-template name="traceOpt">
              <xsl:with-param name="progname" select="./@name"/>
            </xsl:call-template>
          </xsl:attribute>
          <xsl:copy-of select="./*"/>
        </program>
      </pipeline>
    </mode>
    <!-- Untrimmed debug modes, prefixed @: -->
    <mode install="no">
      <xsl:attribute name="name">
        <xsl:text>@</xsl:text>
        <xsl:value-of select="../../@name"/>
        <xsl:call-template name="debugSuffix">
          <xsl:with-param name="progname" select="./@name"/>
          <xsl:with-param name="suff-attr" select="./@debug-suff"/>
        </xsl:call-template>
      </xsl:attribute>
      <pipeline>
        <xsl:apply-templates select="./preceding-sibling::*" mode="untrim"/>
        <program>
          <xsl:attribute name="name">
            <xsl:call-template name="replaceString">
              <xsl:with-param name="haystack" select="./@name"/>
              <xsl:with-param name="needle" select="'$1'"/>
              <xsl:with-param name="replacement" select="'-d'"/>
            </xsl:call-template>
            <xsl:call-template name="traceOpt">
              <xsl:with-param name="progname" select="./@name"/>
            </xsl:call-template>
          </xsl:attribute>
          <xsl:apply-templates select="./*" mode="untrim"/>
        </program>
      </pipeline>
    </mode>
  </xsl:template>

  <!-- For untrimmed modes, we assume they have the name
       LANG1-LANG2.automorf-untrimmed.bin corresponding to
       LANG1-LANG2.automorf.bin
  -->
  <xsl:template match="file" mode="untrim">
    <xsl:choose>
      <xsl:when test="contains(@name, '.automorf.')">
        <file>
          <xsl:attribute name="name">
            <xsl:call-template name="replaceString">
              <xsl:with-param name="haystack" select="./@name"/>
              <xsl:with-param name="needle" select="'.automorf.'"/>
              <xsl:with-param name="replacement" select="'.automorf-untrimmed.'"/>
            </xsl:call-template>
          </xsl:attribute>
        </file>
      </xsl:when>
      <xsl:otherwise>
        <xsl:copy>
          <xsl:apply-templates select="@* | node()" mode="untrim"/>
        </xsl:copy>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <xsl:template match="@* | node()" mode="untrim">
    <xsl:copy>
      <xsl:apply-templates select="@* | node()" mode="untrim"/>
    </xsl:copy>
  </xsl:template>

  <!-- catch-all -->
  <xsl:template match="@* | node()">
    <xsl:copy>
      <xsl:apply-templates select="@* | node()"/>
    </xsl:copy>
  </xsl:template>

</xsl:stylesheet>
