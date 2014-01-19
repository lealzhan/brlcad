/* S H A P E _ D E F I N I T I O N _ R E P R E S E N T A T I O N . C P P
 *
 * BRL-CAD
 *
 * Copyright (c) 2013-2014 United States Government as represented by
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
/** @file ON_Brep.cpp
 *
 * File for writing out a Shape Definition Representation
 *
 */

#include "common.h"

#include "G_STEP_internal.h"
#include "ON_Brep.h"

/* Shape Definition Representation
 *
 */
STEPentity *
Add_Shape_Definition_Representation(Registry *registry, InstMgr *instance_list, SdaiRepresentation *sdairep)
{
    // SHAPE_DEFINITION_REPRESENTATION
    STEPentity *ret_entity = registry->ObjCreate("SHAPE_DEFINITION_REPRESENTATION");
    instance_list->Append(ret_entity, completeSE);
    SdaiShape_definition_representation *shape_def_rep = (SdaiShape_definition_representation *)ret_entity;
    shape_def_rep->used_representation_(sdairep);

    // PRODUCT_DEFINITION_SHAPE
    SdaiProduct_definition_shape *prod_def_shape = (SdaiProduct_definition_shape *)registry->ObjCreate("PRODUCT_DEFINITION_SHAPE");
    instance_list->Append((STEPentity *)prod_def_shape, completeSE);
    prod_def_shape->name_("''");
    prod_def_shape->description_("''");

    // PRODUCT_DEFINITION
    SdaiProduct_definition *prod_def = (SdaiProduct_definition *)registry->ObjCreate("PRODUCT_DEFINITION");
    instance_list->Append((STEPentity *)prod_def, completeSE);
    SdaiCharacterized_product_definition *char_def_prod = new SdaiCharacterized_product_definition(prod_def);
    SdaiCharacterized_definition *char_def= new SdaiCharacterized_definition(char_def_prod);
    prod_def_shape->definition_(char_def);
    prod_def->id_("''");
    prod_def->description_("''");

    // PRODUCT_DEFINITION_FORMATION
    SdaiProduct_definition_formation *prod_def_form = (SdaiProduct_definition_formation *)registry->ObjCreate("PRODUCT_DEFINITION_FORMATION");
    instance_list->Append((STEPentity *)prod_def_form, completeSE);
    prod_def->formation_(prod_def_form);
    prod_def_form->id_("''");
    prod_def_form->description_("''");

    // PRODUCT_RELATED_PRODUCT_CATEGORY
    SdaiProduct *prod_rel_prod_cat = (SdaiProduct *)registry->ObjCreate("PRODUCT_RELATED_PRODUCT_CATEGORY");
    instance_list->Append((STEPentity *)prod_rel_prod_cat, completeSE);
    prod_rel_prod_cat->id_("''");
    prod_rel_prod_cat->name_("''");
    prod_rel_prod_cat->description_("''");

    // PRODUCT
    SdaiProduct *prod = (SdaiProduct *)registry->ObjCreate("PRODUCT");
    instance_list->Append((STEPentity *)prod, completeSE);
    prod_def_form->of_product_(prod);
    prod->id_("''");
    prod->name_("''");
    prod->description_("''");

    // PRODUCT_DEFINITION_CONTEXT
    SdaiProduct_definition_context *prod_def_context = (SdaiProduct_definition_context *)registry->ObjCreate("PRODUCT_DEFINITION_CONTEXT");
    instance_list->Append((STEPentity *)prod_def_context, completeSE);
    prod_def->frame_of_reference_(prod_def_context);
    prod_def_context->life_cycle_stage_("'part definition'");

    // PRODUCT_CONTEXT
    SdaiProduct_context *product_context = (SdaiProduct_context *)registry->ObjCreate("PRODUCT_CONTEXT");
    instance_list->Append((STEPentity *)product_context, completeSE);
    product_context->name_("''");

    EntityAggregate *context_agg = new EntityAggregate();
    context_agg->AddNode(new EntityNode((SDAI_Application_instance *) product_context));
    prod->frame_of_reference_(context_agg);

    // APPLICATION_CONTEXT
    SdaiApplication_context *app_context = (SdaiApplication_context *)registry->ObjCreate("APPLICATION_CONTEXT");
    instance_list->Append((STEPentity *)app_context, completeSE);
    prod_def_context->frame_of_reference_(app_context);
    product_context->frame_of_reference_(app_context);
    app_context->application_("'Core Data for Automotive Mechanical Design Process'");

    //return ret_entity;
    // The product definition is what is used to define assemblies, so return that
    return (STEPentity *)prod_def;
}


// Local Variables:
// tab-width: 8
// mode: C++
// c-basic-offset: 4
// indent-tabs-mode: t
// c-file-style: "stroustrup"
// End:
// ex: shiftwidth=4 tabstop=8