/*
 *			W D B . H
 *
 *  Interface structures and routines for libwdb
 *
 *  Notes -
 *	Any source file that includes this header file must also include
 *	<stdio.h> to obtain the typedef for FILE
 *
 *  Author -
 *	Michael John Muuss
 *  
 *  Source -
 *	SECAD/VLD Computing Consortium, Bldg 394
 *	The U. S. Army Ballistic Research Laboratory
 *	Aberdeen Proving Ground, Maryland  21005
 *  
 *  Copyright Notice -
 *	This software is Copyright (C) 1988 by the United States Army.
 *	All rights reserved.
 *
 *  $Header$
 */

#ifndef SEEN_RTLIST_H
# include "rtlist.h"
#endif

#ifndef WDB_H
#define WDB_H seen
/*
 *  Macros for providing function prototypes, regardless of whether
 *  the compiler understands them or not.
 *  It is vital that the argument list given for "args" be enclosed
 *  in parens.
 */
#if __STDC__
#	define	WDB_EXTERN(type_and_name,args)	extern type_and_name args
#	define	WDB_ARGS(args)			args
#else
#	define	WDB_EXTERN(type_and_name,args)	extern type_and_name()
#	define	WDB_ARGS(args)			()
#endif

/*
 *  In-memory form of database combinations
 */
struct wmember  {
	struct rt_list	l;
	int		wm_op;		/* Boolean operation */
	mat_t		wm_mat;
	char		wm_name[16+3];	/* NAMESIZE */
};
#define WMEMBER_NULL	((struct wmember *)0)
#define WMEMBER_MAGIC	0x43128912

/*
 *  Definitions for pipe (wire) segments
 */
struct wdb_pipeseg  {
	struct rt_list	l;		/* doubly linked list support */
	point_t		ps_start;	/* start point of centerline */
	point_t		ps_bendcenter;	/* BEND only: center of bend circle */
	fastf_t		ps_id;		/* inner diam, <=0 if solid (wire) */
	fastf_t		ps_od;		/* pipe outer diam */
	int		ps_type;	/* WDB_PIPESEG_TYPE_{END, LINEAR, BEND} */
};
#define WDB_PIPESEG_NULL	((struct wdb_pipeseg *)0)
#define WDB_PIPESEG_MAGIC	0x9723ffef
#define WDB_PIPESEG_TYPE_END	1	/* End of pipe.  Required, last */
#define WDB_PIPESEG_TYPE_LINEAR	2	/* Linear pipe segment */
#define WDB_PIPESEG_TYPE_BEND	3	/* Bending pipe segment */

/*
 *  Solid conversion routines
 */
WDB_EXTERN(int mk_id, (FILE *fp, char *title) );
WDB_EXTERN(int mk_half, (FILE *fp, char *name, vect_t norm, double d) );
WDB_EXTERN(int mk_rpp, (FILE *fp, char *name, point_t min, point_t max) );
WDB_EXTERN(int mk_arb4, (FILE *fp, char *name, point_t pts[]) );
WDB_EXTERN(int mk_arb8, (FILE *fp, char *name, point_t pts[]) );
WDB_EXTERN(int mk_sph, (FILE *fp, char *name, point_t center, fastf_t radius) );
WDB_EXTERN(int mk_ell, (FILE *fp, char *name, point_t center,
			vect_t a, vect_t b, vect_t c) );
WDB_EXTERN(int mk_tor, (FILE *fp, char *name, point_t center, vect_t inorm,
			double r1, double r2) );
WDB_EXTERN(int mk_rcc, (FILE *fp, char *name, point_t base, vect_t height,
			fastf_t radius) );
WDB_EXTERN(int mk_tgc, (FILE *fp, char *name, point_t base, vect_t height,
			vect_t a, vect_t b, vect_t c, vect_t d) );
WDB_EXTERN(int mk_trc, (FILE *fp, char *name, point_t ibase, vect_t iheight,
			fastf_t radbase, fastf_t radtop) );
WDB_EXTERN(int mk_trc_top, (FILE *fp, char *name, point_t ibase, point_t itop,
			fastf_t radbase, fastf_t radtop) );
WDB_EXTERN(int mk_polysolid, (FILE *fp, char *name) );
WDB_EXTERN(int mk_poly, (FILE *fp, int npts,
			fastf_t verts[][3], fastf_t norms[][3]) );
WDB_EXTERN(int mk_fpoly, (FILE *fp, int npts, fastf_t verts[][3]) );

WDB_EXTERN(int mk_arbn, (FILE *fp, char *name, int neqn, plane_t eqn[]) );
WDB_EXTERN(int mk_ars, (FILE *fp, char *name, int ncurves, int pts_per_curve,
			fastf_t	*curves[]) );
WDB_EXTERN(int mk_bsolid, (FILE *fp, char *name, int nsurf, double res) );
WDB_EXTERN(int mk_bsurf, (FILE *fp, struct b_spline *bp) );
WDB_EXTERN(int mk_particle, (FILE *fp, char *name, point_t vertex,
			vect_t height, double vradius, double hradius) );
WDB_EXTERN(int mk_pipe, (FILE *fp, char *name, struct wdb_pipeseg *headp) );

/*
 *  Combination conversion routines
 */
WDB_EXTERN(int mk_comb, (FILE *fp, char *name, int len, int region_flag,
			char *matname, char *matparm, char *rgb,
			int inherit_flag) );
WDB_EXTERN(int mk_rcomb, (FILE *fp, char *name, int len, int region_flag,
			char *matname, char *matparm, char *rgb,
			int id, int air, int material, int los,
			int inherit_flag) );
WDB_EXTERN(int mk_fcomb, (FILE *fp, char *name, int len, int region_flag) );
WDB_EXTERN(int mk_memb, (FILE *fp, char *name, mat_t mat, int bool_op) );
WDB_EXTERN(struct wmember *mk_addmember, (char *name, struct wmember *headp) );
WDB_EXTERN(int mk_lcomb, (FILE *fp, char *name, struct wmember *headp,
			int region_flag,
			char *matname, char *matparm, char *rgb,
			int inherit_flag) );
WDB_EXTERN(int mk_lrcomb, (FILE *fp, char *name, struct wmember *headp,
			int region_flag,
			char *matname, char *matparm, char *rgb,
			int id, int air, int material, int los,
			int inherit_flag) );

/* Values for wm_op.  These must track db.h */
#define WMOP_INTERSECT	'+'
#define WMOP_SUBTRACT	'-'
#define WMOP_UNION	'u'

/* Convienient definitions */
#define mk_lfcomb(fp,name,headp,region)		mk_lcomb( fp, name, headp, \
	region, (char *)0, (char *)0, (char *)0, 0 );

/*
 *  Routines to establish conversion factors
 */
WDB_EXTERN(int mk_conversion, (char *units_string) );
WDB_EXTERN(int mk_set_conversion, (double val) );
WDB_EXTERN(double mk_cvt_factor, (char *units_string) );

/*
 * This internal variable should not be directly modified;
 * call mk_conversion() or mk_set_conversion() instead.
 */
extern double	mk_conv2mm;		/* Conversion factor to mm */

/*
 *  Internal routines
 */
WDB_EXTERN(int mk_freemembers, (struct wmember *headp) );

#endif /* WDB_H */
