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

  <xsl:preserve-space elements="*" />

<xsl:template match="section-rules">
  <section-rules>
    <xsl:for-each select="./rule">
      <xsl:choose>
	<xsl:when test="./@v=$lang">
          <rule>
	  <xsl:copy-of select="./*"/>
          </rule>
        </xsl:when>
	<xsl:when test="count(./@v)=0">
          <xsl:copy-of select="."/>
        </xsl:when>
	<xsl:otherwise/>
      </xsl:choose>
    </xsl:for-each>
  </section-rules>
</xsl:template>

<xsl:template match="section-def-macros">
  <section-def-macros>
    <xsl:for-each select="./def-macro">
      <xsl:choose>
	<xsl:when test="./@v=$lang">
          <def-macro n="{./@n}" npar="{./@npar}">
	  <xsl:copy-of select="./*"/>
          </def-macro>
        </xsl:when>
	<xsl:when test="count(./@v)=0">
          <xsl:copy-of select="."/>
        </xsl:when>
	<xsl:otherwise/>
      </xsl:choose>
    </xsl:for-each>
  </section-def-macros>
</xsl:template>

<xsl:template match="transfer[@default]">
<transfer default="{./@default}">
<xsl:apply-templates select="./section-def-cats"/>
<xsl:apply-templates select="./section-def-attrs"/>
<xsl:copy-of select="section-def-vars"/>
<xsl:copy-of select="section-def-lists"/>
<xsl:apply-templates select="./section-def-macros"/>
<xsl:apply-templates select="./section-rules"/>
</transfer>
</xsl:template>

<xsl:template match="transfer">
<transfer>
<xsl:apply-templates select="./section-def-cats"/>
<xsl:apply-templates select="./section-def-attrs"/>
<xsl:copy-of select="section-def-vars"/>
<xsl:copy-of select="section-def-lists"/>
<xsl:apply-templates select="./section-def-macros"/>
<xsl:apply-templates select="./section-rules"/>
</transfer>
</xsl:template>

<xsl:template match="interchunk">
<interchunk>
<xsl:apply-templates select="./section-def-cats"/>
<xsl:apply-templates select="./section-def-attrs"/>
<xsl:copy-of select="section-def-vars"/>
<xsl:copy-of select="section-def-lists"/>
<xsl:apply-templates select="./section-def-macros"/>
<xsl:apply-templates select="./section-rules"/>
</interchunk>
</xsl:template>

<xsl:template match="postchunk">
<postchunk>
<xsl:apply-templates select="./section-def-cats"/>
<xsl:apply-templates select="./section-def-attrs"/>
<xsl:copy-of select="section-def-vars"/>
<xsl:copy-of select="section-def-lists"/>
<xsl:apply-templates select="./section-def-macros"/>
<xsl:apply-templates select="./section-rules"/>
</postchunk>
</xsl:template>

<xsl:template match="lrx|rules|rule|def-seqs|def-seq|match|select|remove|or|repeat|seq|metalrx|def-macros|def-macro|macro|section-def-cats|def-cat|cat-item|section-def-attrs|def-attr|attr-item">
	<xsl:copy>
		<xsl:copy-of select="@*[not(local-name()='v')]"/>
		<xsl:for-each select="./node()">
			<xsl:choose>
				<xsl:when test="./@v=$lang or count(./@v)=0">
					<xsl:apply-templates select="."/>
				</xsl:when>
				<xsl:otherwise/>
			</xsl:choose>
		</xsl:for-each>
	</xsl:copy>
</xsl:template>

</xsl:stylesheet>
