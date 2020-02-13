<?xml version="1.0" encoding="UTF-8"?> <!-- -*- nxml -*- -->
<!--
Copyright (C) 2016 Universitat d'Alacant / Universidad de Alicante
Copyright (C) 2018 Xavi Ivars <xavi.ivars@gmail.com>

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
  <xsl:output method="xml" encoding="UTF-8" indent="yes"/>

  <xsl:template match="metalrx">
    <xsl:apply-templates select="./rules" />
  </xsl:template>

  <xsl:template match="seq">
    <xsl:copy-of select="/metalrx/def-seqs/def-seq[@n = current()/@n]/*"/>
  </xsl:template>
  
  <xsl:template match="prule">
	<xsl:call-template name="prule1">
	  <xsl:with-param name="p1" select="@p1" />
      <xsl:with-param name="p2" select="@p2" />
	  <xsl:with-param name="p3" select="@p3" />
      <xsl:with-param name="p4" select="@p4" />
	</xsl:call-template>
  </xsl:template>

  <xsl:template name="prule1">
	<xsl:param name="p1" />
	<xsl:param name="p2" />
	<xsl:param name="p3" />
	<xsl:param name="p4" />
	
  </xsl:template>

  <!-- Rules that don't have a repeat[@upto] element are
       just copied, their repeats ignored: -->
  <xsl:template match="rule[./repeat[@upto]]">
    <xsl:apply-templates select="./repeat[@upto]"/>
  </xsl:template>

  <xsl:template match="repeat[@upto]">
    <xsl:choose>
      <xsl:when test="not(./following-sibling::repeat[1])">
        <xsl:call-template name="repeat1">
          <xsl:with-param name="upto" select="@upto" />
          <xsl:with-param name="from" select="@from" />
        </xsl:call-template>
      </xsl:when>
      <xsl:otherwise>
        <ERROR-multiple-repeats><xsl:text>Several repeats per rule not supported yet</xsl:text></ERROR-multiple-repeats>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <!-- Actually output the stuff, and recurse: -->
  <xsl:template name="repeat1">
    <xsl:param name="upto" />
    <xsl:param name="from" select="1" />
    <xsl:choose><xsl:when test="$upto &gt; 0">
      <rule>
        <xsl:copy-of select="./../@*"/>
        <xsl:apply-templates select="./preceding-sibling::*"/>
        <xsl:call-template name="repeatElts">
          <xsl:with-param name="elts" select="./*" />
          <xsl:with-param name="upto" select="$upto" />
          <xsl:with-param name="from" select="$from" />
        </xsl:call-template>
        <xsl:apply-templates select="./following-sibling::*"/>
      </rule>
      <xsl:variable name="next">
        <repeat>
          <xsl:attribute name="upto"><xsl:value-of select="@upto - 1"/></xsl:attribute>
          <xsl:attribute name="from"><xsl:value-of select="@from"/></xsl:attribute>
          <xsl:copy-of select="./*"/>
        </repeat>
      </xsl:variable>
      <xsl:call-template name="repeat1">
        <xsl:with-param name="upto" select="$upto - 1" />
        <xsl:with-param name="from" select="$from" />
      </xsl:call-template>
    </xsl:when>
    <xsl:when test="$upto = 0 and $from = 0">
		<rule>
			<xsl:copy-of select="./../@*"/>
			<xsl:apply-templates select="./preceding-sibling::*"/>
			<xsl:apply-templates select="./following-sibling::*"/>
		</rule>
    </xsl:when>
    </xsl:choose>
  </xsl:template>

  <xsl:template match="repeat"> <!-- if no upto, you get zero repeats! -->
  </xsl:template>

  <!-- Repeat elts upto times -->
  <xsl:template name="repeatElts">
    <xsl:param name="elts" />
    <xsl:param name="upto" />
    <xsl:param name="from" />
    <xsl:if test="$upto &gt; 0">
      <xsl:apply-templates select="$elts" />
      <xsl:call-template name="repeatElts">
        <xsl:with-param name="elts" select="$elts" />
        <xsl:with-param name="upto" select="$upto - 1" />
        <xsl:with-param name="from" select="$from" />
      </xsl:call-template>
    </xsl:if>
  </xsl:template>

  <!-- catch-all -->
  <xsl:template match="@* | node()">
    <xsl:copy>
      <xsl:apply-templates select="@* | node()"/>
    </xsl:copy>
  </xsl:template>

</xsl:stylesheet>
