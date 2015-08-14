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
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
  <xsl:output method="xml" encoding="ISO-8859-1"/>
<xsl:param name="r2l"/>

<xsl:template match="s">
  <s n="{./@n}"/>
</xsl:template>

<xsl:template match="b">
  <b />
</xsl:template>

<xsl:template match="g">
  <g><xsl:apply-templates/></g>
</xsl:template>

<xsl:template match="a">
  <a/>
</xsl:template>

<xsl:template match="j">
  <j/>
</xsl:template>

<xsl:template match="l">
  <xsl:apply-templates select="./*|text()"/>
</xsl:template>

<xsl:template match="r">
  <xsl:apply-templates select="./*|text()"/>
</xsl:template>

<xsl:template match="par">
  <par n="{./@n}"/>
</xsl:template>

<xsl:template match="re">
  <re><xsl:apply-templates/></re>
</xsl:template>

<xsl:template match="p">
  <p>
  <xsl:choose>
    <xsl:when test="not($r2l=string('yes'))">
      <xsl:choose>
	<xsl:when test="not($r2l=string('yes')) and not(count(../@srl)=0)">
	  <l><xsl:apply-templates select="./r/text()|./r/*[not(name(.)=string('s'))]"/>__<xsl:apply-templates select="../@srl"/><xsl:apply-templates select="./r/*[name(.)=string('s')]"/></l>
	</xsl:when>
	<xsl:otherwise>
	  <l><xsl:apply-templates select="./r/*|./r/text()"/></l>
	</xsl:otherwise>
      </xsl:choose>
      <r><xsl:apply-templates select="./l/*|./l/text()"/></r>
    </xsl:when>
    <xsl:otherwise>
      <xsl:choose>
        <xsl:when test="($r2l=string('yes')) and not(count(../@slr)=0)">
          <l><xsl:apply-templates select="./l/text()|./l/*[not(name(.)=string('s'))]"/>__<xsl:apply-templates select="../@slr"/><xsl:apply-templates select="./l/*[name(.)=string('s')]"/></l>
        </xsl:when>
        <xsl:otherwise>
	  <l><xsl:apply-templates select="./l/*|./l/text()"/></l>
        </xsl:otherwise>
      </xsl:choose>
      <r><xsl:apply-templates select="./r/*|./r/text()"/></r>
    </xsl:otherwise>
  </xsl:choose>
  </p>
</xsl:template>


<xsl:template match="i">
  <p>
  <xsl:choose>
    <xsl:when test="not($r2l=string('yes'))">
      <xsl:choose>
	<xsl:when test="not($r2l=string('yes')) and not(count(../@srl)=0)">
	  <l><xsl:apply-templates select="text()|*[not(name(.)=string('s'))]"/>__<xsl:apply-templates select="../@srl"/><xsl:apply-templates select="*[name(.)=string('s')]"/></l>
	</xsl:when>
	<xsl:otherwise>
	  <l><xsl:apply-templates select="*|text()"/></l>
	</xsl:otherwise>
      </xsl:choose>
      <r><xsl:apply-templates select="*|text()"/></r>
    </xsl:when>
    <xsl:otherwise>
      <xsl:choose>
        <xsl:when test="($r2l=string('yes')) and not(count(../@slr)=0)">
          <l><xsl:apply-templates select="text()|*[not(name(.)=string('s'))]"/>__<xsl:apply-templates select="../@slr"/><xsl:apply-templates select="*[name(.)=string('s')]"/></l>
        </xsl:when>
        <xsl:otherwise>
	  <l><xsl:apply-templates select="*|text()"/></l>
        </xsl:otherwise>
      </xsl:choose>
      <r><xsl:apply-templates select="*|text()"/></r>
    </xsl:otherwise>
  </xsl:choose>
  </p>
</xsl:template>


<xsl:template match="e">
  <xsl:choose>
    <xsl:when test="./@i=string('yes')">
    </xsl:when>
    <xsl:when test="not($r2l=string('yes'))">
      <xsl:if test="not(./@r=string('LR'))">
        <e><xsl:apply-templates select="./*"/></e>
      </xsl:if>
    </xsl:when>
    <xsl:otherwise>
      <xsl:if test="not(./@r=string('RL'))">
        <e><xsl:apply-templates select="./*"/></e>
      </xsl:if>
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>


<xsl:template match="dictionary">
<dictionary>
  <xsl:value-of select="string('&#xA;')"/>
  <xsl:copy-of select="./alphabet"/>
  <xsl:value-of select="string('&#xA;')"/>
  <xsl:copy-of select="./sdefs"/>
  <xsl:value-of select="string('&#xA;')"/>
  <xsl:if test="not(count(./pardefs/pardef)=0)">
    <pardefs>
  <xsl:value-of select="string('&#xA;')"/>

      <xsl:for-each select="./pardefs/pardef">
  <xsl:value-of select="string('&#xA;')"/>

	<pardef n="{./@n}">
	  <xsl:apply-templates/>
	</pardef>
      </xsl:for-each>
  <xsl:value-of select="string('&#xA;')"/>

    </pardefs>
  </xsl:if>
  <xsl:value-of select="string('&#xA;')"/>
  <xsl:for-each select="./section">
    <section id="{./@id}" type="{./@type}">
      <xsl:apply-templates/>
    </section>
  </xsl:for-each>
</dictionary>

</xsl:template>


</xsl:stylesheet>
