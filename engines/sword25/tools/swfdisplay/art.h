/* Libart_LGPL - library of basic graphic primitives
 * Copyright (C) 1998 Raph Levien
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

/* Simple macros to set up storage allocation and basic types for libart
   functions. */

#ifndef __ART_MISC_H__
#define __ART_MISC_H__

#define DEBUG 1

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <math.h>

typedef unsigned char byte;
typedef unsigned short uint16;
typedef unsigned int uint32;
typedef unsigned int uint;
typedef short int16;
typedef int int32;

typedef byte art_u8;
typedef uint16 art_u16;
typedef uint32 art_u32;

#define FLIP_NONE 0

#define BS_ASSERT(x) assert(x)
#define error(x) assert(0)
#define BS_LOG_ERRORLN(x) assert(0)

#define BS_ARGB(A,R,G,B)    (((A) << 24) | ((R) << 16) | ((G) << 8) | (B))

/* These aren't, strictly speaking, configuration macros, but they're
   damn handy to have around, and may be worth playing with for
   debugging. */
#define art_new(type, n) ((type *)malloc ((n) * sizeof(type)))

#define art_renew(p, type, n) ((type *)realloc (p, (n) * sizeof(type)))

/* This one must be used carefully - in particular, p and max should
   be variables. They can also be pstruct->el lvalues. */
#define art_expand(p, type, max) do { if(max) { p = art_renew (p, type, max <<= 1); } else { max = 1; p = art_new(type, 1); } } while (0)

typedef int art_boolean;
#define ART_FALSE 0
#define ART_TRUE 1

/* define pi */
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif  /*  M_PI  */

#ifndef M_SQRT2
#define M_SQRT2         1.41421356237309504880  /* sqrt(2) */
#endif  /* M_SQRT2 */

/* Provide macros to feature the GCC function attribute.
 */
#if defined(__GNUC__) && (__GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ > 4))
#define ART_GNUC_PRINTF( format_idx, arg_idx )    \
	__attribute__((__format__ (__printf__, format_idx, arg_idx)))
#define ART_GNUC_NORETURN                         \
	__attribute__((__noreturn__))
#else   /* !__GNUC__ */
#define ART_GNUC_PRINTF( format_idx, arg_idx )
#define ART_GNUC_NORETURN
#endif  /* !__GNUC__ */

void ART_GNUC_NORETURN
art_die(const char *fmt, ...) ART_GNUC_PRINTF(1, 2);

void
art_warn(const char *fmt, ...) ART_GNUC_PRINTF(1, 2);

#define ART_USE_NEW_INTERSECTOR

typedef struct _ArtDRect ArtDRect;
typedef struct _ArtIRect ArtIRect;

struct _ArtDRect {
	/*< public >*/
	double x0, y0, x1, y1;
};

struct _ArtIRect {
	/*< public >*/
	int x0, y0, x1, y1;
};

typedef struct _ArtPoint ArtPoint;

struct _ArtPoint {
	/*< public >*/
	double x, y;
};

/* Basic data structures and constructors for sorted vector paths */

typedef struct _ArtSVP ArtSVP;
typedef struct _ArtSVPSeg ArtSVPSeg;

struct _ArtSVPSeg {
	int n_points;
	int dir; /* == 0 for "up", 1 for "down" */
	ArtDRect bbox;
	ArtPoint *points;
};

struct _ArtSVP {
	int n_segs;
	ArtSVPSeg segs[1];
};

void
art_svp_free(ArtSVP *svp);

int
art_svp_seg_compare(const void *s1, const void *s2);

/* Basic data structures and constructors for bezier paths */

typedef enum {
	ART_MOVETO,
	ART_MOVETO_OPEN,
	ART_CURVETO,
	ART_LINETO,
	ART_END
} ArtPathcode;

typedef struct _ArtBpath ArtBpath;

struct _ArtBpath {
	/*< public >*/
	ArtPathcode code;
	double x1;
	double y1;
	double x2;
	double y2;
	double x3;
	double y3;
};

/* Basic data structures and constructors for simple vector paths */

typedef struct _ArtVpath ArtVpath;

/* CURVETO is not allowed! */
struct _ArtVpath {
	ArtPathcode code;
	double x;
	double y;
};

/* Some of the functions need to go into their own modules */

void
art_vpath_add_point(ArtVpath **p_vpath, int *pn_points, int *pn_points_max,
                    ArtPathcode code, double x, double y);

ArtVpath *art_bez_path_to_vec(const ArtBpath *bez, double flatness);

#endif /* __ART_MISC_H__ */
