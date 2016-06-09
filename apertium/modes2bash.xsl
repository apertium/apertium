<?xml version="1.0" encoding="UTF-8"?><!-- -*- nxml -*- -->
<!--
 Copyright (C) 2005-2014 Universitat d'Alacant / Universidad de Alicante

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
<xsl:output method="text" encoding="UTF-8" indent="no"/>

<xsl:param name="installdir"/>
<xsl:param name="devdir"/>

<xsl:template match="modes">
  <xsl:apply-templates/>
</xsl:template>


<!-- Print filenames first, then the contents of each file. This gets
     parsed by apertium-createmodes.awk which splits it into the
     specific files. -->
<xsl:template match="mode">
  <xsl:choose>
    <xsl:when test="@install = 'yes'">
      <xsl:text>
# </xsl:text>
      <xsl:value-of select="./@name"/>
      <xsl:text>.mode
      </xsl:text>
      <xsl:apply-templates>
        <xsl:with-param name="dir"><xsl:value-of select="$installdir"/></xsl:with-param>
      </xsl:apply-templates>
    </xsl:when>
  </xsl:choose>
  <xsl:variable name="dir">
    <xsl:text>'</xsl:text>
    <xsl:value-of select="$devdir"/>
    <xsl:text>'</xsl:text>
  </xsl:variable>
  <xsl:text>
# modes/</xsl:text>
  <xsl:value-of select="./@name"/>
  <xsl:text>.mode
  </xsl:text>
  <xsl:apply-templates>
    <xsl:with-param name="dir"><xsl:value-of select="$devdir"/></xsl:with-param>
  </xsl:apply-templates>
</xsl:template>


<xsl:template match="pipeline">
  <xsl:param name="dir" />
  <xsl:for-each select="./*">
    <xsl:if test="not(position()=1)">
      <xsl:text>| </xsl:text>
    </xsl:if>
    <xsl:apply-templates select=".">
      <xsl:with-param name="dir"><xsl:value-of select="$dir"/></xsl:with-param>
    </xsl:apply-templates>
  </xsl:for-each>
</xsl:template>

<xsl:template match="program">
  <xsl:param name="dir" />
  <xsl:value-of select="./@name"/>
  <xsl:for-each select="./*">
    <xsl:text> </xsl:text>
    <xsl:apply-templates select=".">
      <xsl:with-param name="dir"><xsl:value-of select="$dir"/></xsl:with-param>
    </xsl:apply-templates>
  </xsl:for-each>
</xsl:template>

<xsl:template match="arg">
  <xsl:value-of select="./@name"/>
</xsl:template>

<xsl:template match="file">
  <xsl:param name="dir" />
  <xsl:text>'</xsl:text>
  <xsl:value-of select="$dir" />
  <xsl:text>/</xsl:text>
  <xsl:value-of select="./@name"/>
  <xsl:text>' </xsl:text>
</xsl:template>


</xsl:stylesheet>
