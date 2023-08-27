//
//  THIS PROGRAM IS A PARSER FOR "d" SHAPE DEFINITIONS IN SVG PATHS 
//

/*
 * Copyright (c) 2013-14 Mikko Mononen memon@inside.org
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 * claim that you wrote the original software. If you use this software
 * in a product, an acknowledgment in the product documentation would be
 * appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 * misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 * The SVG parser is based on Anti-Grain Geometry 2.4 SVG example
 * Copyright (C) 2002-2004 Maxim Shemanarev (McSeem) (http://www.antigrain.com/)
 *
 * Arc calculation code based on canvg (https://code.google.com/p/canvg/)
 *
 * Bounding box calculation based on http://blog.hackers-cafe.net/2009/06/how-to-calculate-bezier-curves-bounding.html
 *
 */

/*
 * Copyright (c) 2023 Nilo Stolte https://github.com/nilostolte
 *
 * This is an altered and more compact version of the software above that is suplied 
 * with the same permissions and restrictions of the original work of Mikko Mononen. 
 * The original work can be dowloaded from:
 * https://raw.githubusercontent.com/memononen/nanosvg/master/src/nanosvg.h
 * 
 * The original functions and data structures are generally preceeded by
 * "nsvg__" prefix. These functions are identical to the original ones, except
 * those with first "{" in the same line as the name of the function. These functions
 * were only slightly modified.
 * 
 * Functions supplied under the label of "Generic functions" had all their names
 * changed for convenience. Some functions there are new.
 * 
 * This version *only* parses "d" shape definitions in SVG paths and store 
 * them in a new internal format. The original work represented all elements 
 * as cubic bezier patches. The modified version supplied here stores the elements 
 * in their original format except for HLINETO and VLINETO. Because all elements  
 * are systematically modified by a transformation matrix, elements containing only
 * one coordinate such as these cannot be rotated. These elements are represented  
 * internally by a LINETO with two coordinates. A similar problem occurs with ARCTO 
 * commands. To rotate an ARCTO command the rotation angle is added to the angle of the
 * ARCTO, while the last point is multipled by the matrix.
 * 
 * The function getRotationAngleFromMatrix can be used to get this angle from the matrix.
 *
 * Finally, command line parameters were implemented to supply a matrix (-m) or to
 * instruct the parser to generate relative coordinates (-r) instead of the default 
 * absolute coordinates (the internal representation), as well as a path to be parsed.
 *
 * The motivation of this work is to convert any path to absolute coordinates,
 * which gives the opportunity to work with them by hand, and afterwards reconvert
 * them to relative coordinates to make it easier to place them by hand. 
 */

#ifndef NULL
#define NULL 0
#endif

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#include <float.h>

//#define DEBUG       // when defined it print traces
//#define VERBOSE   // when not defined it uses only 3 digits after the decimal point
#define NSVG_INLINE inline

#define NSVG_EPSILON (1e-12)
#define NSVG_PI (3.14159265358979323846264338327f)

/* Internal codes for commands - they hide the number of arguments in the 2nd byte*/
#define LINETO      (2 << 8)
#define TQUADTO     (LINETO + 1)    // T command, smooth quadto
#define QUADTO      (((TQUADTO & 0xff) + 1) | (4 << 8))
#define SCURVETO    (QUADTO + 1)    // S command, smooth curveto
#define CURVETO		(((SCURVETO & 0xff) + 1) | (6 << 8))
#define ARCTO       (((CURVETO & 0xff) + 1) | (7 << 8))
#define NCOMMANDS 	((ARCTO & 0xff) + 1) // the number of commands above

// macro to convert a float pointer to a 32 bits integer pointer
#define command(p) ((int32_t *)(p))       
// macro to round and trucate at 3 digits after the decimal point 
#define dig3(n) (roundf((n)*1000.0f)/1000.0f)


#ifdef VERBOSE
#define trnc(a) (a)
char *L_format = "L%f,%f ";
char *H_format = "H%f ";
char *V_format = "V%f ";
char *l_format = "l%f,%f ";
char *h_format = "h%f ";
char *v_format = "v%f ";
#else
#define trnc(a) dig3(a)
char *L_format = "L%g,%g ";
char *H_format = "H%g ";
char *V_format = "V%g ";
char *l_format = "l%g,%g ";
char *h_format = "h%g ";
char *v_format = "v%g ";
#endif

#define ITEM_SIZE 64

/* Types */

//  SVGpath is the node of a linked list of subpaths that are part of the same path.
//  The subpath corresponding to this node is stored in the array of float elements.
//	These elements are either point coordinates, either a command represented by a
//  32 bits integer, either other parameters of an arcto command. The size of this 
//  array is stored in variable size. Elements are to be treated in groups strting
//  with a leading command. The elements have these formats:
//
//  First 2 elements: x, y coordinates of the imitial moveto (moveto commands are omitted)
//
//  +----------------------+------------------------------------------+------------ +
//  |  Command             |   following elements                     | SVG command |
//  +----------------------+------------------------------------------+------------ +
//  |  "moveto" (omitted)  |   x, y  (first 2 elements - no command)  |      M      |
//  |  LINETO              |   x, y                                   |      L      |
//  |  TQUADTO             |   x, y                                   |      T      |
//  |  QUADTO              |   x1, y1, x, y                           |      Q      |
//  |  SCURVETO            |   x1, y1, x, y                           |      S      |
//	|  CURVETO             |   x1, y1, x2, y2, x, y                   |      C      |
//  |  ARCTO               |   rx, ry, angle, 1.0/0.0, 1.0/0.0, x, y  |      A      |
//  +----------------------+------------------------------------------+-------------+
//
typedef struct SVGpath {
	float* elements;        // elements array (see description above)
	int size;               // Total number elenyss.
	char closed;            // Flag indicating if shapes should be treated as closed.
	struct SVGpath* next;   // Pointer to next path, or NULL if last element.
} SVGpath;

// Data Structure used by the parser - plist is the linked list of all parsed subpaths
typedef struct SVGPathparser {
	float* matrix;          // tranformation matrix, systematically multiplied by every pt
	float* elements;        // elements Arraylist (parsed commands are temporarily stored here)
	int size;               // size of space used in Arraylist
	int capacity;           // capacity of elements Arraylist (increases when needed)
	float angle;            // angle corresponding to the rotation in the matrix
	SVGpath* plist;         // once parsed, elements are copied to elements in a new SVGpath
	                        // which becomes head of this linked list
} SVGPathparser;

// Table to be indexed like sz[c-'A'], where c is any letter in the alphabet. sz[c-'A'] either 
// returns c or the number of arguments of the SVG command represented by the letter in c
static int sz[58] = {
	7,   'B', 6,   'D', 'E', 'F', 'G', 1,   'I', 'J',
	'K', 2,   2,   'N', 'O', 'P', 4,   'R', 4,   2,
	'U', 1,   'W', 'X', 'Y', 0,   '[','\\', ']', '^',
	'_', '`', 7,   'b', 6,   'd', 'e', 'f', 'g', 1,
	'i', 'j', 'k', 2,   2,   'n', 'o', 'p', 4,   'r',
	4,   2,   'u', 1,   'w', 'x', 'y', 0
};

// commands[<command> && 0xff] and relative[<command> && 0xff] give the
// SVG letter command given <command> = LINETO|TQUADTO|QUADTO|SCURVETO|CURVETO|ARCTO
static char commands[NCOMMANDS] = { 'L', 'T', 'Q', 'S', 'C', 'A' };
static char relative[NCOMMANDS] = { 'l', 't', 'q', 's', 'c', 'a' };

/* Generic functions  */

static float sqr(float x) { return x*x; }
static float norm(float x, float y) { return sqrtf(x*x + y*y); }
static double bezierCoordinate(double t, double p0, double p1, double p2, double p3) {
	double it = 1.0-t;
	return it*it*it*p0 + 3.0*it*it*t*p1 + 3.0*it*t*t*p2 + t*t*t*p3;
}


static int digit(char c) {
	return c >= '0' && c <= '9';
}

static double str2f(const char* s) {
	char* cur = (char*)s;
	char* end = NULL;
	double res = 0.0, sign = 1.0;
	long long intPart = 0, fracPart = 0;
	char hasIntPart = 0, hasFracPart = 0;

	// Parse optional sign
	if (*cur == '+') {
		cur++;
	} else if (*cur == '-') {
		sign = -1;
		cur++;
	}

	// Parse integer part
	if (digit(*cur)) {
		// Parse digit sequence
		intPart = strtoll(cur, &end, 10);
		if (cur != end) {
			res = (double)intPart;
			hasIntPart = 1;
			cur = end;
		}
	}

	// Parse fractional part.
	if (*cur == '.') {
		cur++; // Skip '.'
		if (digit(*cur)) {
			// Parse digit sequence
			fracPart = strtoll(cur, &end, 10);
			if (cur != end) {
				res += (double)fracPart / pow(10.0, (double)(end - cur));
				hasFracPart = 1;
				cur = end;
			}
		}
	}

	// A valid number should have integer or fractional part.
	if (!hasIntPart && !hasFracPart)
		return 0.0;

	// Parse optional exponent
	if (*cur == 'e' || *cur == 'E') {
		long expPart = 0;
		cur++; // skip 'E'
		expPart = strtol(cur, &end, 10); // Parse digit sequence with sign
		if (cur != end) {
			res *= pow(10.0, (double)expPart);
		}
	}

	return res * sign;
}

static char* parseNumber(char* s, char* it, const int size)
{
	const int last = size-1;
	int i = 0;

	// sign
	if (*s == '-' || *s == '+') {
		if (i < last) it[i++] = *s;
		s++;
	}
	// integer part
	while (*s && digit(*s)) {
		if (i < last) it[i++] = *s;
		s++;
	}
	if (*s == '.') {
		// decimal point
		if (i < last) it[i++] = *s;
		s++;
		// fraction part
		while (*s && digit(*s)) {
			if (i < last) it[i++] = *s;
			s++;
		}
	}
	// exponent
	if ((*s == 'e' || *s == 'E') && (s[1] != 'm' && s[1] != 'x')) {
		if (i < last) it[i++] = *s;
		s++;
		if (*s == '-' || *s == '+') {
			if (i < last) it[i++] = *s;
			s++;
		}
		while (*s && digit(*s)) {
			if (i < last) it[i++] = *s;
			s++;
		}
	}
	it[i] = '\0';

	return s;
}

static void copyMatrix(float* t, float* m) {
	t[0] = m[0]; t[1] = m[1];
	t[2] = m[2]; t[3] = m[3];
	t[4] = m[4]; t[5] = m[5];
}

static void identityMatrix(float* t) {
	t[0] = 1.0f; t[1] = 0.0f;
	t[2] = 0.0f; t[3] = 1.0f;
	t[4] = 0.0f; t[5] = 0.0f;
}

static void setTranslationInMatrix(float* t, float tx, float ty) {
	t[0] = 1.0f; t[1] = 0.0f;
	t[2] = 0.0f; t[3] = 1.0f;
	t[4] = tx; t[5] = ty;
}

static void setScaleInMatrix(float* t, float sx, float sy){
	t[0] = sx; t[1] = 0.0f;
	t[2] = 0.0f; t[3] = sy;
	t[4] = 0.0f; t[5] = 0.0f;
}

static void setSkewXInMatrix(float* t, float a){
	t[0] = 1.0f; t[1] = 0.0f;
	t[2] = tanf(a); t[3] = 1.0f;
	t[4] = 0.0f; t[5] = 0.0f;
}

static void setSkewYInMatrix(float* t, float a){
	t[0] = 1.0f; t[1] = tanf(a);
	t[2] = 0.0f; t[3] = 1.0f;
	t[4] = 0.0f; t[5] = 0.0f;
}

static void setRotation(float*t, float cs, float sn) {
	t[0] = cs; t[1] = sn;
	t[2] = -sn; t[3] = cs;
	t[4] = 0.0f; t[5] = 0.0f;
}

static void setRotationInMatrix(float* t, float a){
	if (a == 0.0f) { identityMatrix(t); return; }
	if (a == 90.0f) { setRotation(t, 0.0f, 1.0f); return; }
	if (a == 180.0f) { setRotation(t, -1.0f, 0.0f); return; }
	if (a == 270.0f) { setRotation(t, 0.0f, -1.0f); return; }
	setRotation(t, cosf(a), sinf(a));
}

static void matrixMultiply(float* t, float* s){
	float t0 = t[0] * s[0] + t[1] * s[2];
	float t2 = t[2] * s[0] + t[3] * s[2];
	float t4 = t[4] * s[0] + t[5] * s[2] + s[4];
	t[1] = t[0] * s[1] + t[1] * s[3];
	t[3] = t[2] * s[1] + t[3] * s[3];
	t[5] = t[4] * s[1] + t[5] * s[3] + s[5];
	t[0] = t0;
	t[2] = t2;
	t[4] = t4;
}

static void matrixInverse(float* inv, float* t){
	double invdet, det = (double)t[0] * t[3] - (double)t[2] * t[1];
	if (det > -1e-6 && det < 1e-6) {
		identityMatrix(t);
		return;
	}
	invdet = 1.0 / det;
	inv[0] = (float)(t[3] * invdet);
	inv[2] = (float)(-t[2] * invdet);
	inv[4] = (float)(((double)t[2] * t[5] - (double)t[3] * t[4]) * invdet);
	inv[1] = (float)(-t[1] * invdet);
	inv[3] = (float)(t[0] * invdet);
	inv[5] = (float)(((double)t[1] * t[4] - (double)t[0] * t[5]) * invdet);
}

static void matrixPremultiply(float* t, float* s){
	float s2[6];
	memcpy(s2, s, sizeof(float)*6);
	matrixMultiply(s2, t);
	memcpy(t, s2, sizeof(float)*6);
}

static void pointMatrixMultiply(float* dx, float* dy, float x, float y, float* t){
	*dx = x*t[0] + y*t[2] + t[4];
	*dy = x*t[1] + y*t[3] + t[5];
}

static void vectorMatrixMultiply(float* dx, float* dy, float x, float y, float* t) {
	*dx = x*t[0] + y*t[2];
	*dy = x*t[1] + y*t[3];
}

static float getRotationAngleFromMatrix(float* t) {
	return(atan2(t[1], t[0])*180.0f/NSVG_PI);
}

/* end generic fuctions */

static NSVG_INLINE float nsvg__minf(float a, float b) { return a < b ? a : b; }
static NSVG_INLINE float nsvg__maxf(float a, float b) { return a > b ? a : b; }

/* parser functions */

static void nsvg__resetPath(SVGPathparser* p)
{
	p->size = 0;
}

static int space(char c)
{
	return strchr(" \t\n\v\f\r", c) != 0;
}

static int coordinate(const char* s)
{
	// optional sign
	if (*s == '-' || *s == '+')
		s++;
	// must have at least one digit, or start by a dot
	return (digit(*s) || *s == '.');
}

static int numberOfArguments(int cmd){
	int i = cmd - 65;
	int n;
	if (i < 0 || i > 57 || ((n = sz[i]) == cmd) ) return -1;
	return n;
}


static float* guarantee_capacity(SVGPathparser* p, int n){
	if (p->size+n > p->capacity) {
		p->capacity = p->capacity ? (p->capacity<<1) : 32;
		p->elements = (float*)realloc(p->elements, p->capacity*sizeof(float));
	}
	return p->elements;
}

static void moveto(SVGPathparser* p, float x, float y){
	if (p->size > 0) {
		printf("**Error: unexpected moveTo with size of elements not zero\n");
		exit(1);
	}
	if (!guarantee_capacity(p,2)) return;
	p->elements[0] = x;
	p->elements[1] = y;
	p->size+=2;
}


static void lineto(SVGPathparser* p, float x, float y) {
	if (p->size > 0) {
		if (!guarantee_capacity(p,3)) return;
		*command(p->elements+p->size) = LINETO;
		p->elements[p->size+1] = x;
		p->elements[p->size+2] = y;
		p->size+=3;
	}
}

static void curveto(SVGPathparser* p, float cpx1, float cpy1, float cpx2, float cpy2, float x, float y) {
	if (p->size > 0) {
		if (!guarantee_capacity(p,7)) return;
		*command(p->elements+p->size) = CURVETO;
		p->elements[p->size+1] = cpx1;
		p->elements[p->size+2] = cpy1;
		p->elements[p->size+3] = cpx2;
		p->elements[p->size+4] = cpy2;
		p->elements[p->size+5] = x;
		p->elements[p->size+6] = y;
		p->size+=7;
#ifdef DEBUG		
		printf("curveto: %f %f %f %f %f %f\n",  cpx1, cpy1, cpx2, cpy2, x, y);
#endif
	}
}

static void scurveto(SVGPathparser* p, float cpx2, float cpy2, float x, float y) {
	if (p->size > 0) {
		if (!guarantee_capacity(p,5)) return;
		*command(p->elements+p->size) = SCURVETO;
		p->elements[p->size+1] = cpx2;
		p->elements[p->size+2] = cpy2;
		p->elements[p->size+3] = x;
		p->elements[p->size+4] = y;
		p->size+=5;
#ifdef DEBUG
		printf("scurveto: %f %f %f %f\n", cpx2, cpy2, x, y);
#endif
	}
}

static void quadto(SVGPathparser* p, float cpx, float cpy, float x, float y) {
	if (p->size > 0) {
		printf("quadto: %f %f ",  cpx, cpy);
		if (!guarantee_capacity(p,5)) return;
		*command(p->elements+p->size) = QUADTO;
		p->elements[p->size+1] = cpx;
		p->elements[p->size+2] = cpy;
		p->elements[p->size+3] = x;
		p->elements[p->size+4] = y;
		p->size+=5;
#ifdef DEBUG
		printf("%f %f\n", x, y);
#endif
	}
}

static void tquadto(SVGPathparser* p, float x, float y) {
	if (p->size > 0) {
		if (!guarantee_capacity(p,3)) return;
		*command(p->elements+p->size) = TQUADTO;
		p->elements[p->size+1] = x;
		p->elements[p->size+2] = y;
		p->size+=3;
#ifdef DEBUG
		printf("tquadto: %f %f ",  x, y);
#endif
	}
}

static void arcto(SVGPathparser* p, float* args, float x, float y) {
	if (p->size > 0) {
		if (!guarantee_capacity(p,8)) return;
		*command(p->elements+p->size) = ARCTO;
		p->elements[p->size+1] = args[0];
		p->elements[p->size+2] = args[1];
		p->elements[p->size+3] = args[2];
		p->elements[p->size+4] = args[3];
		p->elements[p->size+5] = args[4];
		p->elements[p->size+6] = x;
		p->elements[p->size+7] = y;
		p->size+=8;
#ifdef DEBUG
		printf("arcto: %f %f %f %f %f %f %f\n", args[0],args[1],args[2],args[3],args[4],x, y);
#endif
	}
}

static void nsvg__pathMoveTo(SVGPathparser* p, float* cpx, float* cpy, float* args, int rel) {
	if (rel) {
		*cpx += args[0];
		*cpy += args[1];
	} else {
		*cpx = args[0];
		*cpy = args[1];
	}
	moveto(p, *cpx, *cpy);
#ifdef DEBUG
	printf("moveto: %f %f\n", *cpx, *cpy);
#endif
}

static void nsvg__pathLineTo(SVGPathparser* p, float* cpx, float* cpy, float* args, int rel) {
	if (rel) {
		*cpx += args[0];
		*cpy += args[1];
	} else {
		*cpx = args[0];
		*cpy = args[1];
	}
	lineto(p, *cpx, *cpy);
#ifdef DEBUG
	printf("lineto: %f %f\n", *cpx, *cpy);
#endif
}

static void nsvg__pathHLineTo(SVGPathparser* p, float* cpx, float* cpy, float* args, int rel) {
	if (rel)
		*cpx += args[0];
	else
		*cpx = args[0];
	lineto(p, *cpx, *cpy);
#ifdef DEBUG
	printf("hlineto -> lineto: %f %f\n", *cpx, *cpy);
#endif
}

static void nsvg__pathVLineTo(SVGPathparser* p, float* cpx, float* cpy, float* args, int rel) {
	if (rel)
		*cpy += args[0];
	else
		*cpy = args[0];
	lineto(p, *cpx, *cpy);
#ifdef DEBUG
	printf("vlineto -> lineto: %f %f\n", *cpx, *cpy);
#endif
}

static void nsvg__pathCubicBezTo(SVGPathparser* p, float* cpx, float* cpy,
								 float* cpx2, float* cpy2, float* args, int rel)
{
	float x2, y2, cx1, cy1, cx2, cy2;

	if (rel) {
		cx1 = *cpx + args[0];
		cy1 = *cpy + args[1];
		cx2 = *cpx + args[2];
		cy2 = *cpy + args[3];
		x2 = *cpx + args[4];
		y2 = *cpy + args[5];
	} else {
		cx1 = args[0];
		cy1 = args[1];
		cx2 = args[2];
		cy2 = args[3];
		x2 = args[4];
		y2 = args[5];
	}

	curveto(p, cx1,cy1, cx2,cy2, x2,y2);

	*cpx2 = cx2;
	*cpy2 = cy2;
	*cpx = x2;
	*cpy = y2;
}

static void nsvg__pathCubicBezShortTo(SVGPathparser* p, float* cpx, float* cpy,
									  float* cpx2, float* cpy2, float* args, int rel) {
	float x1, y1, x2, y2,  cx2, cy2;

	x1 = *cpx;
	y1 = *cpy;
	if (rel) {
		cx2 = *cpx + args[0];
		cy2 = *cpy + args[1];
		x2 = *cpx + args[2];
		y2 = *cpy + args[3];
	} else {
		cx2 = args[0];
		cy2 = args[1];
		x2 = args[2];
		y2 = args[3];
	}

	scurveto(p, cx2,cy2, x2,y2);

	*cpx2 = cx2;
	*cpy2 = cy2;
	*cpx = x2;
	*cpy = y2;
}

static void nsvg__pathQuadBezTo(SVGPathparser* p, float* cpx, float* cpy,
								float* cpx2, float* cpy2, float* args, int rel) {
	float x2, y2, cx, cy;

	if (rel) {
		cx = *cpx + args[0];
		cy = *cpy + args[1];
		x2 = *cpx + args[2];
		y2 = *cpy + args[3];
	} else {
		cx = args[0];
		cy = args[1];
		x2 = args[2];
		y2 = args[3];
	}

	quadto(p, cx,cy, x2,y2);

	*cpx2 = cx;
	*cpy2 = cy;
	*cpx = x2;
	*cpy = y2;
}

static void nsvg__pathQuadBezShortTo(SVGPathparser* p, float* cpx, float* cpy,
									 float* cpx2, float* cpy2, float* args, int rel) {
	float x1, y1, x2, y2, cx, cy;
	float cx1, cy1, cx2, cy2;

	x1 = *cpx;
	y1 = *cpy;
	if (rel) {
		x2 = *cpx + args[0];
		y2 = *cpy + args[1];
	} else {
		x2 = args[0];
		y2 = args[1];
	}

	cx = 2*x1 - *cpx2;
	cy = 2*y1 - *cpy2;

	tquadto(p, x2,y2);

	*cpx2 = cx;
	*cpy2 = cy;
	*cpx = x2;
	*cpy = y2;
}

static float nsvg__vecrat(float ux, float uy, float vx, float vy)
{
	return (ux*vx + uy*vy) / (norm(ux,uy) * norm(vx,vy));
}

static float nsvg__vecang(float ux, float uy, float vx, float vy)
{
	float r = nsvg__vecrat(ux,uy, vx,vy);
	if (r < -1.0f) r = -1.0f;
	if (r > 1.0f) r = 1.0f;
	return ((ux*vy < uy*vx) ? -1.0f : 1.0f) * acosf(r);
}

static void nsvg__pathArcTo(SVGPathparser* p, float* cpx, float* cpy, float* args, int rel) {
	float rx, ry;
	float x1, y1, x2, y2, dx, dy, d;

	int fa, fs;


	rx = fabsf(args[0]);				// y radius
	ry = fabsf(args[1]);				// x radius
	x1 = *cpx;							// start point
	y1 = *cpy;
	if (rel) {							// end point
		x2 = *cpx + args[5];
		y2 = *cpy + args[6];
	} else {
		x2 = args[5];
		y2 = args[6];
	}

	dx = x1 - x2;
	dy = y1 - y2;
	d = sqrtf(dx*dx + dy*dy);
	if (d < 1e-6f || rx < 1e-6f || ry < 1e-6f) {
		// The arc degenerates to a line
		lineto(p, x2, y2);
		*cpx = x2;
		*cpy = y2;
		return;
	}
	
	arcto(p, args, x2, y2);
	
	*cpx = x2;
	*cpy = y2;
}


static void nsvg__addPath(SVGPathparser* p, char closed)
{
	SVGpath* path = NULL;
	int i, j;
	int nargs;
	int32_t cmd;
	float xmin, ymin, xmax, ymax;
	if (p->size < 4)
		return;

	if (closed)
		lineto(p, p->elements[0], p->elements[1]);


	path = (SVGpath*)malloc(sizeof(SVGpath));
	if (path == NULL) goto error;
	memset(path, 0, sizeof(SVGpath));

	path->elements = (float*)malloc(p->size*sizeof(float));
	if (path->elements == NULL) goto error;
	path->closed = closed;
	path->size = p->size;

	// Transform path.
	
	pointMatrixMultiply(&path->elements[0], &path->elements[1], p->elements[0], p->elements[1], p->matrix);
	for (i = 2; i < p->size; ) {
		*command(path->elements+i) = cmd = *command(p->elements+i);
		nargs = cmd >> 8;    // get number of argments from command code
		if (nargs < 7) { // for every one except arcs
			for (j = 1; j < nargs; j+=2 ) {
			   pointMatrixMultiply(&path->elements[i+j], &path->elements[i+j+1], p->elements[i+j], p->elements[i+j+1], p->matrix);
               xmin = nsvg__minf(path->elements[i+j],xmin);
			   ymin = nsvg__minf(path->elements[i+j+1],ymin);
			   xmax = nsvg__maxf(path->elements[i+j],xmax);
			   ymax = nsvg__maxf(path->elements[i+j+1],ymax);
			}
			i += j;
			continue;
		}
		// this is an arc - we use the matrix only for the last point
		path->elements[i+1] = p->elements[i+1]; // x radius remains constant
		path->elements[i+2] = p->elements[i+2]; // y radius remains constant
		path->elements[i+3] = p->elements[i+3] + p->angle;	// adds the angle of the transformation matrix
		path->elements[i+4] = p->elements[i+4]; // remains constant
		path->elements[i+5] = p->elements[i+5]; // remains constant
		pointMatrixMultiply(&path->elements[i+6], &path->elements[i+7], p->elements[i+6], p->elements[i+7], p->matrix);
        xmin = nsvg__minf(path->elements[i+6],xmin);
	    ymin = nsvg__minf(path->elements[i+7],ymin);
        xmax = nsvg__maxf(path->elements[i+6],xmax);
        ymax = nsvg__maxf(path->elements[i+7],ymax);
		i += 8;
	}

	path->next = p->plist;
	p->plist = path;
	return;

error:
	if (path != NULL) {
		if (path->elements != NULL) free(path->elements);
		free(path);
	}
	printf("allocation error: addPath\n");
}

static float nsvg__getAverageScale(float* t)
{
	float sx = sqrtf(t[0]*t[0] + t[2]*t[2]);
	float sy = sqrtf(t[1]*t[1] + t[3]*t[3]);
	return (sx + sy) * 0.5f;
}


static char* nsvg__getNextPathItemWhenArcFlag(char* s, char* it)
{
	it[0] = '\0';
	while (*s && (space(*s) || *s == ',')) s++;
	if (!*s) return s;
	if (*s == '0' || *s == '1') {
		it[0] = *s++;
		it[1] = '\0';
		return s;
	}
	return s;
}

static char* nsvg__getNextPathItem(char* s, char* it)
{
	it[0] = '\0';
	// Skip white spaces and commas
	while (*s && (space(*s) || *s == ',')) s++;
	if (!*s) return s;
	if (*s == '-' || *s == '+' || *s == '.' || digit(*s)) {
		s = parseNumber(s, it, ITEM_SIZE);
	} else {
		// Parse command
		it[0] = *s++;
		it[1] = '\0';
		return s;
	}

	return s;
}

static void nsvg__parsePath(SVGPathparser* p, char*s)
{
	char cmd = '\0';
	float args[10];
	int nargs;
	int rargs = 0;
	char initPoint;
	float cpx, cpy, cpx2, cpy2;
	const char* tmp[4];
	char closedFlag;
	int i;
	char item[ITEM_SIZE];
	if (s) {
		nsvg__resetPath(p);
		cpx = 0; cpy = 0;
		cpx2 = 0; cpy2 = 0;
		initPoint = 0;
		closedFlag = 0;
		nargs = 0;
		while (*s) {
			item[0] = '\0';
			if ((cmd == 'A' || cmd == 'a') && (nargs == 3 || nargs == 4))
				s = nsvg__getNextPathItemWhenArcFlag(s, item);
			if (!*item)
				s = nsvg__getNextPathItem(s, item);
			if (!*item) break;
			if (cmd != '\0' && coordinate(item)) {
				if (nargs < 10)
					args[nargs++] = (float)str2f(item);
				if (nargs >= rargs) {
					switch (cmd) {
						case 'm':
						case 'M':
							nsvg__pathMoveTo(p, &cpx, &cpy, args, cmd == 'm' ? 1 : 0);
							// Moveto can be followed by multiple coordinate pairs,
							// which should be treated as linetos.
							cmd = (cmd == 'm') ? 'l' : 'L';
							rargs = numberOfArguments(cmd);
							cpx2 = cpx; cpy2 = cpy;
							initPoint = 1;
							break;
						case 'l':
						case 'L':
							nsvg__pathLineTo(p, &cpx, &cpy, args, cmd == 'l' ? 1 : 0);
							cpx2 = cpx; cpy2 = cpy;
							break;
						case 'H':
						case 'h':
							nsvg__pathHLineTo(p, &cpx, &cpy, args, cmd == 'h' ? 1 : 0);
							cpx2 = cpx; cpy2 = cpy;
							break;
						case 'V':
						case 'v':
							nsvg__pathVLineTo(p, &cpx, &cpy, args, cmd == 'v' ? 1 : 0);
							cpx2 = cpx; cpy2 = cpy;
							break;
						case 'C':
						case 'c':
							nsvg__pathCubicBezTo(p, &cpx, &cpy, &cpx2, &cpy2, args, cmd == 'c' ? 1 : 0);
							break;
						case 'S':
						case 's':
							nsvg__pathCubicBezShortTo(p, &cpx, &cpy, &cpx2, &cpy2, args, cmd == 's' ? 1 : 0);
							break;
						case 'Q':
						case 'q':
							nsvg__pathQuadBezTo(p, &cpx, &cpy, &cpx2, &cpy2, args, cmd == 'q' ? 1 : 0);
							break;
						case 'T':
						case 't':
							nsvg__pathQuadBezShortTo(p, &cpx, &cpy, &cpx2, &cpy2, args, cmd == 't' ? 1 : 0);
							break;
						case 'A':
						case 'a':
							nsvg__pathArcTo(p, &cpx, &cpy, args, cmd == 'a' ? 1 : 0);
							cpx2 = cpx; cpy2 = cpy;
							break;
						default:
							if (nargs >= 2) {
								cpx = args[nargs-2];
								cpy = args[nargs-1];
								cpx2 = cpx; cpy2 = cpy;
							}
							break;
					}

					nargs = 0;
				}
			} else {
				cmd = item[0];
				if (cmd == 'M' || cmd == 'm') {
					// Commit path.
					if (p->size > 0)
						nsvg__addPath(p, closedFlag);
					// Start new subpath.
					nsvg__resetPath(p);
					closedFlag = 0;
					nargs = 0;
				} else if (initPoint == 0) {
					// Do not allow other commands until initial point has been set (moveTo called once).
					cmd = '\0';
				}
				if (cmd == 'Z' || cmd == 'z') {
					closedFlag = 1;
					// Commit path.
					if (p->size > 0) {
						// Move current point to first point
						cpx = p->elements[0];
						cpy = p->elements[1];
						cpx2 = cpx; cpy2 = cpy;
						nsvg__addPath(p, closedFlag);
					}
					// Start new subpath.
					nsvg__resetPath(p);
					moveto(p, cpx, cpy);
					closedFlag = 0;
					nargs = 0;
				}
				rargs = numberOfArguments(cmd);
				if (rargs == -1) {
					// Command not recognized
					cmd = '\0';
					rargs = 0;
				}
			}
		}
		// Commit path.
		if (p->size)
			nsvg__addPath(p, closedFlag);
	}

}

//
//  Prints an SVG path with absolute coordinates
//    Input:  a linked list of SVG subpaths (SVGpath)
//    Output: prints the svg path on the console by using the internal path
//            representation, and substuting linetos by hlinetos or vlinetos
//            when appropriate.
//
static void generateAbsoluteSVG(SVGpath* path) {
	SVGpath* p;
	float* elements;
	int size;
	int i, j, nargs;
	int32_t cmd;
	float cpx, cpy, x, y;
	for( p = path; p; p = p->next){
		elements = p->elements;
		size = p->size;
#ifdef VERBOSE
		printf("M%f,%f ", elements[0], elements[1]);
		cpx = elements[0]; cpy = elements[1];
#else
		printf("M%g,%g ", dig3(elements[0]), dig3(elements[1]));
		cpx = dig3(elements[0]); cpy = dig3(elements[1]);
#endif
		for (i = 2; i < size; ) {
			cmd = *command(p->elements+i);
			if (cmd == LINETO) {
#ifdef VERBOSE
				x = p->elements[i+1]; y = p->elements[i+2]);
#else
				x = dig3(p->elements[i+1]); y = dig3(p->elements[i+2]);
#endif
				if ( x == cpx ) {
				   printf(V_format, y);
				   cpy = y;
				   i+=3;
				   continue;
				}
				if ( y == cpy ) {
				   printf(H_format, x);
				   cpx = x;
				   i+=3;
				   continue;
				}
				printf(L_format, x, y);
				cpx = x; cpy = y;
				i+=3;
				continue;
			}
			printf("%c", commands[cmd & 0xff]);
			nargs = cmd >> 8;    // get number of argments from command code
			if (nargs < 7) { // for every one except arcs
#ifdef VERBOSE
              for (j = 1; j < nargs; j += 2 )
				 printf("%f,%f ", p->elements[i+j], p->elements[i+j+1]);
              i += j;
              cpx = p->elements[i-2]; cpy = p->elements[i-1];
#else
              for (j = 1; j < nargs; j += 2 )
				 printf("%g,%g ", dig3(p->elements[i+j]),dig3(p->elements[i+j+1]));
              i += j;
              cpx = dig3(p->elements[i-2]); cpy = dig3(p->elements[i-1]);
#endif				   		
				continue;
			}
			// this is an arc
#ifdef VERBOSE
			printf("%f,%f %f %f %f %f,%f ", 
				p->elements[i+1], p->elements[i+2], p->elements[i+3], p->elements[i+4], 
				p->elements[i+5], p->elements[i+6], p->elements[i+7]
			);
			cpx = p->elements[i+6]; cpy = p->elements[i+7];
#else
			printf("%g,%g %g %g %g %g,%g ", 
				dig3(p->elements[i+1]), dig3(p->elements[i+2]), dig3(p->elements[i+3]), 
				dig3(p->elements[i+4]), dig3(p->elements[i+5]), dig3(p->elements[i+6]), 
				dig3(p->elements[i+7])
			);
			cpx = dig3(p->elements[i+6]); cpy = dig3(p->elements[i+7]);
#endif		
			i += 8;			
		}
	}
}

static void generateRelativeSVG(SVGpath* path) {
	SVGpath* p;
	float* elements;
	int size;
	int i, j, nargs;
	int32_t cmd;
	float cpx, cpy, x, y;
	for( p = path; p; p = p->next){
		elements = p->elements;
		size = p->size;
#ifdef VERBOSE
		printf("M%f,%f ", elements[0], elements[1]);
#else
		printf("M%g,%g ", dig3(elements[0]), dig3(elements[1]));
#endif
        cpx = elements[0]; cpy = elements[1];
		for (i = 2; i < size; ) {
			cmd = *command(p->elements+i);
			if (cmd == LINETO) {
				x = p->elements[i+1]; y = p->elements[i+2];
				if ( x == cpx ) {
					if ( y != cpy) {
					    printf(v_format, trnc(y - cpy));
				        cpy = y;
					}
				   i+=3;
				   continue;
				}
				if ( y == cpy ) {
					if ( x != cpx ) {
						printf(h_format, trnc(x - cpx));
						cpx = x;
					}
				   i+=3;
				   continue;
				}
				printf(l_format, trnc(x - cpx), trnc(y - cpy));
				cpx = x; cpy = y;
				i+=3;
				continue;
			}
			printf("%c", relative[cmd & 0xff]);
			nargs = cmd >> 8;    // get number of argments from command code
			if (nargs < 7) { // for every one except arcs
#ifdef VERBOSE
              for (j = 1; j < nargs; j += 2 )
				 printf("%f,%f ", p->elements[i+j] - cpx, p->elements[i+j+1] - cpy);
              i += j;
              cpx = p->elements[i-2]; cpy = p->elements[i-1];
#else
              for (j = 1; j < nargs; j += 2 )
				 printf("%g,%g ", dig3(p->elements[i+j] - cpx),dig3(p->elements[i+j+1] - cpy));
              i += j;
              cpx = dig3(p->elements[i-2]); cpy = dig3(p->elements[i-1]);
#endif				   		
				continue;
			}
			// this is an arc
#ifdef VERBOSE
			printf("%f,%f %f %f %f %f,%f ", 
				p->elements[i+1], p->elements[i+2], p->elements[i+3], p->elements[i+4], 
				p->elements[i+5], p->elements[i+6] - cpx, p->elements[i+7] - cpy
			);
			cpx = p->elements[i+6]; cpy = p->elements[i+7];
#else
			printf("%g,%g %g %g %g %g,%g ", 
				dig3(p->elements[i+1]), dig3(p->elements[i+2]), dig3(p->elements[i+3]), 
				dig3(p->elements[i+4]), dig3(p->elements[i+5]), dig3(p->elements[i+6] - cpx), 
				dig3(p->elements[i+7] - cpy)
			);
			cpx = dig3(p->elements[i+6]); cpy = dig3(p->elements[i+7]);
#endif		
			i += 8;			
		}
	}
}

static SVGPathparser* nsvg__createParser(void){
	SVGPathparser* p;
	p = (SVGPathparser*)malloc(sizeof(SVGPathparser));
	if (p == NULL) goto error;
	memset(p, 0, sizeof(SVGPathparser));
	p->matrix = (float*) malloc(sizeof(float)*6);
	if (p->matrix == NULL) goto error;
	identityMatrix(p->matrix);
	p->angle = 0;
	return p;

error:
	if (p) {
		if (p->matrix) free(p->matrix);
		free(p);
	}
	printf("allocation error: nsvg__createParser\n");
	return NULL;
}

int main(int argc, char *argv[]) {
	char c, c1, c2;
	char* d = NULL;
	char* n;
	char item[ITEM_SIZE];
	char *pars = NULL;
	char *end = NULL;
	float t[] = { 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f };
	int absolute = 1; // generate absolute code by default
	int angle = 0;    // flag to indicate an angle was given
	int matrix = 0;   // flag to indicate a matrix was given
	float a, ang = 0.0f; 
	int i,j;
#ifdef DEBUG
	printf("**start**\n");
#endif
	SVGPathparser *p = nsvg__createParser();
	for (i = 1; i < argc; i++) {
#ifdef DEBUG
		printf("**argument %d\n%s\n", i, argv[i]);
#endif
		c1 = *argv[i];
		if (c1 == '-') {
			c2 = *(argv[i]+1);
			n = argv[i]+2;
			if (c2 == 'p' || c2 == 'P') {
				pars = argv[i]+2;                      // extra attributes of the path like color stroke width, etc.
				continue;
			}
			if (c2 == 'e' || c2 == 'E') {
				end = argv[i]+2;                      // end of path data to be added to the end of a path (ex. 'z')
				continue;
			}
			if (c2 == 'a' || c2 == 'A') {
				if (angle) continue;                 // if angle already given, ignore new one
				angle = 1;
				n = parseNumber(n, item, ITEM_SIZE);
				if (!*item) continue;                // angle is not anumber or not given. ignore
				ang = str2f(item);     
#ifdef DEBUG
				printf("%f\n",ang);
#endif          // if it doesn't match matrix, ignore it
				if (matrix && (truncf(ang) != truncf(a))) continue;
				p->angle = ang;                      // matches matrix or new angle
				continue;
			}
			if (c2 == 'r' || c2 == 'R') {
				absolute = 0;
				continue;
			}
			if (c2 == 'm' || c2 == 'M') {
				if (matrix) continue;                 // if already given, ignore new one
				matrix = 1;
				for (j = 0; j < 6; j++) {
				   while (*n && space(*n)) n++;
				   n = parseNumber(n, item, ITEM_SIZE);
				   if (!*item) break;                 // matrix incomplete. exit loop
				   t[j] = str2f(item);
				}
				if (!*item) continue;                 // matrix incomplete. ignore
				copyMatrix(p->matrix, t);
				a = getRotationAngleFromMatrix(t);    // mismatch with matrix, take matrix
				if (truncf(ang) != truncf(a)) p->angle = a;			
#ifdef DEBUG
				printf("[%f %f %f %f %f %f]\n",t[0],t[1],t[2],t[3],t[4],t[5],t[6]);
				printf("ang: %f - angle calculated: %f - angle stored: %f\n", ang, a, p->angle);
#endif
				continue;
			}
			continue;
		}
		d = argv[i]; // it's no a flag, thus, we assume it's the path
	}
	if (angle && !matrix) setRotationInMatrix(p->matrix, ang * NSVG_PI / 180.0f);
#ifdef DEBUG
	copyMatrix(t, p->matrix);
    printf("final matrix: [%f %f %f %f %f %f]\n",t[0],t[1],t[2],t[3],t[4],t[5],t[6]);
	printf("**parsing*\n");
#endif
	nsvg__parsePath(p,d);
#ifdef DEBUG
	printf("**generating SVG with %s coordinates**\n", ((absolute)? "absolute" : "relative"));
#endif
	if  (!pars) printf("<path d=\"");
	else  printf("<path %s d=\"", pars);
	if (absolute) generateAbsoluteSVG(p->plist);
	else generateRelativeSVG(p->plist);
	if (!end) printf("\"/>\n");
	else printf("%s\"/>\n", end);
#ifdef DEBUG
	printf("**finished**\n");
#endif
	return 0;
}
