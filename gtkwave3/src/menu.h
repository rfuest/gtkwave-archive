/* 
 * Copyright (c) Tony Bybell 1999-2008.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#include "globals.h"

/* example-start menu menufactory.h */

#ifndef __MENUFACTORY_H__
#define __MENUFACTORY_H__

#include <gtk/gtk.h>
#include <stdio.h>

#ifndef _MSC_VER
	#include <strings.h>
#endif

#include <errno.h>
#include "currenttime.h"
#include "fgetdynamic.h"
#include "strace.h"
#include "debug.h"
#include "symbol.h"
#include "main.h"

void get_main_menu (GtkWidget *, GtkWidget **menubar);
int file_quit_cmd_callback (GtkWidget *widget, gpointer data);
int set_wave_menu_accelerator(char *str);
int execute_script(char *name);
char *append_array_row(nptr n);

struct stringchain_t
{
struct stringchain_t *next;
char *name;
};


enum WV_MenuItems {
#if !defined __MINGW32__ && !defined _MSC_VER
WV_MENU_FONV,
#endif
WV_MENU_FONVT,
WV_MENU_FRW,
WV_MENU_WRVCD,
WV_MENU_WRLXT,
WV_MENU_WCLOSE,
WV_MENU_SEP2VCD,
WV_MENU_FPTF,
WV_MENU_SEP1,
WV_MENU_FRSF,
WV_MENU_FWSF,
WV_MENU_FWSFAS,
WV_MENU_SEP2,
WV_MENU_FRLF,
WV_MENU_SEP2LF,
#if !defined __MINGW32__ && !defined _MSC_VER
WV_MENU_FRSTMF,
WV_MENU_SEP2STMF,
#endif
WV_MENU_FQY,
WV_MENU_FQN,
WV_MENU_ESTMH,
WV_MENU_SEP3,
WV_MENU_EIB,
WV_MENU_EIC,
WV_MENU_EIA,
WV_MENU_EAHT,
WV_MENU_ERHA,
WV_MENU_EC,
WV_MENU_EP,
WV_MENU_SEP4,
WV_MENU_EE,
WV_MENU_ECD,
WV_MENU_ECU,
WV_MENU_ERSBV,
WV_MENU_SEP5,
WV_MENU_EDFH,
WV_MENU_EDFD,
WV_MENU_EDFSD,
WV_MENU_EDFB,
WV_MENU_EDFO,
WV_MENU_EDFA,
WV_MENU_EDRL,
WV_MENU_EDFRJON,
WV_MENU_EDFRJOFF,
WV_MENU_EDFION,
WV_MENU_EDFIOFF,
WV_MENU_EDFRON,
WV_MENU_EDFROFF,
WV_MENU_EDFAOFF,
WV_MENU_EDFASTEP,
WV_MENU_EDFAINTERPOL,
WV_MENU_EDFAINTERPOL2,
WV_MENU_EDFARSD,
WV_MENU_EDFARAD,
WV_MENU_RFILL0,
WV_MENU_RFILL1,
WV_MENU_RFILLOFF,
WV_MENU_XLF_0,
WV_MENU_XLF_1,
WV_MENU_XLP_0,
WV_MENU_XLP_1,
WV_MENU_ESCAH,
WV_MENU_ESCFH,
WV_MENU_SEP6,
WV_MENU_WARP,
WV_MENU_UNWARP,
WV_MENU_UNWARPA,
WV_MENU_SEP7A,
WV_MENU_EEX,
WV_MENU_ESH,
WV_MENU_SEP6A,
WV_MENU_EXA,
WV_MENU_CPA,
WV_MENU_SEP6A1,
WV_MENU_EHR,
WV_MENU_EUHR,
WV_MENU_EHA,
WV_MENU_EUHA,
WV_MENU_SEP6B,
WV_MENU_ALPHA,
WV_MENU_ALPHA2,
WV_MENU_LEX,
WV_MENU_RVS,
WV_MENU_SPS,
WV_MENU_SEP7B,
WV_MENU_SSR,
WV_MENU_SSH,
WV_MENU_SST,
WV_MENU_SEP7,
WV_MENU_ACOL,
WV_MENU_ACOLR,
WV_MENU_ABON,
WV_MENU_HTGP,
WV_MENU_TMTT,
WV_MENU_TZZA,
WV_MENU_TZZB,
WV_MENU_TZZI,
WV_MENU_TZZO,
WV_MENU_TZZBFL,
WV_MENU_TZZBF,
WV_MENU_TZZTS,
WV_MENU_TZZTE,
WV_MENU_TZUZ,
WV_MENU_TFFS,
WV_MENU_TFFR,
WV_MENU_TFFL,
WV_MENU_TDDR,
WV_MENU_TDDL,
WV_MENU_TSSR,
WV_MENU_TSSL,
WV_MENU_TPPR,
WV_MENU_TPPL,
WV_MENU_MSCMD,
WV_MENU_MDNM,
WV_MENU_MCNM,
WV_MENU_MCANM,
WV_MENU_MDPM,
WV_MENU_SEP8,
WV_MENU_MWSON,
WV_MENU_VSG,
WV_MENU_SEP9,
#if !defined _MSC_VER && !defined __MINGW32__
WV_MENU_VSMO,
WV_MENU_SEP9A,
#endif
WV_MENU_VSBS,
WV_MENU_SEP10,
WV_MENU_VDR,
WV_MENU_SEP11,
WV_MENU_VCZ,
WV_MENU_SEP12,
WV_MENU_VTDF,
WV_MENU_VTMM,
WV_MENU_SEP13,
WV_MENU_VCMU,
WV_MENU_SEP14,
WV_MENU_VDRV,
WV_MENU_SEP15,
WV_MENU_VLJS,
WV_MENU_VRJS,
WV_MENU_SEP16,
WV_MENU_VZPS,
WV_MENU_VFTP,
WV_MENU_SEP17,
WV_MENU_RMRKS,
WV_MENU_SEP17A,
WV_MENU_USECOLOR,
WV_MENU_USEBW,
WV_MENU_SEP18,
WV_MENU_LXTCC2Z,
WV_MENU_HWH,
WV_MENU_HWV,

WV_MENU_NUMITEMS
};


void menu_new_viewer(GtkWidget *widget, gpointer data);
void menu_write_vcd_file(GtkWidget *widget, gpointer data);
void menu_write_lxt_file(GtkWidget *widget, gpointer data);
void menu_print(GtkWidget *widget, gpointer data);
void menu_read_save_file(GtkWidget *widget, gpointer data);
void menu_write_save_file(GtkWidget *widget, gpointer data);
void menu_write_save_file_as(GtkWidget *widget, gpointer data);
void menu_read_log_file(GtkWidget *widget, gpointer data);
void menu_read_stems_file(GtkWidget *widget, gpointer data);
void menu_quit(GtkWidget *widget, gpointer data);
void menu_set_max_hier(GtkWidget *widget, gpointer data);
void menu_insert_blank_traces(GtkWidget *widget, gpointer data);
void menu_insert_comment_traces(GtkWidget *widget, gpointer data);
void menu_insert_analog_height_extension(GtkWidget *widget, gpointer data);
void menu_alias(GtkWidget *widget, gpointer data);
void menu_remove_aliases(GtkWidget *widget, gpointer data);
void menu_cut_traces(GtkWidget *widget, gpointer data);
void menu_paste_traces(GtkWidget *widget, gpointer data);
void menu_expand(GtkWidget *widget, gpointer data);
void menu_combine_down(GtkWidget *widget, gpointer data);
void menu_combine_up(GtkWidget *widget, gpointer data);
void menu_reduce_singlebit_vex(GtkWidget *widget, gpointer data);
void menu_dataformat_hex(GtkWidget *widget, gpointer data);
void menu_dataformat_dec(GtkWidget *widget, gpointer data);
void menu_dataformat_signed(GtkWidget *widget, gpointer data);
void menu_dataformat_bin(GtkWidget *widget, gpointer data);
void menu_dataformat_oct(GtkWidget *widget, gpointer data);
void menu_dataformat_ascii(GtkWidget *widget, gpointer data);
void menu_dataformat_real(GtkWidget *widget, gpointer data);
void menu_dataformat_rjustify_on(GtkWidget *widget, gpointer data);
void menu_dataformat_rjustify_off(GtkWidget *widget, gpointer data);
void menu_dataformat_invert_on(GtkWidget *widget, gpointer data);
void menu_dataformat_invert_off(GtkWidget *widget, gpointer data);
void menu_dataformat_reverse_on(GtkWidget *widget, gpointer data);
void menu_dataformat_reverse_off(GtkWidget *widget, gpointer data);
void menu_dataformat_xlate_file_0(GtkWidget *widget, gpointer data);
void menu_dataformat_xlate_file_1(GtkWidget *widget, gpointer data);
void menu_dataformat_xlate_proc_0(GtkWidget *widget, gpointer data);
void menu_dataformat_xlate_proc_1(GtkWidget *widget, gpointer data);
void menu_dataformat_analog_off(GtkWidget *widget, gpointer data);
void menu_dataformat_analog_step(GtkWidget *widget, gpointer data);
void menu_dataformat_analog_interpol(GtkWidget *widget, gpointer data);
void menu_showchangeall(GtkWidget *widget, gpointer data);
void menu_showchange(GtkWidget *widget, gpointer data);
void menu_warp_traces(GtkWidget *widget, gpointer data);
void menu_unwarp_traces(GtkWidget *widget, gpointer data);
void menu_unwarp_traces_all(GtkWidget *widget, gpointer data);
void menu_dataformat_exclude_on(GtkWidget *widget, gpointer data);
void menu_dataformat_exclude_off(GtkWidget *widget, gpointer data);
void menu_expand_all(GtkWidget *widget, gpointer data);
void menu_collapse_all(GtkWidget *widget, gpointer data);
void menu_regexp_highlight(GtkWidget *widget, gpointer data);
void menu_regexp_unhighlight(GtkWidget *widget, gpointer data);
void menu_dataformat_highlight_all(GtkWidget *widget, gpointer data);
void menu_dataformat_unhighlight_all(GtkWidget *widget, gpointer data);
void menu_alphabetize(GtkWidget *widget, gpointer data);
void menu_alphabetize2(GtkWidget *widget, gpointer data);
void menu_lexize(GtkWidget *widget, gpointer data);
void menu_reverse(GtkWidget *widget, gpointer data);
void menu_tracesearchbox(GtkWidget *widget, gpointer data);
void menu_signalsearch(GtkWidget *widget, gpointer data);
void menu_hiersearch(GtkWidget *widget, gpointer data);
void menu_treesearch(GtkWidget *widget, gpointer data);
void menu_autocoalesce(GtkWidget *widget, gpointer data);
void menu_autocoalesce_reversal(GtkWidget *widget, gpointer data);
void menu_autoname_bundles_on(GtkWidget *widget, gpointer data);
void menu_hgrouping(GtkWidget *widget, gpointer data);
void menu_movetotime(GtkWidget *widget, gpointer data);
void menu_zoomsize(GtkWidget *widget, gpointer data);
void menu_zoombase(GtkWidget *widget, gpointer data);
void menu_fetchsize(GtkWidget *widget, gpointer data);
void menu_markerbox(GtkWidget *widget, gpointer data);
void drop_named_marker(GtkWidget *widget, gpointer data);
void collect_named_marker(GtkWidget *widget, gpointer data);
void collect_all_named_markers(GtkWidget *widget, gpointer data);
void delete_unnamed_marker(GtkWidget *widget, gpointer data);
void wave_scrolling_on(GtkWidget *widget, gpointer data);
void menu_show_grid(GtkWidget *widget, gpointer data);
void menu_show_mouseover(GtkWidget *widget, gpointer data);
void menu_show_base(GtkWidget *widget, gpointer data);
void menu_enable_dynamic_resize(GtkWidget *widget, gpointer data);
void menu_center_zooms(GtkWidget *widget, gpointer data);
void menu_toggle_delta_or_frequency(GtkWidget *widget, gpointer data);
void menu_toggle_max_or_marker(GtkWidget *widget, gpointer data);
void menu_enable_constant_marker_update(GtkWidget *widget, gpointer data);
void menu_use_roundcaps(GtkWidget *widget, gpointer data);
void menu_left_justify(GtkWidget *widget, gpointer data);
void menu_right_justify(GtkWidget *widget, gpointer data);
void menu_zoom10_snap(GtkWidget *widget, gpointer data);
void menu_use_full_precision(GtkWidget *widget, gpointer data);
void menu_remove_marked(GtkWidget *widget, gpointer data);
void menu_lxt_clk_compress(GtkWidget *widget, gpointer data);
void menu_help(GtkWidget *widget, gpointer data);
void menu_version(GtkWidget *widget, gpointer data);

GtkItemFactoryEntry *retrieve_menu_items_array(int *num_items);


/* These should eventually have error values */
void write_save_helper(FILE *file);
void read_save_helper(char *wname);

#endif

/*
 * $Id$
 * $Log$
 * Revision 1.13  2008/09/24 23:41:23  gtkwave
 * drag from signal window into external process
 *
 * Revision 1.12  2008/03/25 03:22:11  gtkwave
 * expanded zero fill to include also a one fill (for pre-inverted nets)
 *
 * Revision 1.11  2008/03/24 19:34:00  gtkwave
 * added zero range fill feature
 *
 * Revision 1.10  2008/02/12 23:35:42  gtkwave
 * preparing for 3.1.5 revision bump
 *
 * Revision 1.9  2008/01/25 04:10:14  gtkwave
 * added new resizing options to menu
 *
 * Revision 1.8  2008/01/23 02:05:43  gtkwave
 * added interpolated + step mode
 *
 * Revision 1.7  2008/01/12 21:36:44  gtkwave
 * added black and white vs color rendering menu options
 *
 * Revision 1.6  2007/09/23 18:33:55  gtkwave
 * warnings cleanups from sun compile
 *
 * Revision 1.5  2007/09/09 20:10:30  gtkwave
 * preliminary support for tabbed viewing of multiple waveforms
 *
 * Revision 1.4  2007/08/26 21:35:43  gtkwave
 * integrated global context management from SystemOfCode2007 branch
 *
 * Revision 1.1.1.1.2.6  2007/08/25 19:43:45  gtkwave
 * header cleanups
 *
 * Revision 1.1.1.1.2.5  2007/08/23 02:19:49  gtkwave
 * merge GLOBALS state from old hier_search widget into new one
 *
 * Revision 1.1.1.1.2.4  2007/08/15 03:26:01  kermin
 * Reload button does not cause a fault, however, state is still somehow incorrect.
 *
 * Revision 1.1.1.1.2.3  2007/08/05 02:27:21  kermin
 * Semi working global struct
 *
 * Revision 1.1.1.1.2.2  2007/07/28 19:50:40  kermin
 * Merged in the main line
 *
 * Revision 1.3  2007/07/23 23:13:08  gtkwave
 * adds for color tags in filtered trace data
 *
 * Revision 1.2  2007/06/18 05:05:08  gtkwave
 * noticed non-extern declaraction (detected only by tcc it seems...)
 *
 * Revision 1.1.1.1  2007/05/30 04:27:58  gtkwave
 * Imported sources
 *
 * Revision 1.2  2007/04/20 02:08:13  gtkwave
 * initial release
 *
 */

