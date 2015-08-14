<?xml version="1.0" encoding="ISO-8859-1"?><!-- -*- xml-*- -->
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
<!--
==========================================================================
| File..........: convert.xsl
| Author........: Marco A. Montava
| Date..........: 29-Jul-2006
| Description...: Conversor de diccionaris amb polisemia a tractament simple
==========================================================================
-->

<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
<xsl:output method="xml" encoding="ISO-8859-1" indent="no"/>

<xsl:template match="/">
  <xsl:value-of select="string('&#xa;')"/><!-- \n -->
  <xsl:apply-templates select="dictionary"/>
</xsl:template>

<xsl:template match="dictionary">
  <dictionary><xsl:value-of select="string('&#xa;')"/><!-- \n -->
  <xsl:copy-of select="alphabet"/> <xsl:value-of select="string('&#xa;')"/><!-- \n -->
  <xsl:copy-of select="sdefs"/> <xsl:value-of select="string('&#xa;')"/><!-- \n -->
  <xsl:copy-of select="pardefs"/> <xsl:value-of select="string('&#xa;')"/><!-- \n -->
  <xsl:apply-templates select="section"/>
  </dictionary><xsl:value-of select="string('&#xa;')"/><!-- \n -->
</xsl:template>

<xsl:template match="section">
  <xsl:value-of select="string('&#xa;')"/><!-- \n -->
  <section id='{@id}' type='{@type}'><xsl:value-of select="string('&#xa;')"/><!-- \n -->
  <xsl:apply-templates />
  </section><xsl:value-of select="string('&#xa;')"/><!-- \n -->
</xsl:template>




<xsl:template match="e[@slr|@srl]"> <!-- si te problemes de polisemia -->
  <xsl:choose>
     <!-- si te conflicte L-R y R-L -->
     <xsl:when test="./@slr!='' and ./@srl!='' ">
         <xsl:choose>
         <!-- si es per Defecte R-L i L-R-->
         <xsl:when test="substring(./@srl,(string-length(./@srl)-1),2)=' D' and substring(./@slr,(string-length(./@slr)-1),2)=' D' ">
            <e><xsl:value-of select="string('&#xa;')"/><!-- \n -->
               <xsl:copy-of select="*"/>
            <xsl:value-of select="string('&#xa;')"/><!-- \n -->
            </e>
         </xsl:when>
         <!-- si es per Defecte L-R -->
         <xsl:when test="substring(./@slr,(string-length(./@slr)-1),2)=' D'">
            <e r="LR"><xsl:value-of select="string('&#xa;')"/><!-- \n -->
               <xsl:copy-of select="*"/>
            <xsl:value-of select="string('&#xa;')"/><!-- \n -->
            </e>
         </xsl:when>
         <!-- si es per Defecte R-L -->
         <xsl:when test="substring(./@srl,(string-length(./@srl)-1),2)=' D'">
            <e r="RL"><xsl:value-of select="string('&#xa;')"/><!-- \n -->
               <xsl:copy-of select="*"/>
            <xsl:value-of select="string('&#xa;')"/><!-- \n -->
            </e>
         </xsl:when>
         <!-- **** si no es per defecte cap, llavors s'elimina, s'ignora **** -->
          </xsl:choose>
      </xsl:when>
      <!-- si sols te conflicte R-L -->
      <xsl:when test="./@srl!='' ">
         <xsl:choose>
         <!-- si es la solucio per Defecte R-L i te restriccio RL-->
         <xsl:when test="substring(./@srl,(string-length(./@srl)-1),2)=' D' and ./@r='RL'">
            <e r="RL"><xsl:value-of select="string('&#xa;')"/><!-- \n -->
               <xsl:copy-of select="*"/>
            <xsl:value-of select="string('&#xa;')"/><!-- \n -->
            </e>
         </xsl:when>
        <!-- si es la solucio per Defecte R-L -->
         <xsl:when test="substring(./@srl,(string-length(./@srl)-1),2)=' D' ">
            <e><xsl:value-of select="string('&#xa;')"/><!-- \n -->
               <xsl:copy-of select="*"/>
            <xsl:value-of select="string('&#xa;')"/><!-- \n -->
            </e>
         </xsl:when>
         <!-- si no es la solucio per Defecte R-L i te restriccio RL-->
         <xsl:when test="@r='RL' ">
            <!-- L'ELIMINEM -->
         </xsl:when>
         <!-- si no es la solucio per Defecte R-L -->
         <xsl:otherwise>
            <e r="LR"><xsl:value-of select="string('&#xa;')"/><!-- \n -->
               <xsl:copy-of select="*"/>
            <xsl:value-of select="string('&#xa;')"/><!-- \n -->
            </e>
         </xsl:otherwise>
         </xsl:choose>
      </xsl:when>
      <!-- si sols te conflicte L-R -->
      <xsl:when test="./@slr!=''">
         <xsl:choose>
         <!-- si es la solucio per Defecte L-R i te restriccio LR-->
         <xsl:when test="substring(./@slr,(string-length(./@slr)-1),2)=' D' and ./@r='LR'">
            <e r="LR"><xsl:value-of select="string('&#xa;')"/><!-- \n -->
               <xsl:copy-of select="*"/>
            <xsl:value-of select="string('&#xa;')"/><!-- \n -->
            </e>
         </xsl:when>
        <!-- si es la solucio per Defecte L-R -->
         <xsl:when test="substring(./@slr,(string-length(./@slr)-1),2)=' D' ">
            <e><xsl:value-of select="string('&#xa;')"/><!-- \n -->
               <xsl:copy-of select="*"/>
            <xsl:value-of select="string('&#xa;')"/><!-- \n -->
            </e>
         </xsl:when>
         <!-- si no es la solucio per Defecte L-R i te restriccio LR-->
         <xsl:when test="@r='LR' ">
            <!-- L'ELIMINEM -->
         </xsl:when>
         <!-- si no es la solucio per Defecte L-R -->
         <xsl:otherwise>
            <e r="RL"><xsl:value-of select="string('&#xa;')"/><!-- \n -->
               <xsl:copy-of select="*"/>
            <xsl:value-of select="string('&#xa;')"/><!-- \n -->
            </e>
         </xsl:otherwise>
         </xsl:choose>
      </xsl:when>
  </xsl:choose>    
</xsl:template>

<xsl:template match="e[not(@slr|@srl)]">  <!-- elements sense polisemia -->
  <xsl:copy-of select ="."/>
</xsl:template>


</xsl:stylesheet>