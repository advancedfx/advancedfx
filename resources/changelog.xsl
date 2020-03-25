<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE xsl:stylesheet[
	<!ENTITY nbsp "&#160;">
]>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="2.0">
<xsl:output method="html" omit-xml-declaration="no" indent="yes" />

<xsl:template match="/changelog">
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en" lang="en">
<head>
<title>Changelog</title>
<style type="text/css">
body {
	background:#e0e0e0;
}
h1 {
	background-color:#e0ffc0;
	border-color:#808080;
	border-style:solid;
	border-width:1px;
	padding:4pt;
}
.entry {
	display:block;
	background:#ffffff;
	border-color:#808080;
	border-style:solid;
	border-width:1px;
	margin-top:16pt;
	margin-bottom:16pt;
}
.title {
	background-color:#e0ffc0;
	display:block;
	font-family:monospace;
	font-weight:bold;
}
.version {
	display:inline;
}
.time {
	display:inline;
}
.notelist {
	display:block;
	font-family:monospace;
}
.note {
	display:block;
	margin-bottom:8pt;
	margin-left:4pt;
	margin-top:8pt;
	margin-right:4pt;
}
.note_icon {
	display:inline;
	font-weight:bold;
}
.comments {
	display:block;
	margin-left:4pt;
	margin-right:4pt;
}
</style>
</head>
<body>
<h1>Changelog</h1>
<xsl:apply-templates select="release" />
</body>
</html>
</xsl:template>

<xsl:template match="release">
	<div class="entry">
		<div class="title">
			<xsl:value-of select="name" />&nbsp;<xsl:value-of select="version" />
			<xsl:if test="time">&nbsp;(<xsl:value-of select="time" />)</xsl:if>
		</div>
		<xsl:apply-templates select="changes" />
		<xsl:apply-templates select="comments" />
	</div>
</xsl:template>

<xsl:template match="changes">
	<div class="notelist">
		<xsl:apply-templates select="change" />
	</div>
</xsl:template>

<xsl:template match="change">
	<xsl:choose>
		<xsl:when test="@type = 'added'">
			<div class="note">
				<span class="note_icon">* </span>
				<xsl:apply-templates select="node()" />
			</div>
		</xsl:when>
		<xsl:when test="@type = 'fixed'">
			<div class="note">
				<span class="note_icon">+ </span>
				<xsl:apply-templates select="node()" />
			</div>
		</xsl:when>
		<xsl:when test="@type = 'removed'">
			<div class="note">
				<span class="note_icon">- </span>
				<xsl:apply-templates select="node()" />
			</div>
		</xsl:when>
		<xsl:when test="@type = 'updated'">
			<div class="note">
				<span class="note_icon">U </span>
				<xsl:apply-templates select="node()" />
			</div>
		</xsl:when>
		<xsl:otherwise>
			<div class="note">
				<span class="note_icon">? </span>
				<xsl:apply-templates select="node()" />
			</div>
		</xsl:otherwise>
	</xsl:choose>
</xsl:template>

<xsl:template match="comments">
	<div class="comments">
		<xsl:apply-templates select="node()" />
	</div>
</xsl:template>

<xsl:template match="br">
	<br />
</xsl:template>

</xsl:stylesheet>
