/*
 *                      E N F - G
 *
 *  Program to convert the tessellated Elysium Neutral File format to BRL-CAD.
 *
 *  Author -
 *      John R. Anderson
 *  
 *  Source -
 *	The U. S. Army Research Laboratory
 *	Aberdeen Proving Ground, Maryland  21005-5068  USA
 *  
 *  Distribution Notice -
 *	Re-distribution of this software is restricted, as described in
 *	your "Statement of Terms and Conditions for the Release of
 *	The BRL-CAD Pacakge" agreement.
 *
 *  Copyright Notice -
 *	This software is Copyright (C) 2001 by the United States Army
 *	in all countries except the USA.  All rights reserved.
 */

#ifndef lint
static const char RCSid[] = "$Header$";
#endif

#include "conf.h"

#include <stdio.h>
#include <math.h>
#ifdef USE_STRING_H
#include <string.h>
#else
#include <strings.h>
#endif
#include <ctype.h>
#include <errno.h>

#include "machine.h"
#include "externs.h"
#include "db.h"
#include "vmath.h"
#include "nmg.h"
#include "rtgeom.h"
#include "raytrace.h"
#include "wdb.h"
#include "plot3.h"
#include "../librt/debug.h"

#define MAX_LINE_LEN	256

static	FILE *fd_in;
static	struct rt_wdb *fd_out;
static	char line[MAX_LINE_LEN];
static	fastf_t local_tol;
static	fastf_t local_tol_sq;
static	int ident;

static int verbose=0;

extern char *optarg;
extern int optind, opterr, optopt;

struct obj_info {
	char obj_type;			/* type of this object (from defines below) */
	char *obj_name;			/* name of this object */
	char *brlcad_comb;		/* unique BRL-CAD name for this region or assembly */
	char *brlcad_solid;		/* unique BRL-CAD name for the solid if this is a region */
	int obj_id;			/* id number (from ENF file) */
	int part_count;			/* number of members (for assembly), number of faces (for part) */
	struct obj_info **members;	/* pointer to array of member objects (only valid for assemblies) */
};

/* object types */
#define UNKNOWN_TYPE	0
#define PART_TYPE	1
#define ASSEMBLY_TYPE	2

static fastf_t *part_verts=NULL;	/* list of vertices for current part */
static int max_vert=0;			/* number of vertices currently malloced */
static int curr_vert=0;			/* number of vertices currently being used */

#define VERT_BLOCK 512			/* numer of vertices to malloc per call */

static int *part_tris=NULL;		/* list of triangles for current part */
static int max_tri=0;			/* number of triangles currently malloced */
static int curr_tri=0;			/* number of triangles currently being used */

#define TRI_BLOCK 512			/* number of triangles to malloc per call */

/* structure to make vertex searching fast
 * Each leaf represents a vertex, and has an index into the
 * part_verts array.
 * Each node is a cutting plane at the "cut_val" on the "coord" (0, 1, or 2) axis.
 * All vertices with "coord" value less than the "cut_val" are in the "lower"
 * subtree, others are in the "higher".
 */
union vert_tree {
	char type;
	struct vert_leaf {
		char type;
		int index;
	} vleaf;
	struct vert_node {
		char type;
		double cut_val;
		int coord;
		union vert_tree *higher, *lower;
	} vnode;
} *vert_root=NULL;

/* types for the above "vert_tree" */
#define VERT_LEAF	'l'
#define VERT_NODE	'n'



/* routine to check for bad triangles
 * only checks for triangles with duplicate vertices
 */
int
bad_triangle( int v[3], fastf_t *vertices )
{
	fastf_t dist;
	fastf_t coord;
	int i;

	if( v[0] == v[1] || v[1] == v[2] || v[0] == v[2] )
		return( 1 );

	dist = 0;
	for( i=0 ; i<3 ; i++ ) {
		coord = vertices[v[0]*3+i] - vertices[v[1]*3+i];
		dist += coord * coord;
	}
	dist = sqrt( dist );
	if( dist < local_tol ) {
		return( 1 );
	}

	dist = 0;
	for( i=0 ; i<3 ; i++ ) {
		coord = vertices[v[1]*3+i] - vertices[v[2]*3+i];
		dist += coord * coord;
	}
	dist = sqrt( dist );
	if( dist < local_tol ) {
		return( 1 );
	}

	dist = 0;
	for( i=0 ; i<3 ; i++ ) {
		coord = vertices[v[0]*3+i] - vertices[v[2]*3+i];
		dist += coord * coord;
	}
	dist = sqrt( dist );
	if( dist < local_tol ) {
		return( 1 );
	}

	return( 0 );
}


/* routine to add a new triangle to the current part */
void
add_triangle( int v[3] )
{
	if( curr_tri >= max_tri ) {
		/* allocate more memory for triangles */
		max_tri += TRI_BLOCK;
		part_tris = (int *)bu_realloc( part_tris, sizeof( int ) * max_tri * 3, "part_tris" );
	}

	/* fill in triangle info */
	VMOVE( &part_tris[curr_tri*3], v );

	/* increment count */
	curr_tri++;
}

/* routine to free the "vert_tree"
 * called after each part is output
 */
void
free_vert_tree( union vert_tree *ptr )
{
	if( !ptr )
		return;

	if( ptr->type == VERT_NODE ) {
		free_vert_tree( ptr->vnode.higher );
		free_vert_tree( ptr->vnode.lower );
	}

	free( (char *)ptr );
}


/* routine to add a vertex to the current list of part vertices */
int
Add_vert( point_t vertex )
{
	union vert_tree *ptr, *prev=NULL, *new_leaf, *new_node;
	vect_t diff;

	/* look for this vertex already in the list */
	ptr = vert_root;
	while( ptr ) {
		if( ptr->type == VERT_NODE ) {
			prev = ptr;
			if( vertex[ptr->vnode.coord] >= ptr->vnode.cut_val ) {
				ptr = ptr->vnode.higher;
			} else {
				ptr = ptr->vnode.lower;
			}
		} else {
			int ij;

			ij = ptr->vleaf.index*3;
			diff[0] = fabs( vertex[0] - part_verts[ij] ); 
			diff[1] = fabs( vertex[1] - part_verts[ij+1] ); 
			diff[2] = fabs( vertex[2] - part_verts[ij+2] ); 
			if( (diff[0]*diff[0] + diff[1]*diff[1] + diff[2]*diff[2]) <= local_tol_sq ) {
				/* close enough, use this vertex again */
				return( ptr->vleaf.index );
			}
			break;
		}
	}

	/* add this vertex to the list */
	if( curr_vert >= max_vert ) {
		/* allocate more memory for vertices */
		max_vert += VERT_BLOCK;

		part_verts = (fastf_t *)bu_realloc( part_verts, sizeof( fastf_t ) * max_vert * 3,
						    "part_verts" );
	}

	VMOVE( &part_verts[curr_vert*3], vertex );

	/* add to the tree also */
	new_leaf = (union vert_tree *)bu_malloc( sizeof( union vert_tree ), "new_leaf" );
	new_leaf->vleaf.type = VERT_LEAF;
	new_leaf->vleaf.index = curr_vert++;
	if( !vert_root ) {
		/* first vertex, it becomes the root */
		vert_root = new_leaf;
	} else if( ptr && ptr->type == VERT_LEAF ) {
		/* search above ended at a leaf, need to add a node above this leaf and the new leaf */
		new_node = (union vert_tree *)malloc( sizeof( union vert_tree ) );
		new_node->vnode.type = VERT_NODE;

		/* select the cutting coord based on the biggest difference */
		if( diff[0] >= diff[1] && diff[0] >= diff[2] ) {
			new_node->vnode.coord = 0;
		} else if( diff[1] >= diff[2] && diff[1] >= diff[0] ) {
			new_node->vnode.coord = 1;
		} else if( diff[2] >= diff[1] && diff[2] >= diff[0] ) {
			new_node->vnode.coord = 2;
		}

		/* set the cut value to the mid value between the two vertices */
		new_node->vnode.cut_val = (vertex[new_node->vnode.coord] +
					   part_verts[ptr->vleaf.index * 3 + new_node->vnode.coord]) * 0.5;

		/* set the node "lower" nad "higher" pointers */
		if( vertex[new_node->vnode.coord] >= 
		    part_verts[ptr->vleaf.index * 3 + new_node->vnode.coord] ) {
			new_node->vnode.higher = new_leaf;
			new_node->vnode.lower = ptr;
		} else {
			new_node->vnode.higher = ptr;
			new_node->vnode.lower = new_leaf;
		}

		if( ptr == vert_root ) {
			/* if the above search ended at the root, redefine the root */
			vert_root =  new_node;
		} else {
			/* set the previous node to point to our new one */
			if( prev->vnode.higher == ptr ) {
				prev->vnode.higher = new_node;
			} else {
				prev->vnode.lower = new_node;
			}
		}
	} else if( ptr && ptr->type == VERT_NODE ) {
		/* above search ended at a node, just add the new leaf */
		prev = ptr;
		if( vertex[prev->vnode.coord] >= prev->vnode.cut_val ) {
			if( prev->vnode.higher ) {
				exit(1);
			}
			prev->vnode.higher = new_leaf;
		} else {
			if( prev->vnode.lower ) {
				exit(1);
			}
			prev->vnode.lower = new_leaf;
		}
	} else {
		fprintf( stderr, "*********ERROR********\n" );
	}

	/* return the index into the vertex array */
	return( new_leaf->vleaf.index );
}

void
List_assem( struct obj_info *assem )
{
	int i;

	if( assem->obj_type != ASSEMBLY_TYPE ) {
		bu_log( "ERROR: List_assem called for non-assembly\n" );
	}
	bu_log( "Assembly: %s (id=%d)\n", assem->obj_name, assem->obj_id );
	bu_log( "\t%d members\n", assem->part_count );
	for( i=0 ; i<assem->part_count ; i++ ) {
		bu_log( "\t\ty %s\n", assem->members[i]->obj_name );
	}
}

void
Usage()
{
	bu_log( "Usage:\n\tenf-g [-i starting_ident] [-t tolerance] input_facets_file output_brlcad_file.g\n" );
}

void
Make_brlcad_names( struct obj_info *part )
{
	struct bu_vls vls;
	struct directory *dp;
	int count=0;
	char *tmp_name, *ptr;

	bu_vls_init( &vls );

	/* make a copy of object name, then make it a legal BRL-CAD name */
	tmp_name = bu_strdup( part->obj_name );
	ptr = tmp_name;
	while( *ptr != '\0' ) {
		if( !isalnum( *ptr ) ) {
			*ptr = '_';
		}
		ptr++;
	}

	if( part->obj_type == PART_TYPE ) {
		/* find a unique solid name */
		bu_vls_printf( &vls, "s.%s", tmp_name );
		while( (dp=db_lookup( fd_out->dbip, bu_vls_addr( &vls ), LOOKUP_QUIET )) != DIR_NULL) {
			count++;
			bu_vls_trunc( &vls, 0 );
			bu_vls_printf( &vls, "s.%s.%d", tmp_name, count );
		}
		part->brlcad_solid = bu_vls_strgrab( &vls );
	} else {
		part->brlcad_solid = NULL;
	}

	/* find a unique non-primitive name */
	bu_vls_printf( &vls, "%s", tmp_name );
	while( (dp=db_lookup( fd_out->dbip, bu_vls_addr( &vls ), LOOKUP_QUIET) ) != DIR_NULL ) {
		count++;
		bu_vls_trunc( &vls, 0 );
		bu_vls_printf( &vls, "%s.%d", tmp_name, count );
	}
	part->brlcad_comb = bu_vls_strgrab( &vls );

	bu_free( tmp_name, "tmp_name" );
}


struct obj_info *
Part_import( int id_start )
{
	struct obj_info *part;
	struct wmember reg_head;
	unsigned char rgb[3];
	int surf_count=0;
	int id_end;
	int last_surf=0;
	int i;
	int tri[3];
	int corner_index=-1;

	VSETALL( rgb, 128 );

	part = (struct obj_info *)bu_calloc( 1, sizeof( struct obj_info ), "part" );
	part->obj_type = PART_TYPE;
	part->obj_id = id_start;
	while( fgets( line, MAX_LINE_LEN, fd_in ) ) {
		if( !strncmp( line, "PartName", 8 ) ) {
			line[strlen( line ) - 1] = '\0';
			part->obj_name = bu_strdup( &line[9] );
			Make_brlcad_names( part );
			bu_log( "Found Part %s\n", part->obj_name );
		} else if( !strncmp( line, "FaceCount", 9 ) ) {
			surf_count = atoi( &line[10] );
		} else if( !strncmp( line, "EndPartId", 9 ) ) {
			/* found end of part, check id */
			id_end = atoi( &line[10] );
			if( id_end != id_start ) {
				bu_log( "ERROR: found end of part id %d while processing part %d\n",
					id_end, id_start );
				exit( 1 );
			}
			if( last_surf ) {
				break;
			}
		} else if( !strncmp( line, "FaceRGB", 7 ) ) {
			/* get face color */
			char *ptr;

			i = 8;
			ptr = strtok( &line[i], " \t" );
			for( i=0 ; i<3 && ptr ; i++ ) {
				rgb[i] = atof( ptr );
				ptr = strtok( (char *)NULL, " \t" );
			}
		} else if( !strncmp( line, "Facet", 5 ) ) {
			/* read a triangle */
			VSETALL( tri, -1 );
			corner_index = -1;
		} else if( !strncmp( line, "Face", 4 ) ) {
			/* start of a surface */
			int surf_no;

			surf_no = atoi( &line[5] );
			if( surf_no == surf_count ) {
				last_surf = 1;
			}
		} else if( !strncmp( line, "TriangleCount", 13 ) ) {
			/* get number of triangles for this surface */
		} else if( !strncmp( line, "Verticies", 9 ) ) {
			/* get vertex list for this triangle */
		} else if( !strncmp( line, "Vertex", 6 ) ) {
			/* get a vertex */
			char *ptr;
			point_t v;

			i = 7;
			while( !isspace( line[i] ) && line[i] != '\0' )
				i++;
			ptr = strtok( &line[i], " \t" );
			for( i=0 ; i<3 && ptr ; i++ ) {
				v[i] = atof( ptr );
				ptr = strtok( (char *)NULL, " \t" );
			}
			tri[++corner_index] = Add_vert( v );
			if( corner_index == 2 ) {
				if( !bad_triangle( tri, part_verts ) ) {
					add_triangle( tri );
				}
			}
		} else if( !strncmp( line, "Normal", 6 ) ) {
			/* get a vertex normal */
		} else if( !strncmp( line, "PointCount", 10 ) ) {
			/* get number of vertices for this surface */
		} else {
			bu_log( "ERROR: unrecognized line encountered while processing part id %d:\n",
				id_start );
			bu_log( "%s\n", line );
			exit( 1 );
		}
	}

	/* write this part to database, first make a primitive solid */
	if( mk_bot( fd_out, part->brlcad_solid, RT_BOT_SOLID, RT_BOT_UNORIENTED, 0,
		curr_vert, curr_tri, part_verts, part_tris, NULL, NULL ) ) {
			bu_log( "Failed to write primitive %s (%s) to database\n",
				part->brlcad_solid, part->obj_name );
			exit( 1 );
	}

	/* then a region */
	BU_LIST_INIT( &reg_head.l );
	if( mk_addmember( part->brlcad_solid, &reg_head.l, WMOP_UNION ) == WMEMBER_NULL ) {
		bu_log( "ERROR: Failed to add solid (%s), to region (%s)\n",
			part->brlcad_solid, part->brlcad_comb );
		exit( 1 );
	}
	if( mk_comb( fd_out, part->brlcad_comb, &reg_head.l, 1, NULL, NULL, rgb, ident++,
		     0, 0, 0, 0, 0, 0 ) ) {
		bu_log( "Failed to write region %s (%s) to database\n",
			part->brlcad_comb, part->obj_name );
		exit( 1 );
	}

	/* free some memory */
	free_vert_tree( vert_root );
	vert_root = NULL;
	bu_free( (char *)part_tris, "part_tris" );
	max_tri = 0;
	curr_tri = 0;
	part_tris = NULL;
	bu_free( (char *)part_verts, "part_verts" );
	max_vert = 0;
	curr_vert = 0;
	part_verts = NULL;

	return( part );
}

struct obj_info *
Assembly_import( int id_start )
{
	struct obj_info *this_assem, *member;
	struct wmember assem_head;
	int id_end, member_id;
	int i;

	this_assem = (struct obj_info *)bu_calloc( 1, sizeof( struct obj_info ),
						   "this_assem" );
	this_assem->obj_type = ASSEMBLY_TYPE;
	this_assem->obj_id = id_start;
	this_assem->part_count = 0;
	this_assem->members = NULL;
	while( fgets( line, MAX_LINE_LEN, fd_in ) ) {
		if( !strncmp( line, "AssemblyName", 12 ) ) {
			line[strlen( line ) - 1] = '\0';
			this_assem->obj_name = bu_strdup( &line[13] );
			bu_log( "Found assembly %s\n", this_assem->obj_name );
		} else if( !strncmp( line, "PartId", 6 ) ) {
			/* found a member part */
			member_id = atoi( &line[7] );
			member = Part_import( member_id );
			this_assem->part_count++;
			this_assem->members = (struct obj_info **)bu_realloc(
			      this_assem->members,
			      this_assem->part_count * sizeof( struct obj_info *),
			      "this_assem->members" );
			this_assem->members[this_assem->part_count-1] = member;
		} else if( !strncmp( line, "AssemblyId", 10 ) ) {
			/* found a member assembly */
			member_id = atoi( &line[11] );
			member = Assembly_import( member_id );
			this_assem->part_count++;
			this_assem->members = (struct obj_info **)bu_realloc(
			      this_assem->members,
			      this_assem->part_count * sizeof( struct obj_info *),
			      "this_assem->members" );
			this_assem->members[this_assem->part_count-1] = member;
		} else if( !strncmp( line, "EndAssemblyId", 13 ) ) {
			/* found end of assembly, make sure it is this one */
			id_end = atoi( &line[14] );
			if( id_end != id_start ) {
				bu_log( "ERROR: found end of assembly id %d while processing id %d\n",
					id_end, id_start );
				exit( 1 );
			}
			break;
		} else {
			bu_log( "Unrecognized line encountered while processing assembly id %d:\n",
				id_start );
			bu_log( "%s\n", line );
			exit( 1 );
		}
	}

	Make_brlcad_names( this_assem );

	/* write this assembly to the database */
	BU_LIST_INIT( &assem_head.l );

	for( i=0 ; i<this_assem->part_count ; i++ ) {
		if( mk_addmember( this_assem->members[i]->brlcad_comb,
				  &assem_head.l, WMOP_UNION ) == WMEMBER_NULL ) {
				bu_log( "ERROR: Failed to add region %s to assembly %s\n",
					this_assem->members[i]->brlcad_comb,
					this_assem->brlcad_comb );
				exit( 1 );
		}
	}

	if( mk_comb( fd_out, this_assem->brlcad_comb, &assem_head.l, 0, NULL, NULL, NULL,
		     0, 0, 0, 0, 0, 0, 0 ) ) {
		bu_log( "ERROR: Failed to write combination (%s) to database\n",
			this_assem->brlcad_comb );
		exit( 1 );
	}

	return( this_assem );
}

int
main( int argc, char *argv[] )
{
	char *input_file, *output_file;
	struct obj_info **top_level_assems=NULL;
	int top_level_assem_count=0;
	int curr_top_level=-1;
	fastf_t tmp;
	int id;
	int c;

	local_tol = 0.005;
	local_tol_sq = local_tol * local_tol;
	ident = 1000;

	while( (c=getopt( argc, argv, "vi:t:" ) ) != EOF ) {
		switch( c ) {
			case 'v':	/* verbose */
				verbose = 1;
				break;
			case 'i':	/* starting ident number */
				ident = atoi( optarg );
				break;
			case 't':	/* tolerance */
				tmp = atof( optarg );
				if( tmp <= 0.0 ) {
					bu_log( "Illegal tolerance (%g), musy be > 0.0\n",
						tmp );
					exit( 1 );
				}
				break;
			default:
				bu_log( "Unrecognized option %c\n", c );
				Usage();
				exit( 1 );
		}
	}

	if( argc - optind != 2 ) {
		bu_log( "Not enough arguments!!\n" );
		Usage();
		exit( 1 );
	}

	input_file = bu_strdup( argv[optind] );
	output_file = bu_strdup( argv[optind+1] );

	if( (fd_in=fopen( input_file, "r" )) == NULL ) {
		bu_log( "Cannot open %s for reading\n", input_file );
		perror( argv[0] );
		exit( 1 );
	}

	if( (fd_out=wdb_fopen( output_file )) == NULL ) {
		bu_log( "Cannot open %s for writing\n", output_file );
		perror( argv[0] );
		exit( 1 );
	}

	/* finally, start processing the input */
	while( fgets( line, MAX_LINE_LEN, fd_in ) ) {
		if( !strncmp( line, "FileName", 8 ) ) {
			bu_log( "Converting facets originally from %s",
				&line[9] );
		} else if( !strncmp( line, "TopAssemblies", 13 ) ) {
			bu_log( "Top level assemblies: %s", &line[14] );
			top_level_assem_count = atoi( &line[14] );
			top_level_assems = (struct obj_info **)bu_calloc( top_level_assem_count,
									 sizeof( struct obj_info * ),
									 "top_level_assems" );
		} else if( !strncmp( line, "PartCount", 9 ) ) {
			bu_log( "Part count: %s", &line[10] );
		} else if( !strncmp( line, "AssemblyId", 10 ) ) {
			id = atoi( &line[11] );
			curr_top_level++;
			if( curr_top_level >= top_level_assem_count ) {
				bu_log( "Warning: too many top level assemblies\n" );
				bu_log( "\texpected %d, this os number %d\n",
					top_level_assem_count, curr_top_level+1 );
				top_level_assem_count = curr_top_level+1;
				top_level_assems = (struct obj_info **)bu_realloc( top_level_assems,
									  top_level_assem_count *
									 sizeof( struct obj_info * ),
									 "top_level_assems" );
			}
			top_level_assems[curr_top_level] = Assembly_import( id );
		} else if( !strncmp( line, "PartId", 6 ) ) {
			/* found a top-level part */
			struct obj_info *top_level_part;

			id = atoi( &line[7] );
			top_level_part = Part_import( id );
		}
	}

	return( 0 );
}
