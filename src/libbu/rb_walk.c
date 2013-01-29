/*                       R B _ W A L K . C
 * BRL-CAD
 *
 * Copyright (c) 1998-2013 United States Government as represented by
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

#include "common.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "bu.h"

#include "./rb_internals.h"


/**
 * P R E W A L K N O D E S
 *
 * Perform a preorder traversal of a red-black tree
 */
HIDDEN void
prewalknodes(struct bu_rb_node *root, int order, void (*visit) (/* ??? */), int depth)
{
    BU_CKMAG(root, BU_RB_NODE_MAGIC, "red-black node");
    RB_CKORDER(root->rbn_tree, order);

    if (root == RB_NULL(root->rbn_tree))
	return;
    visit(root, depth);
    prewalknodes (RB_LEFT_CHILD(root, order), order, visit, depth + 1);
    prewalknodes (RB_RIGHT_CHILD(root, order), order, visit, depth + 1);
}


/**
 * I N W A L K N O D E S
 *
 * Perform an inorder traversal of a red-black tree
 */
HIDDEN void
inwalknodes(struct bu_rb_node *root, int order, void (*visit) (/* ??? */), int depth)
{
    BU_CKMAG(root, BU_RB_NODE_MAGIC, "red-black node");
    RB_CKORDER(root->rbn_tree, order);

    if (root == RB_NULL(root->rbn_tree))
	return;
    inwalknodes (RB_LEFT_CHILD(root, order), order, visit, depth + 1);
    visit(root, depth);
    inwalknodes (RB_RIGHT_CHILD(root, order), order, visit, depth + 1);
}


/**
 * P O S T W A L K N O D E S
 *
 * Perform a postorder traversal of a red-black tree
 */
HIDDEN void
postwalknodes(struct bu_rb_node *root, int order, void (*visit) (/* ??? */), int depth)
{
    BU_CKMAG(root, BU_RB_NODE_MAGIC, "red-black node");
    RB_CKORDER(root->rbn_tree, order);

    if (root == RB_NULL(root->rbn_tree))
	return;
    postwalknodes (RB_LEFT_CHILD(root, order), order, visit, depth + 1);
    postwalknodes (RB_RIGHT_CHILD(root, order), order, visit, depth + 1);
    visit(root, depth);
}


/**
 * P R E W A L K D A T A
 *
 * Perform a preorder traversal of a red-black tree
 */
HIDDEN void
prewalkdata(struct bu_rb_node *root, int order, void (*visit) (/* ??? */), int depth)
{
    BU_CKMAG(root, BU_RB_NODE_MAGIC, "red-black node");
    RB_CKORDER(root->rbn_tree, order);

    if (root == RB_NULL(root->rbn_tree))
	return;
    visit(RB_DATA(root, order), depth);
    prewalkdata (RB_LEFT_CHILD(root, order), order, visit, depth + 1);
    prewalkdata (RB_RIGHT_CHILD(root, order), order, visit, depth + 1);
}


/**
 * I N W A L K D A T A
 *
 * Perform an inorder traversal of a red-black tree
 */
HIDDEN void
inwalkdata(struct bu_rb_node *root, int order, void (*visit) (/* ??? */), int depth)
{
    BU_CKMAG(root, BU_RB_NODE_MAGIC, "red-black node");
    RB_CKORDER(root->rbn_tree, order);

    if (root == RB_NULL(root->rbn_tree))
	return;
    inwalkdata (RB_LEFT_CHILD(root, order), order, visit, depth + 1);
    visit(RB_DATA(root, order), depth);
    inwalkdata (RB_RIGHT_CHILD(root, order), order, visit, depth + 1);
}


/**
 * P O S T W A L K D A T A
 *
 * Perform a postorder traversal of a red-black tree
 */
HIDDEN void
postwalkdata(struct bu_rb_node *root, int order, void (*visit) (/* ??? */), int depth)
{
    BU_CKMAG(root, BU_RB_NODE_MAGIC, "red-black node");
    RB_CKORDER(root->rbn_tree, order);

    if (root == RB_NULL(root->rbn_tree))
	return;
    postwalkdata (RB_LEFT_CHILD(root, order), order, visit, depth + 1);
    postwalkdata (RB_RIGHT_CHILD(root, order), order, visit, depth + 1);
    visit(RB_DATA(root, order), depth);
}


void
rb_walk(struct bu_rb_tree *tree, int order, void (*visit) (/* ??? */), int what_to_visit, int trav_type)
{
    static void (*walk[][3])() = {
	{ prewalknodes, inwalknodes, postwalknodes },
	{ prewalkdata, inwalkdata, postwalkdata }
    };

    BU_CKMAG(tree, BU_RB_TREE_MAGIC, "red-black tree");
    RB_CKORDER(tree, order);
    switch (trav_type) {
	case PREORDER:
	case INORDER:
	case POSTORDER:
	    switch (what_to_visit) {
		case WALK_NODES:
		case WALK_DATA:
		    (*walk[what_to_visit][trav_type])
			(RB_ROOT(tree, order), order, visit, 0);
		    break;
		default:
		    bu_log("ERROR: rb_walk(): Illegal visitation object: %d\n",
			   what_to_visit);
		    bu_bomb("");
	    }
	    break;
	default:
	    bu_log("ERROR: rb_walk(): Illegal traversal type: %d\n",
		   trav_type);
	    bu_bomb("");
    }
}

void
bu_rb_walk(struct bu_rb_tree *tree, int order, void (*visit) (/* ??? */), int trav_type)
{
    BU_CKMAG(tree, BU_RB_TREE_MAGIC, "red-black tree");
    RB_CKORDER(tree, order);

    rb_walk(tree, order, visit, WALK_DATA, trav_type);
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
