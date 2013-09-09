/* 
 * Copyright (c) Tony Bybell 1999-2011.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#include "globals.h"

/*
 * tree.h 12/05/98ajb
 */
#ifndef WAVE_TREE_H
#define WAVE_TREE_H

#include <gtk/gtk.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "debug.h"
#include "symbol.h"
#include "vcd.h"
#include "tree_component.h"

#define FST_TREE_SEARCH_NEXT_LIMIT (40)

/* Kind of the tree.  */
enum tree_kind
 {
   /* Unknown.  */
   TREE_UNKNOWN,

   /* An internal signal.  */
   TREE_SIGNAL,

   /* An in/out/inout signal.  */
   TREE_IN,
   TREE_OUT,
   TREE_INOUT,

   /* An element of a vector.  */
   TREE_VECTOREL,
   /* An element of a record.  */
   TREE_RECORDEL,

   /* A Subinstance.  */
   TREE_INSTANCE,

   /* A package (somewhat VHDL specific ?).  */
   TREE_PACKAGE,

   /* A base (source file).  Not yet implemented.  */
   TREE_BASE,

   /* Verilog/SV scope types */
   TREE_VCD_ST_MODULE,
   TREE_VCD_ST_TASK,
   TREE_VCD_ST_FUNCTION,
   TREE_VCD_ST_BEGIN,
   TREE_VCD_ST_FORK,
   TREE_VCD_ST_GENERATE,
   TREE_VCD_ST_STRUCT,
   TREE_VCD_ST_UNION,
   TREE_VCD_ST_CLASS,
   TREE_VCD_ST_INTERFACE,
   TREE_VCD_ST_PACKAGE,
   TREE_VCD_ST_PROGRAM,

   /* GHW VHDL scope types */
   TREE_VHDL_ST_DESIGN,
   TREE_VHDL_ST_BLOCK,
   TREE_VHDL_ST_GENIF,
   TREE_VHDL_ST_GENFOR,
   TREE_VHDL_ST_INSTANCE,
   TREE_VHDL_ST_PACKAGE,

   /* GHW VHDL signal types (still as part of scope in GHW) */
   TREE_VHDL_ST_SIGNAL,
   TREE_VHDL_ST_PORTIN,
   TREE_VHDL_ST_PORTOUT,
   TREE_VHDL_ST_PORTINOUT,
   TREE_VHDL_ST_BUFFER,
   TREE_VHDL_ST_LINKAGE,

   /* FSDB VHDL scope types: FSDB also reuses/defines GHW's TREE_VHDL_ST_BLOCK, TREE_VHDL_ST_GENFOR, TREE_VHDL_ST_GENIF */
   TREE_VHDL_ST_ARCHITECTURE,
   TREE_VHDL_ST_FUNCTION,
   TREE_VHDL_ST_PROCEDURE,
   TREE_VHDL_ST_RECORD,
   TREE_VHDL_ST_PROCESS,
   TREE_VHDL_ST_GENERATE
 };

#define WAVE_T_WHICH_UNDEFINED_COMPNAME (-1)
#define WAVE_T_WHICH_COMPNAME_START (-2)

#ifdef WAVE_USE_STRUCT_PACKING
#pragma pack(push)
#pragma pack(1)
#endif

struct tree
{
struct tree *next;
struct tree *child;
int t_which;		/* 'i' for facs[i] table, value of < 0 means not a full signame */

unsigned kind : 7; 	/* Kind of the leaf: ghwlib reads this as val & 0x7f so only 7 bits needed */
unsigned children_in_gui : 1; /* indicates that the child nodes are in the gtk2 tree, but gets borrowed during tree creation for fast judy sort */
char name[1];
};

#ifdef WAVE_USE_STRUCT_PACKING
#pragma pack(pop)
#endif

/* names at the end of the main hierarchy 010104ajb    */

struct treechain
{
struct tree *tree;	/* top of list of selected item in hierarchy */
struct tree *label;	/* actual selected item in hierarchy */
struct treechain *next;
};


struct autocoalesce_free_list
{
struct autocoalesce_free_list *next;	/* list of coalesced names generated by treesearch gadget..only user of this struct */
char *name;				/* free up next time filtering is performed */
};



void init_tree(void);
void build_tree_from_name(const char *s, int which);
int treegraft(struct tree **t);
void treedebug(struct tree *t, char *s);
void maketree(GtkCTreeNode *subtree, struct tree *t);
#if WAVE_USE_GTK2
void maketree2(GtkCTreeNode *subtree, struct tree *t, int depth, GtkCTreeNode *graft);
#endif

char *leastsig_hiername(char *nam);
void allocate_and_decorate_module_tree_node(unsigned char ttype, const char *scopename, const char *compname, uint32_t scopename_len, uint32_t compname_len);
int decorated_module_cleanup(void);

void treesort(struct tree *t, struct tree *p);
void order_facs_from_treesort(struct tree *t, void *v);

void treenamefix(struct tree *t);


#ifdef WAVE_USE_STRUCT_PACKING
#define WAVE_TALLOC_POOL_SIZE (64 * 1024)
#define WAVE_TALLOC_ALTREQ_SIZE (4 * 1024)
struct tree *talloc_2(size_t siz);
#else
#define talloc_2(x) calloc_2(1,(x))
#endif

#endif

