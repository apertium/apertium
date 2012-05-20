<?xml version="1.0" encoding="ISO-8859-1"?><!-- -*- nxml -*- -->
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
<xsl:output method="text" encoding="ISO-8859-1" indent="no"/>

<xsl:param name="prefix"/>
<xsl:param name="dataprefix"/>

<xsl:template match="modes">
  <xsl:for-each select="./mode">
    ----<xsl:value-of select="./@name"/>:<xsl:choose><xsl:when test="@install = 'yes'">install:yes</xsl:when><xsl:otherwise>install:no</xsl:otherwise></xsl:choose>----
    <xsl:apply-templates/>
  </xsl:for-each>
</xsl:template>

<xsl:template match="mode">
  <xsl:apply-templates/>
</xsl:template>

<xsl:template match="pipeline">
  <xsl:for-each select="./*">
    <xsl:if test="not(position()=1)">
      <xsl:value-of select="string('|')"/>
    </xsl:if>
    <xsl:apply-templates select="."/>
  </xsl:for-each>
</xsl:template>

<xsl:template match="program">
  <xsl:choose>
    <xsl:when test="@prefix">
      <xsl:value-of select="@prefix"/>
      <xsl:value-of select="string('/')"/>
    </xsl:when>
  </xsl:choose>
  <xsl:value-of select="./@name"/>
  <xsl:for-each select="./*">  
    <xsl:value-of select="string(' ')"/>
    <xsl:apply-templates select="."/>
  </xsl:for-each>
</xsl:template>

<xsl:template match="file">
  <xsl:value-of select="$dataprefix"/>
  <xsl:value-of select="string('/')"/>
  <xsl:value-of select="./@name"/>
  <xsl:value-of select="string(' ')"/>
</xsl:template>

</xsl:stylesheet>
