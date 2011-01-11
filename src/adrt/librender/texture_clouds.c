/*                     T E X T U R E _ C L O U D S . C
 * BRL-CAD / ADRT
 *
 * Copyright (c) 2002-2010 United States Government as represented by
 * the U.S. Army Research Laboratory.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * version 2.1 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this file; see the file named COPYING for more
 * information.
 */
/** @file texture_clouds.c
 *
 *  Comments -
 *      Texture Library - Perlin Clouds
 *
 */

#include "texture.h"
#include <stdlib.h>
#include "adrt_struct.h"

#include "bu.h"

void texture_clouds_init(struct texture_s *texture, tfloat size, int octaves, int absolute, TIE_3 scale, TIE_3 translate) {
    struct texture_clouds_s *td;

    texture->data = bu_malloc(sizeof(struct texture_clouds_s), "cloud data");
    texture->free = texture_clouds_free;
    texture->work = (struct texture_work_s *)texture_clouds_work;

    td = (struct texture_clouds_s*)texture->data;
    td->size = size;
    td->octaves = octaves;
    td->absolute = absolute;
    td->scale = scale;
    td->translate = translate;

    texture_perlin_init(&td->perlin);
}


void texture_clouds_free(struct texture_s *texture) {
    struct texture_clouds_s *td;

    td = (struct texture_clouds_s*)texture->data;
    texture_perlin_free(&td->perlin);
    bu_free(texture->data, "cloud data");
}


void texture_clouds_work(__TEXTURE_WORK_PROTOTYPE__) {
    struct texture_clouds_s *td;
    TIE_3 p, pt;


    td = (struct texture_clouds_s*)texture->data;

    /* Transform the Point */
    if (td->absolute) {
	p.v[0] = id->pos.v[0] * td->scale.v[0] + td->translate.v[0];
	p.v[1] = id->pos.v[1] * td->scale.v[1] + td->translate.v[1];
	p.v[2] = id->pos.v[2] * td->scale.v[2] + td->translate.v[2];
    } else {
	MATH_VEC_TRANSFORM(pt, id->pos, ADRT_MESH(mesh)->matinv);
	p.v[0] = (ADRT_MESH(mesh)->max.v[0] - ADRT_MESH(mesh)->min.v[0] > TIE_PREC ? (pt.v[0] - ADRT_MESH(mesh)->min.v[0]) / (ADRT_MESH(mesh)->max.v[0] - ADRT_MESH(mesh)->min.v[0]) : 0.0) * td->scale.v[0] + td->translate.v[0];
	p.v[1] = (ADRT_MESH(mesh)->max.v[1] - ADRT_MESH(mesh)->min.v[1] > TIE_PREC ? (pt.v[1] - ADRT_MESH(mesh)->min.v[1]) / (ADRT_MESH(mesh)->max.v[1] - ADRT_MESH(mesh)->min.v[1]) : 0.0) * td->scale.v[1] + td->translate.v[1];
	p.v[2] = (ADRT_MESH(mesh)->max.v[2] - ADRT_MESH(mesh)->min.v[2] > TIE_PREC ? (pt.v[2] - ADRT_MESH(mesh)->min.v[2]) / (ADRT_MESH(mesh)->max.v[2] - ADRT_MESH(mesh)->min.v[2]) : 0.0) * td->scale.v[2] + td->translate.v[2];
    }

    pixel->v[0] = fabs(0.5*texture_perlin_noise3(&td->perlin, p, td->size*1.0, td->octaves) + 0.5);
    pixel->v[1] = fabs(0.5*texture_perlin_noise3(&td->perlin, p, td->size*1.0, td->octaves) + 0.5);
    pixel->v[2] = fabs(0.5*texture_perlin_noise3(&td->perlin, p, td->size*1.0, td->octaves) + 0.5);
}

/*
 * Local Variables:
 * mode: C
 * tab-width: 8
 * indent-tabs-mode: t
 * c-file-style: "stroustrup"
 * End:
 * ex: shiftwidth=4 tabstop=8
 */
