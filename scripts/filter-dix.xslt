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
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 02111-1307, USA.
-->
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
  <xsl:output method="xml" encoding="UTF-8"/>
  <xsl:param name="lang"/> <!-- language of the variant being generated -->
  <xsl:param name="side"/> <!-- one of 'left' or 'right' -->

<xsl:template match="alphabet">
  <alphabet><xsl:apply-templates/></alphabet>
</xsl:template>

<xsl:template match="sdefs">
  <sdefs>
    <xsl:apply-templates/>
  </sdefs>
</xsl:template>

<xsl:template match="sdef">
  <sdef n="{./@n}"/>
</xsl:template>

<xsl:template match="pardefs">
  <pardefs>
    <xsl:apply-templates/>
  </pardefs>
</xsl:template>

<xsl:template match="pardef">
  <pardef n="{./@n}">
    <xsl:apply-templates/>
  </pardef>
</xsl:template>

<xsl:template match="e">
  <xsl:choose>
    <xsl:when test="./@v=$lang">
      <xsl:choose>
        <xsl:when test="count(./@r)=0">
          <e>
  	    <xsl:apply-templates/>
          </e>
        </xsl:when>
        <xsl:otherwise>
          <e r="{./@r}">
  	    <xsl:apply-templates/>
          </e>
        </xsl:otherwise>
      </xsl:choose>    
    </xsl:when>
    <xsl:when test="not(count(./@v)=0) and not(./@v=$lang)">
      <xsl:choose>
	<xsl:when test="$side=string('left')">
	  <e r="LR">
	    <xsl:apply-templates/>
	  </e>
	</xsl:when>
	<xsl:otherwise>
  	  <xsl:if test="not(./@r=string('LR'))">
            <e r="RL">
 	      <xsl:apply-templates/>
	    </e>
          </xsl:if>
	</xsl:otherwise>
      </xsl:choose>
    </xsl:when>
    <xsl:otherwise>
      <xsl:choose>
	<xsl:when test="not(count(./@r))=0">
	  <e r="{./@r}">
	    <xsl:apply-templates/>
	  </e>
	</xsl:when>
	<xsl:otherwise>
	  <e>
	    <xsl:apply-templates/>
	  </e>
	</xsl:otherwise>
      </xsl:choose>
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>

<xsl:template match="p">
  <p>
    <xsl:apply-templates/>
  </p>
</xsl:template>

<xsl:template match="l">
  <l><xsl:apply-templates/></l>
</xsl:template>

<xsl:template match="r">
  <r><xsl:apply-templates/></r>
</xsl:template>

<xsl:template match="s">
  <s n="{./@n}"/>
</xsl:template>

<xsl:template match="b">
  <b/>
</xsl:template>

<xsl:template match="j">
  <j/>
</xsl:template>

<xsl:template match="a">
  <a/>
</xsl:template>

<xsl:template match="re">
  <re><xsl:apply-templates/></re>
</xsl:template>

<xsl:template match="section">
  <section id="{./@id}" type="{./@type}">
    <xsl:apply-templates/>
  </section>
</xsl:template>

<xsl:template match="i">
  <i>
    <xsl:apply-templates/>
  </i>
</xsl:template>

<xsl:template match="g"><g><xsl:apply-templates/></g></xsl:template>

<xsl:template match="par">
  <par n="{./@n}"/>
</xsl:template>

<xsl:template match="dictionary">
<dictionary>
  <xsl:apply-templates/>
</dictionary>
</xsl:template>



</xsl:stylesheet>
