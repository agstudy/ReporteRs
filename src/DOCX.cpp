/*
 * This file is part of ReporteRs.
 * Copyright (c) 2014, David Gohel All rights reserved.
 *
 * ReporteRs is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ReporteRs is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with ReporteRs.  If not, see <http://www.gnu.org/licenses/>.
**/

#include <string.h>
#include <stdio.h>
#include <math.h>

#define R_USE_PROTOTYPES 1

#include "datastruct.h"
#include "colors.h"
#include "dml_utils.h"
#include "common.h"
#include "DOCX.h"
#include "utils.h"

static char docx_elt_tag_start[] = "<wps:wsp>";
static char docx_elt_tag_end[] = "</wps:wsp>";
static char docx_lock_properties[] = "<wps:cNvSpPr><a:spLocks noSelect=\"1\" noResize=\"1\" noEditPoints=\"1\" noTextEdit=\"1\" noMove=\"1\" noRot=\"1\" noChangeShapeType=\"1\"/></wps:cNvSpPr><wps:nvPr />";
static char docx_unlock_properties[] = "<wps:cNvSpPr /><wps:nvPr />";


void DOCX_setRunProperties(pDevDesc dev, R_GE_gcontext *gc, double fontsize){
	DOCDesc *pd = (DOCDesc *) dev->deviceSpecific;
	int alpha =  (int) ((255-R_ALPHA(gc->col))/255.0 * 100000);
	int fontface = gc->fontface;
	fputs("<w:rPr>", pd->dmlFilePointer );
	fprintf(pd->dmlFilePointer, "<w:rFonts w:ascii=\"%s\" w:hAnsi=\"%s\" w:cs=\"%s\" />", pd->fi->fontname, pd->fi->fontname, pd->fi->fontname );
	if ( fontface == 2 || fontface == 4 ) {
		fputs("<w:b />", pd->dmlFilePointer);
	}
	if (fontface == 3 || fontface == 4) {
		fputs( "<w:i />", pd->dmlFilePointer );
	}
	fprintf(pd->dmlFilePointer, "<w:color w:val=\"%s\" />", RGBHexValue(gc->col) );
	fprintf(pd->dmlFilePointer, "<w:sz w:val=\"%.0f\" />", fontsize * 2 );
	fprintf(pd->dmlFilePointer, "<w:szCs w:val=\"%.0f\" />", fontsize * 2 );

	if (alpha > 0) {
		fputs( "<w14:textFill>", pd->dmlFilePointer );
		fputs( "<w14:solidFill>", pd->dmlFilePointer );
		fprintf(pd->dmlFilePointer, "<w14:srgbClr w14:val=\"%s\">", RGBHexValue(gc->col) );
		fprintf(pd->dmlFilePointer, "<w14:alpha w14:val=\"%d\" />", alpha);
		fputs( "</w14:srgbClr>", pd->dmlFilePointer);
		fputs( "</w14:solidFill>", pd->dmlFilePointer );
		fputs( "</w14:textFill>", pd->dmlFilePointer );
	}

	fputs( "</w:rPr>", pd->dmlFilePointer );
}


static Rboolean DOCXDeviceDriver(pDevDesc dev, const char* filename, double* width,
		double* height, double* offx, double* offy, double ps, int nbplots,
		const char* fontname, int id_init_value, int editable) {


	DOCDesc *rd;
	rd = (DOCDesc *) malloc(sizeof(DOCDesc));

	FontInfo *fi = (FontInfo *) malloc(sizeof(FontInfo));
	fi->isinit=0;
	fi->fontsize=(int) ps;
	rd->fi = fi;


	rd->filename = strdup(filename);
	rd->fontname = strdup(fontname);
	rd->id = id_init_value;
	rd->pageNumber = 0;
	rd->offx = offx[0];
	rd->offy = offy[0];
	rd->extx = width[0];
	rd->exty = height[0];
	rd->maxplot = nbplots;
	rd->x = offx;
	rd->y = offy;
	rd->width = width;
	rd->height = height;
	rd->fontface = 1;
	rd->fontsize = (int) ps;
	//rd->env=env;


	//
	//  Device functions
	//
	dev->deviceSpecific = rd;
	dev->activate = DOCX_activate;
	dev->close = DOCX_Close;
	dev->size = DOCX_Size;
	dev->newPage = DOCX_NewPage;
	dev->clip = DOCX_Clip;
	dev->strWidthUTF8 = DOCX_StrWidthUTF8;
	dev->strWidth = DOCX_StrWidth;
	dev->textUTF8 = DOCX_TextUTF8;
	dev->text = DOCX_Text;
	dev->rect = DOCX_Rect;
	dev->circle = DOCX_Circle;
	dev->line = DOCX_Line;
	dev->polyline = DOCX_Polyline;
	dev->polygon = DOCX_Polygon;
	dev->metricInfo = DOCX_MetricInfo;
	dev->hasTextUTF8 = (Rboolean) TRUE;
	dev->wantSymbolUTF8 = (Rboolean) TRUE;
	dev->useRotatedTextInContour = (Rboolean) FALSE;
	/*
	 * Initial graphical settings
	 */
	dev->startfont = 1;
	dev->startps = ps;
	dev->startcol = R_RGB(0, 0, 0);
	dev->startfill = R_TRANWHITE;
	dev->startlty = LTY_SOLID;
	dev->startgamma = 1;


	/*
	 * Device physical characteristics
	 */

	dev->left = 0;
	dev->right = width[0];
	dev->bottom = height[0];
	dev->top = 0;

	dev->clipLeft = 0;
	dev->clipRight = width[0];
	dev->clipBottom = height[0];
	dev->clipTop = 0;

	rd->clippedx0 = dev->clipLeft;
	rd->clippedy0 = dev->clipTop;
	rd->clippedx1 = dev->clipRight;
	rd->clippedy1 = dev->clipBottom;

	dev->cra[0] = 0.9 * ps;
	dev->cra[1] = 1.2 * ps;
	dev->xCharOffset = 0.4900;
	dev->yCharOffset = 0.3333;
	//dev->yLineBias = 0.2;
	dev->ipr[0] = 1.0 / 72.2;
	dev->ipr[1] = 1.0 / 72.2;
	/*
	 * Device capabilities
	 */
	dev->canClip = (Rboolean) TRUE;
	dev->canHAdj = 2;
	dev->canChangeGamma = (Rboolean) FALSE;
	dev->displayListOn = (Rboolean) FALSE;

	dev->haveTransparency = 2;
	dev->haveTransparentBg = 3;

	rd->editable = editable;

	return (Rboolean) TRUE;
}


void GE_DOCXDevice(const char* filename, double* width, double* height, double* offx,
		double* offy, double ps, int nbplots, const char* fontfamily, int id_init_value, int editable) {
	pDevDesc dev = NULL;
	pGEDevDesc dd;
	R_GE_checkVersionOrDie (R_GE_version);
	R_CheckDeviceAvailable();

	if (!(dev = (pDevDesc) calloc(1, sizeof(DevDesc))))
		Rf_error("unable to start DOCX device");
	if (!DOCXDeviceDriver(dev, filename, width, height, offx, offy, ps, nbplots,
			fontfamily, id_init_value, editable)) {
		free(dev);
		Rf_error("unable to start DOCX device");
	}

	dd = GEcreateDevDesc(dev);
	GEaddDevice2(dd, "DOCX");

}

static void DOCX_activate(pDevDesc dev) {
}

static void DOCX_Circle(double x, double y, double r, const pGEcontext gc, pDevDesc dev) {
	DOCDesc *pd = (DOCDesc *) dev->deviceSpecific;
	int idx = get_and_increment_idx(dev);

	fputs( docx_elt_tag_start, pd->dmlFilePointer );

	if( pd->editable > 0 )
		fprintf(pd->dmlFilePointer,	"<wps:cNvPr id=\"%d\" name=\"Point %d\" />%s", idx,	idx, docx_unlock_properties);
	else fprintf(pd->dmlFilePointer,	"<wps:cNvPr id=\"%d\" name=\"Point %d\" />%s", idx,	idx, docx_lock_properties);
	fputs( "<wps:spPr>", pd->dmlFilePointer );
	fputs( "<a:xfrm>", pd->dmlFilePointer);
	fprintf(pd->dmlFilePointer, "<a:off x=\"%.0f\" y=\"%.0f\"/>",
			p2e_(pd->offx + x - r), p2e_(pd->offy + y - r));
	fprintf(pd->dmlFilePointer, "<a:ext cx=\"%.0f\" cy=\"%.0f\"/>", p2e_(r * 2),
			p2e_(r * 2));
	fputs( "</a:xfrm>", pd->dmlFilePointer);
	fputs( "<a:prstGeom prst=\"ellipse\"><a:avLst /></a:prstGeom>", pd->dmlFilePointer);

	DML_SetFillColor(dev, gc);
	DML_SetLineSpec(dev, gc);
	fputs("</wps:spPr>", pd->dmlFilePointer );

	fputs("<wps:bodyPr />", pd->dmlFilePointer );
	fputs(docx_elt_tag_end, pd->dmlFilePointer );
	fflush(pd->dmlFilePointer);
}

static void DOCX_Line(double x1, double y1, double x2, double y2,
		const pGEcontext gc, pDevDesc dev) {
	DOCDesc *pd = (DOCDesc *) dev->deviceSpecific;

	double maxx = 0, maxy = 0;
	double minx = 0, miny = 0;

	DOC_ClipLine(x1, y1, x2, y2, dev);
	x1 = pd->clippedx0;y1 = pd->clippedy0;
	x2 = pd->clippedx1;y2 = pd->clippedy1;

	if (x2 > x1) {
		maxx = x2;
		minx = x1;
	} else {
		maxx = x1;
		minx = x2;
	}
	if (y2 > y1) {
		maxy = y2;
		miny = y1;
	} else {
		maxy = y1;
		miny = y2;
	}

	int idx = get_and_increment_idx(dev);

	fputs(docx_elt_tag_start, pd->dmlFilePointer );
	
	if( pd->editable > 0 )
		fprintf(pd->dmlFilePointer,
			"<wps:cNvPr id=\"%d\" name=\"Line %d\" />%s", idx,	idx, docx_unlock_properties);
	else fprintf(pd->dmlFilePointer,
			"<wps:cNvPr id=\"%d\" name=\"Line %d\" />%s", idx,	idx, docx_lock_properties);
	fputs("<wps:spPr>", pd->dmlFilePointer );
		fputs("<a:xfrm>", pd->dmlFilePointer );//fprintf(pd->dmlFilePointer, "<a:xfrm>");
			fprintf(pd->dmlFilePointer, "<a:off x=\"%.0f\" y=\"%.0f\"/>"
					, p2e_(pd->offx + minx), p2e_(pd->offy + miny));
			fprintf(pd->dmlFilePointer, "<a:ext cx=\"%.0f\" cy=\"%.0f\"/>"
					, p2e_(maxx-minx), p2e_(maxy-miny));

		fputs("</a:xfrm>", pd->dmlFilePointer );
		fputs("<a:custGeom>", pd->dmlFilePointer );
		fputs("<a:pathLst>", pd->dmlFilePointer );
			fprintf(pd->dmlFilePointer, "<a:path w=\"%.0f\" h=\"%.0f\">", p2e_(maxx-minx), p2e_(maxy-miny));


				fprintf(pd->dmlFilePointer,
						"<a:moveTo><a:pt x=\"%.0f\" y=\"%.0f\" /></a:moveTo>", p2e_(x1 - minx), p2e_(y1 - miny));
				fprintf(pd->dmlFilePointer,
						"<a:lnTo><a:pt x=\"%.0f\" y=\"%.0f\" /></a:lnTo>", p2e_(x2 - minx), p2e_(y2 - miny));
			fputs("</a:path>", pd->dmlFilePointer );
		fputs("</a:pathLst>", pd->dmlFilePointer );

	fputs("</a:custGeom><a:noFill />", pd->dmlFilePointer );
	DML_SetLineSpec(dev, gc);
	fputs("</wps:spPr>", pd->dmlFilePointer );

	fputs("<wps:bodyPr />", pd->dmlFilePointer );
	fputs(docx_elt_tag_end, pd->dmlFilePointer );
	//fprintf(pd->dmlFilePointer, "\n");

	fflush(pd->dmlFilePointer);

	//return;
}

static void DOCX_Polyline(int n, double *x, double *y, const pGEcontext gc,
		pDevDesc dev) {

	DOCDesc *pd = (DOCDesc *) dev->deviceSpecific;
	int idx = get_and_increment_idx(dev);
	int i;
	double maxx = 0, maxy = 0;
	for (i = 0; i < n; i++) {
		if (x[i] > maxx)
			maxx = x[i];
		if (y[i] > maxy)
			maxy = y[i];
	}
	double minx = maxx, miny = maxy;

	for (i = 0; i < n; i++) {
		if (x[i] < minx)
			minx = x[i];
		if (y[i] < miny)
			miny = y[i];
	}

	DOC_ClipLine(minx, miny, maxx, maxy, dev);
	minx = pd->clippedx0;miny = pd->clippedy0;
	maxx = pd->clippedx1;maxy = pd->clippedy1;

	for (i = 1; i < n; i++) {
		DOC_ClipLine(x[i-1], y[i-1], x[i], y[i], dev);
		if( i < 2 ){
			x[i-1] = pd->clippedx0;
			y[i-1] = pd->clippedy0;
		}
		x[i] = pd->clippedx1;
		y[i] = pd->clippedy1;
	}

	fputs(docx_elt_tag_start, pd->dmlFilePointer );
	if( pd->editable < 1 )
		fprintf(pd->dmlFilePointer,
			"<wps:cNvPr id=\"%d\" name=\"Polyline %d\" />%s", idx,	idx, docx_lock_properties);
	else fprintf(pd->dmlFilePointer,
			"<wps:cNvPr id=\"%d\" name=\"Polyline %d\" />%s", idx,	idx, docx_unlock_properties);

	fputs("<wps:spPr>", pd->dmlFilePointer );
	fputs("<a:xfrm>", pd->dmlFilePointer );
	fprintf(pd->dmlFilePointer, "<a:off x=\"%.0f\" y=\"%.0f\"/>",
			p2e_(pd->offx + minx), p2e_(pd->offy + miny));
	fprintf(pd->dmlFilePointer, "<a:ext cx=\"%.0f\" cy=\"%.0f\"/>",
			p2e_(maxx - minx), p2e_(maxy - miny));
	fputs("</a:xfrm>", pd->dmlFilePointer );
	fputs("<a:custGeom>", pd->dmlFilePointer );

	fputs("<a:pathLst>", pd->dmlFilePointer );
	fprintf(pd->dmlFilePointer, "<a:path w=\"%.0f\" h=\"%.0f\">", p2e_(maxx-minx), p2e_(maxy-miny));


	fprintf(pd->dmlFilePointer,
			"<a:moveTo><a:pt x=\"%.0f\" y=\"%.0f\" /></a:moveTo>",
			p2e_(x[0] - minx), p2e_(y[0] - miny));

	for (i = 1; i < n; i++) {
		fprintf(pd->dmlFilePointer,
				"<a:lnTo><a:pt x=\"%.0f\" y=\"%.0f\" /></a:lnTo>",
				p2e_(x[i] - minx), p2e_(y[i] - miny));
	}
	//fprintf(pd->dmlFilePointer, "<a:close/>");

	fputs( "</a:path>", pd->dmlFilePointer );
	fputs( "</a:pathLst>", pd->dmlFilePointer );
	fputs( "</a:custGeom>", pd->dmlFilePointer );

	DML_SetLineSpec(dev, gc);
	fputs( "</wps:spPr>", pd->dmlFilePointer );

	fputs( "<wps:bodyPr />", pd->dmlFilePointer );
	fputs(docx_elt_tag_end, pd->dmlFilePointer );
//	fprintf(pd->dmlFilePointer, "\n");
	fflush(pd->dmlFilePointer);

}

static void DOCX_Polygon(int n, double *x, double *y, const pGEcontext gc,
		pDevDesc dev) {

	DOCDesc *pd = (DOCDesc *) dev->deviceSpecific;
	int idx = get_and_increment_idx(dev);
	int i;
	double maxx = 0, maxy = 0;
	for (i = 0; i < n; i++) {
		if (x[i] > maxx)
			maxx = x[i];
		if (y[i] > maxy)
			maxy = y[i];
	}
	double minx = maxx, miny = maxy;

	for (i = 0; i < n; i++) {
		if (x[i] < minx)
			minx = x[i];
		if (y[i] < miny)
			miny = y[i];
	}

	DOC_ClipLine(minx, miny, maxx, maxy, dev);
	minx = pd->clippedx0;miny = pd->clippedy0;
	maxx = pd->clippedx1;maxy = pd->clippedy1;

	for (i = 1; i < n; i++) {
		DOC_ClipLine(x[i-1], y[i-1], x[i], y[i], dev);
		if( i < 2 ){
			x[i-1] = pd->clippedx0;
			y[i-1] = pd->clippedy0;
		}
		x[i] = pd->clippedx1;
		y[i] = pd->clippedy1;
	}

	fputs(docx_elt_tag_start, pd->dmlFilePointer );
	if( pd->editable < 1 )
		fprintf(pd->dmlFilePointer,
			"<wps:cNvPr id=\"%d\" name=\"Polygon %d\" />%s", idx,	idx, docx_lock_properties);
	else fprintf(pd->dmlFilePointer,
			"<wps:cNvPr id=\"%d\" name=\"Polygon %d\" />%s", idx,	idx, docx_unlock_properties);

	fputs( "<wps:spPr>", pd->dmlFilePointer );
	fputs( "<a:xfrm>", pd->dmlFilePointer );

	fprintf(pd->dmlFilePointer, "<a:off x=\"%.0f\" y=\"%.0f\"/>",
			p2e_(pd->offx + minx), p2e_(pd->offy + miny));
	fprintf(pd->dmlFilePointer, "<a:ext cx=\"%.0f\" cy=\"%.0f\"/>",
			p2e_(maxx - minx), p2e_(maxy - miny));

	fputs( "</a:xfrm>", pd->dmlFilePointer );
	fputs( "<a:custGeom>", pd->dmlFilePointer );
	fputs( "<a:pathLst>", pd->dmlFilePointer );
	fprintf(pd->dmlFilePointer, "<a:path w=\"%.0f\" h=\"%.0f\">", p2e_(maxx-minx), p2e_(maxy-miny));
	fprintf(pd->dmlFilePointer,
			"<a:moveTo><a:pt x=\"%.0f\" y=\"%.0f\" /></a:moveTo>",
			p2e_(x[0] - minx), p2e_(y[0] - miny));

	for (i = 1; i < n; i++) {
		fprintf(pd->dmlFilePointer,
				"<a:lnTo><a:pt x=\"%.0f\" y=\"%.0f\" /></a:lnTo>",
				p2e_(x[i] - minx), p2e_(y[i] - miny));
	}
	fputs( "<a:close/>", pd->dmlFilePointer );

	fputs( "</a:path>", pd->dmlFilePointer );
	fputs( "</a:pathLst>", pd->dmlFilePointer );
	fputs( "</a:custGeom>", pd->dmlFilePointer );

	DML_SetFillColor(dev, gc);
	DML_SetLineSpec(dev, gc);
	fputs( "</wps:spPr>", pd->dmlFilePointer );

	fputs( "<wps:bodyPr />", pd->dmlFilePointer );
	fputs(docx_elt_tag_end, pd->dmlFilePointer );
	fflush(pd->dmlFilePointer);

}

static void DOCX_Rect(double x0, double y0, double x1, double y1,
		const pGEcontext gc, pDevDesc dev) {
	double tmp;
	DOCDesc *pd = (DOCDesc *) dev->deviceSpecific;
	int idx = get_and_increment_idx(dev);

	if (x0 >= x1) {
		tmp = x0;
		x0 = x1;
		x1 = tmp;
	}

	if (y0 >= y1) {
		tmp = y0;
		y0 = y1;
		y1 = tmp;
	}

	DOC_ClipRect(x0, y0, x1, y1, dev);
	x0 = pd->clippedx0;y0 = pd->clippedy0;
	x1 = pd->clippedx1;y1 = pd->clippedy1;


	fputs(docx_elt_tag_start, pd->dmlFilePointer );
	if( pd->editable < 1 )
		fprintf(pd->dmlFilePointer,
			"<wps:cNvPr id=\"%d\" name=\"Rectangle %d\" />%s", idx,	idx, docx_lock_properties);
	else fprintf(pd->dmlFilePointer,
			"<wps:cNvPr id=\"%d\" name=\"Rectangle %d\" />%s", idx,	idx, docx_unlock_properties);
	fputs("<wps:spPr>", pd->dmlFilePointer );
	fputs("<a:xfrm>", pd->dmlFilePointer );

	fprintf(pd->dmlFilePointer, "<a:off x=\"%.0f\" y=\"%.0f\"/>",
			p2e_(pd->offx + x0), p2e_(pd->offy + y0));
	fprintf(pd->dmlFilePointer, "<a:ext cx=\"%.0f\" cy=\"%.0f\"/>",
			p2e_(x1 - x0), p2e_(y1 - y0));
	fputs("</a:xfrm>", pd->dmlFilePointer );
	fputs("<a:prstGeom prst=\"rect\"><a:avLst /></a:prstGeom>", pd->dmlFilePointer );
	DML_SetFillColor(dev, gc);
	DML_SetLineSpec(dev, gc);
	fputs("</wps:spPr>", pd->dmlFilePointer );

	fputs("<wps:bodyPr />", pd->dmlFilePointer );
	fputs(docx_elt_tag_end, pd->dmlFilePointer );
//	fprintf(pd->dmlFilePointer, "\n");
	fflush(pd->dmlFilePointer);

	//return;

}


void docx_text(const char *str, DOCDesc *pd){
	unsigned char *p;
	p = (unsigned char *) str;
	int val, val1, val2, val3, val4;
	while(*p){
		val = *(p++);
		if( val < 128 ){ /* ASCII */

			switch(val) {
				case '<':
					fprintf(pd->dmlFilePointer, "&lt;");
					break;
				case '>':
					fprintf(pd->dmlFilePointer, "&gt;");
					break;
				case '&':
					fprintf(pd->dmlFilePointer, "&amp;");
					break;
				default:
					fprintf(pd->dmlFilePointer, "%c", val);
					break;
			}
		} else if( val > 240 ){ /* 4 octets*/
			val1 = (val - 240) * 65536;
			val = *(p++);
			val2 = (val - 128) * 4096;
			val = *(p++);
			val3 = (val - 128) * 64;
			val = *(p++);
			val4 = val - 128;
			val = val1 + val2 + val3 + val4;
			fprintf(pd->dmlFilePointer, "&#%d;", val);
		} else {
			if( val >= 224 ){ /* 3 octets : 224 = 128+64+32 */
				val1 = (val - 224) * 4096;
				val = *(p++);
				val2 = (val-128) * 64;
				val = *(p++);
				val3 = (val-128);
				val = val1 + val2 + val3;
				fprintf(pd->dmlFilePointer, "&#%d;", val);
			} else { /* 2 octets : >192 = 128+64 */
				val1 = (val - 192) * 64;
				val = *(p++);
				val2 = val-128;
				val = val1 + val2;
				fprintf(pd->dmlFilePointer, "&#%d;", val);
			}

		}
	}
}



static void DOCX_TextUTF8(double x, double y, const char *str, double rot,
		double hadj, const pGEcontext gc, pDevDesc dev) {

	DOCDesc *pd = (DOCDesc *) dev->deviceSpecific;
	int idx = get_and_increment_idx(dev);

	double w = DOCX_StrWidthUTF8(str, gc, dev);
	w = getStrWidth( str, w);
	double h = pd->fi->height[getFontface(gc->fontface)];
	double fs = getFontSize(gc->cex, gc->ps );
	if( h < 1.0 ) return;

	double corrected_offx = translate_rotate_x(x, y, rot, h, w, hadj) ;
	double corrected_offy = translate_rotate_y(x, y, rot, h, w, hadj) ;


	fputs(docx_elt_tag_start, pd->dmlFilePointer );
	if( pd->editable < 1 )
		fprintf(pd->dmlFilePointer, "<wps:cNvPr id=\"%d\" name=\"Text %d\" />%s", idx,	idx, docx_lock_properties);
	else fprintf(pd->dmlFilePointer, "<wps:cNvPr id=\"%d\" name=\"Text %d\" />%s", idx,	idx, docx_unlock_properties);

	fputs("<wps:spPr>", pd->dmlFilePointer );
	if( fabs( rot ) < 1 )
		fputs("<a:xfrm>", pd->dmlFilePointer);
	else fprintf(pd->dmlFilePointer, "<a:xfrm rot=\"%.0f\">", (-rot) * 60000);
	fprintf(pd->dmlFilePointer, "<a:off x=\"%.0f\" y=\"%.0f\"/>", p2e_(pd->offx + corrected_offx), p2e_(pd->offy + corrected_offy));
	fprintf(pd->dmlFilePointer, "<a:ext cx=\"%.0f\" cy=\"%.0f\"/>", p2e_(w), p2e_(h));
	fputs("</a:xfrm>", pd->dmlFilePointer );
	fputs("<a:prstGeom prst=\"rect\"><a:avLst /></a:prstGeom>", pd->dmlFilePointer );
	fputs("<a:noFill />", pd->dmlFilePointer );
	fputs("</wps:spPr>", pd->dmlFilePointer );
	fputs("<wps:txbx>", pd->dmlFilePointer );

	fputs("<w:txbxContent>", pd->dmlFilePointer );
	fputs("<w:p>", pd->dmlFilePointer );
	fputs("<w:pPr>", pd->dmlFilePointer );

	if (hadj < 0.25)
		fputs("<w:jc w:val=\"left\" />", pd->dmlFilePointer );
	else if (hadj < 0.75)
		fputs("<w:jc w:val=\"center\" />", pd->dmlFilePointer );
	else
		fputs("<w:jc w:val=\"right\" />", pd->dmlFilePointer );

	fprintf(pd->dmlFilePointer, "<w:spacing w:after=\"0\" w:before=\"0\" w:line=\"%.0f\" w:lineRule=\"exact\" />", h*20);
	DOCX_setRunProperties( dev, gc, h);
	fputs("</w:pPr>", pd->dmlFilePointer );
	fputs("<w:r>", pd->dmlFilePointer );

	DOCX_setRunProperties( dev, gc, fs);

	fputs("<w:t>", pd->dmlFilePointer );
	docx_text(str, pd);
	fputs("</w:t>", pd->dmlFilePointer );

	fputs("</w:r></w:p>", pd->dmlFilePointer );

	fputs("</w:txbxContent>", pd->dmlFilePointer );
	fputs("</wps:txbx>", pd->dmlFilePointer );
	fputs("<wps:bodyPr lIns=\"0\" tIns=\"0\" rIns=\"0\" bIns=\"0\" anchor=\"b\">", pd->dmlFilePointer );

	fputs( "<a:spAutoFit /></wps:bodyPr>", pd->dmlFilePointer);
	fputs(docx_elt_tag_end, pd->dmlFilePointer );
	fflush(pd->dmlFilePointer);

}
static void DOCX_Text(double x, double y, const char *str, double rot,
		double hadj, const pGEcontext gc, pDevDesc dev) {

	DOCDesc *pd = (DOCDesc *) dev->deviceSpecific;
	int idx = get_and_increment_idx(dev);

	double w = DOCX_StrWidth(str, gc, dev);
	w = getStrWidth( str, w);
	double h = pd->fi->height[getFontface(gc->fontface)];
	double fs = getFontSize(gc->cex, gc->ps );
	if( h < 1.0 ) return;

	double corrected_offx = translate_rotate_x(x, y, rot, h, w, hadj) ;
	double corrected_offy = translate_rotate_y(x, y, rot, h, w, hadj) ;


	fputs(docx_elt_tag_start, pd->dmlFilePointer );
	if( pd->editable < 1 )
		fprintf(pd->dmlFilePointer, "<wps:cNvPr id=\"%d\" name=\"Text %d\" />%s", idx,	idx, docx_lock_properties);
	else fprintf(pd->dmlFilePointer, "<wps:cNvPr id=\"%d\" name=\"Text %d\" />%s", idx,	idx, docx_unlock_properties);

	fputs("<wps:spPr>", pd->dmlFilePointer );
	if( fabs( rot ) < 1 )
		fputs("<a:xfrm>", pd->dmlFilePointer);
	else fprintf(pd->dmlFilePointer, "<a:xfrm rot=\"%.0f\">", (-rot) * 60000);
	fprintf(pd->dmlFilePointer, "<a:off x=\"%.0f\" y=\"%.0f\"/>", p2e_(pd->offx + corrected_offx), p2e_(pd->offy + corrected_offy));
	fprintf(pd->dmlFilePointer, "<a:ext cx=\"%.0f\" cy=\"%.0f\"/>", p2e_(w), p2e_(h));
	fputs("</a:xfrm>", pd->dmlFilePointer );
	fputs("<a:prstGeom prst=\"rect\"><a:avLst /></a:prstGeom>", pd->dmlFilePointer );
	fputs("<a:noFill />", pd->dmlFilePointer );
	fputs("</wps:spPr>", pd->dmlFilePointer );
	fputs("<wps:txbx>", pd->dmlFilePointer );

	fputs("<w:txbxContent>", pd->dmlFilePointer );
	fputs("<w:p>", pd->dmlFilePointer );
	fputs("<w:pPr>", pd->dmlFilePointer );

	if (hadj < 0.25)
		fputs("<w:jc w:val=\"left\" />", pd->dmlFilePointer );
	else if (hadj < 0.75)
		fputs("<w:jc w:val=\"center\" />", pd->dmlFilePointer );
	else
		fputs("<w:jc w:val=\"right\" />", pd->dmlFilePointer );

	fprintf(pd->dmlFilePointer, "<w:spacing w:after=\"0\" w:before=\"0\" w:line=\"%.0f\" w:lineRule=\"exact\" />", h*20);
	DOCX_setRunProperties( dev, gc, h);
	fputs("</w:pPr>", pd->dmlFilePointer );
	fputs("<w:r>", pd->dmlFilePointer );

	DOCX_setRunProperties( dev, gc, fs);

	fputs("<w:t>", pd->dmlFilePointer );
	dml_text_native(str, pd);
	fputs("</w:t>", pd->dmlFilePointer );

	fputs("</w:r></w:p>", pd->dmlFilePointer );

	fputs("</w:txbxContent>", pd->dmlFilePointer );
	fputs("</wps:txbx>", pd->dmlFilePointer );
	fputs("<wps:bodyPr lIns=\"0\" tIns=\"0\" rIns=\"0\" bIns=\"0\" anchor=\"b\">", pd->dmlFilePointer );

	fputs( "<a:spAutoFit /></wps:bodyPr>", pd->dmlFilePointer);
	fputs(docx_elt_tag_end, pd->dmlFilePointer );
	fflush(pd->dmlFilePointer);

}
static void DOCX_NewPage(const pGEcontext gc, pDevDesc dev) {
	DOCDesc *pd = (DOCDesc *) dev->deviceSpecific;
	if (pd->pageNumber > 0) {
		closeFile(pd->dmlFilePointer);
	}

	int which = pd->pageNumber % pd->maxplot;
	pd->pageNumber++;

	//update_start_id(dev);
	dev->right = pd->width[which];
	dev->bottom = pd->height[which];
	dev->left = 0;
	dev->top = 0;

	dev->clipLeft = 0;
	dev->clipRight = dev->right;
	dev->clipBottom = dev->bottom;
	dev->clipTop = 0;

	pd->clippedx0 = dev->clipLeft;
	pd->clippedy0 = dev->clipTop;
	pd->clippedx1 = dev->clipRight;
	pd->clippedy1 = dev->clipBottom;

	pd->offx = pd->x[which];
	pd->offy = pd->y[which];
	pd->extx = pd->width[which];
	pd->exty = pd->height[which];

	char *str={0};
	str = get_dml_filename(pd->filename, pd->pageNumber);
	pd->dmlFilePointer = (FILE *) fopen(str, "w");

	if (pd->dmlFilePointer == NULL) {
		Rf_error("error while opening %s\n", str);
	}
	updateFontInfo(dev, gc);
	free(str);
}
static void DOCX_Close(pDevDesc dev) {
	DOCDesc *pd = (DOCDesc *) dev->deviceSpecific;
	//update_start_id(dev);
	closeFile(pd->dmlFilePointer);
	free(pd);
}

static void DOCX_Clip(double x0, double x1, double y0, double y1, pDevDesc dev) {
	dev->clipLeft = x0;
	dev->clipRight = x1;
	dev->clipBottom = y1;
	dev->clipTop = y0;
}

static void DOCX_MetricInfo(int c, const pGEcontext gc, double* ascent,
		double* descent, double* width, pDevDesc dev) {
	DOC_MetricInfo(c, gc, ascent, descent, width, dev);
}

static void DOCX_Size(double *left, double *right, double *bottom, double *top,
		pDevDesc dev) {
	*left = dev->left;
	*right = dev->right;
	*bottom = dev->bottom;
	*top = dev->top;
}



static double DOCX_StrWidthUTF8(const char *str, const pGEcontext gc, pDevDesc dev) {
	return DOC_StrWidthUTF8(str, gc, dev);
}

static double DOCX_StrWidth(const char *str, const pGEcontext gc, pDevDesc dev) {
	return DOC_StrWidth(str, gc, dev);
}


SEXP R_DOCX_Device(SEXP filename
		, SEXP width, SEXP height, SEXP offx,
		SEXP offy, SEXP pointsize, SEXP fontfamily, SEXP start_id, SEXP is_editable ) {

	double* w = REAL(width);
	double* h = REAL(height);

	double* x = REAL(offx);
	double* y = REAL(offy);
	int nx = length(width);

	double ps = asReal(pointsize);
	int id_init_value = INTEGER(start_id)[0];
	int editable = INTEGER(is_editable)[0];
	BEGIN_SUSPEND_INTERRUPTS;
	GE_DOCXDevice(CHAR(STRING_ELT(filename, 0))
			, w, h, x, y, ps, nx, CHAR(STRING_ELT(fontfamily, 0))
			, id_init_value, editable
			);
	END_SUSPEND_INTERRUPTS;
	return R_NilValue;
}


