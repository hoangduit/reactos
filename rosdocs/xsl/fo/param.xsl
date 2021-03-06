<?xml version="1.0" encoding="utf-8"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:src="http://nwalsh.com/xmlns/litprog/fragment" exclude-result-prefixes="src" version="1.0">

<!-- This file is generated from param.xweb; do not edit this file! -->

<!-- ********************************************************************
     $Id: param.xsl 3064 2002-06-13 20:29:51Z chorns $
     ********************************************************************

     This file is part of the XSL DocBook Stylesheet distribution.
     See ../README or http://nwalsh.com/docbook/xsl/ for copyright
     and other information.

     ******************************************************************** -->

<xsl:param name="admon.graphics.extension" select="'.png'"/>
<xsl:param name="admon.graphics" select="0"/>
<xsl:param name="admon.graphics.path">images/</xsl:param>
<xsl:attribute-set name="admonition.properties"/>

<xsl:attribute-set name="admonition.title.properties">
  <xsl:attribute name="font-size">14pt</xsl:attribute>
  <xsl:attribute name="font-weight">bold</xsl:attribute>
  <xsl:attribute name="hyphenate">false</xsl:attribute>
  <xsl:attribute name="keep-with-next.within-column">always</xsl:attribute>
</xsl:attribute-set>
<xsl:param name="alignment">justify</xsl:param>
<xsl:param name="appendix.autolabel" select="1"/>
<xsl:param name="arbortext.extensions" select="0"/>
<xsl:param name="author.othername.in.middle" select="1"/>
<xsl:param name="autotoc.label.separator" select="'. '"/>
<xsl:param name="biblioentry.item.separator">. </xsl:param>
<xsl:param name="bibliography.collection" select="'http://docbook.sourceforge.net/release/bibliography/bibliography.xml'"/>

<xsl:attribute-set name="blockquote.properties">
<xsl:attribute name="start-indent">0.5in</xsl:attribute>
<xsl:attribute name="end-indent">0.5in</xsl:attribute>
<xsl:attribute name="space-after.minimum">0.5em</xsl:attribute>
<xsl:attribute name="space-after.optimum">1em</xsl:attribute>
<xsl:attribute name="space-after.maximum">2em</xsl:attribute>
</xsl:attribute-set>

<xsl:param name="body.font.family">Times Roman</xsl:param>
<xsl:param name="body.font.master">10</xsl:param>
<xsl:param name="body.font.size">
 <xsl:value-of select="$body.font.master"/><xsl:text>pt</xsl:text>
</xsl:param>

<xsl:param name="body.margin.bottom">0.5in</xsl:param>


<xsl:param name="body.margin.top">1in</xsl:param>

<xsl:param name="callout.defaultcolumn" select="'60'"/>
<xsl:param name="callout.graphics.extension" select="'.png'"/>
<xsl:param name="callout.graphics" select="'1'"/>
<xsl:param name="callout.graphics.number.limit" select="'10'"/>
<xsl:param name="callout.graphics.path" select="'images/callouts/'"/>
<xsl:param name="callout.unicode.font" select="'ZapfDingbats'"/>
<xsl:param name="callout.unicode" select="0"/>
<xsl:param name="callout.unicode.number.limit" select="'10'"/>
<xsl:param name="callout.unicode.start.character" select="10102"/>
<xsl:param name="callouts.extension" select="'1'"/>
<xsl:param name="chapter.autolabel" select="1"/>
<xsl:param name="column.count" select="'1'"/>

<xsl:param name="column.count.of.index" select="$column.count"/>

<xsl:attribute-set name="compact.list.item.spacing">
  <xsl:attribute name="space-before.optimum">0em</xsl:attribute>
  <xsl:attribute name="space-before.minimum">0em</xsl:attribute>
  <xsl:attribute name="space-before.maximum">0.2em</xsl:attribute>
</xsl:attribute-set>
<xsl:attribute-set name="component.title.properties">
  <xsl:attribute name="space-before.optimum">2em</xsl:attribute>
  <xsl:attribute name="space-before.minimum">1.8em</xsl:attribute>
  <xsl:attribute name="space-before.maximum">2.2em</xsl:attribute>
  <xsl:attribute name="font-weight">bold</xsl:attribute>
  <xsl:attribute name="font-size">18pt</xsl:attribute>
  <xsl:attribute name="space-after.optimum">1.5em</xsl:attribute>
  <xsl:attribute name="space-after.minimum">1.3em</xsl:attribute>
  <xsl:attribute name="space-after.maximum">1.8em</xsl:attribute>
  <xsl:attribute name="hyphenate">false</xsl:attribute>
  <xsl:attribute name="keep-with-next.within-column">always</xsl:attribute>
</xsl:attribute-set>
<xsl:param name="default.table.width" select="''"/>
<xsl:param name="default.units" select="'pt'"/>
<xsl:param name="dingbat.font.family">Times Roman</xsl:param>
<xsl:param name="double.sided" select="'0'"/>

<xsl:param name="draft.watermark.image" select="'http://docbook.sourceforge.net/release/images/draft.png'"/>


<xsl:param name="firstterm.only.link" select="0"/>

<xsl:param name="footnote.font.size">
 <xsl:value-of select="$body.font.master * 0.8"/><xsl:text>pt</xsl:text>
</xsl:param>
<xsl:param name="fop.extensions" select="0"/>
<xsl:attribute-set name="formal.object.properties">
  <xsl:attribute name="space-before.minimum">0.5em</xsl:attribute>
  <xsl:attribute name="space-before.optimum">1em</xsl:attribute>
  <xsl:attribute name="space-before.maximum">2em</xsl:attribute>
  <xsl:attribute name="space-after.minimum">0.5em</xsl:attribute>
  <xsl:attribute name="space-after.optimum">1em</xsl:attribute>
  <xsl:attribute name="space-after.maximum">2em</xsl:attribute>
</xsl:attribute-set>

<xsl:param name="formal.procedures" select="1"/>


<xsl:param name="formal.title.placement">
figure before
example before
equation before
table before
procedure before
</xsl:param>


<xsl:attribute-set name="formal.title.properties" use-attribute-sets="normal.para.spacing">
  <xsl:attribute name="font-weight">bold</xsl:attribute>
  <xsl:attribute name="font-size">12pt</xsl:attribute>
  <xsl:attribute name="hyphenate">false</xsl:attribute>
  <xsl:attribute name="keep-with-next.within-column">always</xsl:attribute>
  <xsl:attribute name="space-after.minimum">0.4em</xsl:attribute>
  <xsl:attribute name="space-after.optimum">0.6em</xsl:attribute>
  <xsl:attribute name="space-after.maximum">0.8em</xsl:attribute>
</xsl:attribute-set>

<xsl:param name="funcsynopsis.decoration" select="1"/>
<xsl:param name="funcsynopsis.style">kr</xsl:param>
<xsl:param name="function.parens">0</xsl:param>
<xsl:param name="generate.index" select="1"/>


<xsl:param name="generate.toc">
/appendix toc
/article  toc
book      toc,figure,table,example,equation
/chapter  toc
part      toc
/preface  toc
qandadiv  toc
qandaset  toc
reference toc
/section  toc
set       toc
</xsl:param>


<xsl:param name="glossary.collection" select="''"/>

<xsl:param name="glossterm.auto.link" select="'0'"/>
<xsl:param name="graphic.default.extension"/>
<xsl:param name="hyphenate">true</xsl:param>
<xsl:attribute-set name="informal.object.properties">
  <xsl:attribute name="space-before.minimum">0.5em</xsl:attribute>
  <xsl:attribute name="space-before.optimum">1em</xsl:attribute>
  <xsl:attribute name="space-before.maximum">2em</xsl:attribute>
  <xsl:attribute name="space-after.minimum">0.5em</xsl:attribute>
  <xsl:attribute name="space-after.optimum">1em</xsl:attribute>
  <xsl:attribute name="space-after.maximum">2em</xsl:attribute>
</xsl:attribute-set>
<xsl:param name="insert.xref.page.number" select="0"/>
<xsl:param name="label.from.part" select="'0'"/>

<xsl:param name="line-height" select="'normal'"/>

<xsl:param name="linenumbering.everyNth" select="'5'"/>
<xsl:param name="linenumbering.extension" select="'1'"/>
<xsl:param name="linenumbering.separator" select="' '"/>
<xsl:param name="linenumbering.width" select="'3'"/>
<xsl:attribute-set name="list.block.spacing">
  <xsl:attribute name="space-before.optimum">1em</xsl:attribute>
  <xsl:attribute name="space-before.minimum">0.8em</xsl:attribute>
  <xsl:attribute name="space-before.maximum">1.2em</xsl:attribute>
</xsl:attribute-set>
<xsl:attribute-set name="list.item.spacing">
  <xsl:attribute name="space-before.optimum">1em</xsl:attribute>
  <xsl:attribute name="space-before.minimum">0.8em</xsl:attribute>
  <xsl:attribute name="space-before.maximum">1.2em</xsl:attribute>
</xsl:attribute-set>
<xsl:param name="make.single.year.ranges" select="0"/>
<xsl:param name="make.year.ranges" select="0"/>
<xsl:param name="monospace.font.family">Courier</xsl:param>
<xsl:attribute-set name="monospace.verbatim.properties" use-attribute-sets="verbatim.properties">
  <xsl:attribute name="font-family">
    <xsl:value-of select="$monospace.font.family"/>
  </xsl:attribute>
  <xsl:attribute name="font-size">
    <xsl:value-of select="$body.font.master * 0.9"/>
    <xsl:text>pt</xsl:text>
  </xsl:attribute>
</xsl:attribute-set>
<xsl:param name="nominal.table.width" select="'6in'"/>
<xsl:attribute-set name="normal.para.spacing">
  <xsl:attribute name="space-before.optimum">1em</xsl:attribute>
  <xsl:attribute name="space-before.minimum">0.8em</xsl:attribute>
  <xsl:attribute name="space-before.maximum">1.2em</xsl:attribute>
</xsl:attribute-set>
<xsl:param name="page.height">
  <xsl:choose>
    <xsl:when test="$page.orientation = 'portrait'">
      <xsl:value-of select="$page.height.portrait"/>
    </xsl:when>
    <xsl:otherwise>
      <xsl:value-of select="$page.width.portrait"/>
    </xsl:otherwise>
  </xsl:choose>
</xsl:param>
<xsl:param name="page.height.portrait">
  <xsl:choose>
    <xsl:when test="$paper.type = 'A4landscape'">210mm</xsl:when>
    <xsl:when test="$paper.type = 'USletter'">11in</xsl:when>
    <xsl:when test="$paper.type = 'USlandscape'">8.5in</xsl:when>
    <xsl:when test="$paper.type = '4A0'">2378mm</xsl:when>
    <xsl:when test="$paper.type = '2A0'">1682mm</xsl:when>
    <xsl:when test="$paper.type = 'A0'">1189mm</xsl:when>
    <xsl:when test="$paper.type = 'A1'">841mm</xsl:when>
    <xsl:when test="$paper.type = 'A2'">594mm</xsl:when>
    <xsl:when test="$paper.type = 'A3'">420mm</xsl:when>
    <xsl:when test="$paper.type = 'A4'">297mm</xsl:when>
    <xsl:when test="$paper.type = 'A5'">210mm</xsl:when>
    <xsl:when test="$paper.type = 'A6'">148mm</xsl:when>
    <xsl:when test="$paper.type = 'A7'">105mm</xsl:when>
    <xsl:when test="$paper.type = 'A8'">74mm</xsl:when>
    <xsl:when test="$paper.type = 'A9'">52mm</xsl:when>
    <xsl:when test="$paper.type = 'A10'">37mm</xsl:when>
    <xsl:when test="$paper.type = 'B0'">1414mm</xsl:when>
    <xsl:when test="$paper.type = 'B1'">1000mm</xsl:when>
    <xsl:when test="$paper.type = 'B2'">707mm</xsl:when>
    <xsl:when test="$paper.type = 'B3'">500mm</xsl:when>
    <xsl:when test="$paper.type = 'B4'">353mm</xsl:when>
    <xsl:when test="$paper.type = 'B5'">250mm</xsl:when>
    <xsl:when test="$paper.type = 'B6'">176mm</xsl:when>
    <xsl:when test="$paper.type = 'B7'">125mm</xsl:when>
    <xsl:when test="$paper.type = 'B8'">88mm</xsl:when>
    <xsl:when test="$paper.type = 'B9'">62mm</xsl:when>
    <xsl:when test="$paper.type = 'B10'">44mm</xsl:when>
    <xsl:when test="$paper.type = 'C0'">1297mm</xsl:when>
    <xsl:when test="$paper.type = 'C1'">917mm</xsl:when>
    <xsl:when test="$paper.type = 'C2'">648mm</xsl:when>
    <xsl:when test="$paper.type = 'C3'">458mm</xsl:when>
    <xsl:when test="$paper.type = 'C4'">324mm</xsl:when>
    <xsl:when test="$paper.type = 'C5'">229mm</xsl:when>
    <xsl:when test="$paper.type = 'C6'">162mm</xsl:when>
    <xsl:when test="$paper.type = 'C7'">114mm</xsl:when>
    <xsl:when test="$paper.type = 'C8'">81mm</xsl:when>
    <xsl:when test="$paper.type = 'C9'">57mm</xsl:when>
    <xsl:when test="$paper.type = 'C10'">40mm</xsl:when>
    <xsl:otherwise>11in</xsl:otherwise>
  </xsl:choose>
</xsl:param>

<xsl:param name="page.margin.bottom">0.5in</xsl:param>

<xsl:param name="page.margin.inner">
  <xsl:choose>
    <xsl:when test="$double.sided != 0">1.25in</xsl:when>
    <xsl:otherwise>1in</xsl:otherwise>
  </xsl:choose>
</xsl:param>
<xsl:param name="page.margin.outer">
  <xsl:choose>
    <xsl:when test="$double.sided != 0">0.75in</xsl:when>
    <xsl:otherwise>1in</xsl:otherwise>
  </xsl:choose>
</xsl:param>

<xsl:param name="page.margin.top">0in</xsl:param>

<xsl:param name="page.orientation" select="'portrait'"/>
<xsl:param name="page.width">
  <xsl:choose>
    <xsl:when test="$page.orientation = 'portrait'">
      <xsl:value-of select="$page.width.portrait"/>
    </xsl:when>
    <xsl:otherwise>
      <xsl:value-of select="$page.height.portrait"/>
    </xsl:otherwise>
  </xsl:choose>
</xsl:param>
<xsl:param name="page.width.portrait">
  <xsl:choose>
    <xsl:when test="$paper.type = 'USletter'">8.5in</xsl:when>
    <xsl:when test="$paper.type = '4A0'">1682mm</xsl:when>
    <xsl:when test="$paper.type = '2A0'">1189mm</xsl:when>
    <xsl:when test="$paper.type = 'A0'">841mm</xsl:when>
    <xsl:when test="$paper.type = 'A1'">594mm</xsl:when>
    <xsl:when test="$paper.type = 'A2'">420mm</xsl:when>
    <xsl:when test="$paper.type = 'A3'">297mm</xsl:when>
    <xsl:when test="$paper.type = 'A4'">210mm</xsl:when>
    <xsl:when test="$paper.type = 'A5'">148mm</xsl:when>
    <xsl:when test="$paper.type = 'A6'">105mm</xsl:when>
    <xsl:when test="$paper.type = 'A7'">74mm</xsl:when>
    <xsl:when test="$paper.type = 'A8'">52mm</xsl:when>
    <xsl:when test="$paper.type = 'A9'">37mm</xsl:when>
    <xsl:when test="$paper.type = 'A10'">26mm</xsl:when>
    <xsl:when test="$paper.type = 'B0'">1000mm</xsl:when>
    <xsl:when test="$paper.type = 'B1'">707mm</xsl:when>
    <xsl:when test="$paper.type = 'B2'">500mm</xsl:when>
    <xsl:when test="$paper.type = 'B3'">353mm</xsl:when>
    <xsl:when test="$paper.type = 'B4'">250mm</xsl:when>
    <xsl:when test="$paper.type = 'B5'">176mm</xsl:when>
    <xsl:when test="$paper.type = 'B6'">125mm</xsl:when>
    <xsl:when test="$paper.type = 'B7'">88mm</xsl:when>
    <xsl:when test="$paper.type = 'B8'">62mm</xsl:when>
    <xsl:when test="$paper.type = 'B9'">44mm</xsl:when>
    <xsl:when test="$paper.type = 'B10'">31mm</xsl:when>
    <xsl:when test="$paper.type = 'C0'">917mm</xsl:when>
    <xsl:when test="$paper.type = 'C1'">648mm</xsl:when>
    <xsl:when test="$paper.type = 'C2'">458mm</xsl:when>
    <xsl:when test="$paper.type = 'C3'">324mm</xsl:when>
    <xsl:when test="$paper.type = 'C4'">229mm</xsl:when>
    <xsl:when test="$paper.type = 'C5'">162mm</xsl:when>
    <xsl:when test="$paper.type = 'C6'">114mm</xsl:when>
    <xsl:when test="$paper.type = 'C7'">81mm</xsl:when>
    <xsl:when test="$paper.type = 'C8'">57mm</xsl:when>
    <xsl:when test="$paper.type = 'C9'">40mm</xsl:when>
    <xsl:when test="$paper.type = 'C10'">28mm</xsl:when>
    <xsl:otherwise>8.5in</xsl:otherwise>
  </xsl:choose>
</xsl:param>
<xsl:param name="paper.type" select="'USletter'"/>
<xsl:param name="part.autolabel" select="1"/>
<xsl:param name="passivetex.extensions" select="0"/>
<xsl:param name="preface.autolabel" select="0"/>
<xsl:param name="process.empty.source.toc" select="0"/>
<xsl:param name="process.source.toc" select="0"/>

<xsl:param name="profile.arch" select="''"/>


<xsl:param name="profile.condition" select="''"/>


<xsl:param name="profile.conformance" select="''"/>


<xsl:param name="profile.lang" select="''"/>


<xsl:param name="profile.os" select="''"/>


<xsl:param name="profile.revision" select="''"/>


<xsl:param name="profile.revisionflag" select="''"/>


<xsl:param name="profile.role" select="''"/>


<xsl:param name="profile.security" select="''"/>


<xsl:param name="profile.userlevel" select="''"/>


<xsl:param name="profile.vendor" select="''"/>


<xsl:param name="profile.attribute" select="''"/>


<xsl:param name="profile.value" select="''"/>


<xsl:param name="profile.separator" select="';'"/>


<xsl:param name="punct.honorific" select="'.'"/>

<xsl:param name="qanda.inherit.numeration" select="1"/>
<xsl:param name="qandadiv.autolabel" select="1"/>
<xsl:param name="refentry.generate.name" select="1"/>

<xsl:param name="refentry.generate.title" select="0"/>

<xsl:param name="refentry.xref.manvolnum" select="1"/>

<xsl:param name="region.after.extent" select="'0.5in'"/>


<xsl:param name="region.before.extent" select="'0.5in'"/>

<xsl:param name="rootid" select="''"/>
<xsl:param name="runinhead.default.title.end.punct" select="'.'"/>
<xsl:param name="runinhead.title.end.punct" select="'.!?:'"/>
<xsl:param name="sans.font.family">Helvetica</xsl:param>
<xsl:param name="section.autolabel" select="0"/>
<xsl:param name="section.label.includes.component.label" select="0"/>

<xsl:param name="segmentedlist.as.table" select="0"/>

<xsl:param name="shade.verbatim" select="0"/>


<xsl:attribute-set name="shade.verbatim.style">
  <xsl:attribute name="background-color">#E0E0E0</xsl:attribute>
</xsl:attribute-set>

<xsl:param name="show.comments">1</xsl:param>

<xsl:param name="table.border.color" select="'black'"/>


<xsl:param name="table.border.style" select="'solid'"/>


<xsl:param name="table.border.thickness" select="'0.5pt'"/>


<xsl:attribute-set name="table.cell.padding">
  <xsl:attribute name="padding-left">2pt</xsl:attribute>
  <xsl:attribute name="padding-right">2pt</xsl:attribute>
  <xsl:attribute name="padding-top">2pt</xsl:attribute>
  <xsl:attribute name="padding-bottom">2pt</xsl:attribute>
</xsl:attribute-set>


<xsl:param name="table.entry.padding" select="'2pt'"/>

<xsl:param name="tablecolumns.extension" select="'1'"/>
<xsl:param name="textinsert.extension" select="'1'"/>

<xsl:param name="tex.math.in.alt" select="''"/>


<xsl:param name="tex.math.delims" select="'1'"/>

<xsl:param name="title.font.family">Helvetica</xsl:param>

<xsl:param name="title.margin.left" select="'-4pc'"/>


<xsl:param name="toc.indent.width" select="24"/>

<xsl:attribute-set name="toc.margin.properties">
  <xsl:attribute name="space-before.minimum">0.5em</xsl:attribute>
  <xsl:attribute name="space-before.optimum">1em</xsl:attribute>
  <xsl:attribute name="space-before.maximum">2em</xsl:attribute>
  <xsl:attribute name="space-after.minimum">0.5em</xsl:attribute>
  <xsl:attribute name="space-after.optimum">1em</xsl:attribute>
  <xsl:attribute name="space-after.maximum">2em</xsl:attribute>
</xsl:attribute-set>
<xsl:param name="toc.section.depth">2</xsl:param>
<xsl:param name="use.extensions" select="'0'"/>

<xsl:param name="use.svg" select="1"/>


<xsl:param name="ulink.footnotes" select="0"/>


<xsl:param name="ulink.hyphenate" select="''"/>


<xsl:param name="ulink.show" select="1"/>


<xsl:param name="variablelist.as.blocks" select="0"/>


<xsl:attribute-set name="verbatim.properties">
  <xsl:attribute name="space-before.minimum">0.8em</xsl:attribute>
  <xsl:attribute name="space-before.optimum">1em</xsl:attribute>
  <xsl:attribute name="space-before.maximum">1.2em</xsl:attribute>
</xsl:attribute-set>
<xsl:param name="xep.extensions" select="0"/>
<xsl:attribute-set name="xref.properties">

</xsl:attribute-set>

</xsl:stylesheet>
