/*	E U C L I D _ F O R M A T
 *
 * Converts from no CR format to Euclid decoded format
 *
 */

#ifndef lint
static char RCSid[] = "$Header$";
#endif

#include "conf.h"

#include <stdio.h>
#include <math.h>
#ifdef USE_STRING_H
#include <string.h>
#else
#include <strings.h>
#endif

#include "machine.h"
#include "externs.h"
#include "vmath.h"
#include "nmg.h"
#include "rtgeom.h"
#include "raytrace.h"
#include "../librt/debug.h"

main()
{
	char str[1024];
	point_t *pts;
	int ident,face_type,npts,face_no;
	plane_t pl;
	int tmp_i;
	float tmp_a;
	float a,b,c,d;
	int old_id=(-1);
	int face_count;

	if( scanf( "%s" , str ) == EOF )
		rt_bomb( "Unexpected EOF\n" );

	while( scanf( "%d %d %f %d %d" , &ident , &face_type , &tmp_a , &tmp_i , &npts ) != EOF )
	{
		int i;

		pts = (point_t *)rt_calloc( npts , sizeof( point_t ) , "pts" );

		for( i=0 ; i<npts ; i++ )
		{
			int j;
			float x,y,z;

			if( scanf( "%d %f %f %f" , &j , &a , &b , &c ) == EOF )
				rt_bomb( "Unexpected EOF\n" );

			if( j != i+1 )
			{
				rt_bomb( "Points out of order\n" );
			}

			VSET( pts[i] , a , b , c );
		}

		if( scanf( "%d %f %f %f %f" , &face_no , &a , &b , &c , &d ) == EOF )
			rt_bomb( "Unexpected EOF\n" );
		VSET( pl , a , b , c );
		pl[3] = d;

		if( ident != old_id )
		{
			face_count = 1;
			printf( "%5d%5d%5d    0    %8d      %5.2f               \n" ,
				face_count , npts , face_type , ident , 0.0 );
			old_id = ident;
		}
		else
		{
			face_count++;
			printf( "%5d%5d%5d    0                                              \n" ,
				face_count , npts , face_type );
		}

		printf( "%11.6f%11.6f%11.6f%13.6f\n" , V4ARGS( pl ) );

		for( i=0 ; i<npts ; i++ )
			printf( "%8f  %8f  %8f  \n" , V3ARGS( pts[i] ) );

		rt_free( (char *)pts , "pts" );
	}
	return 0;
}
