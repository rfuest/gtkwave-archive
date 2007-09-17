/* 
 * Copyright (c) Tony Bybell 1999-2006.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */


/* AIX may need this for alloca to work */ 
#if defined _AIX
  #pragma alloca
#endif

#include "globals.h"
#include <config.h>
#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>

#include "wave_locale.h"

#if !defined _MSC_VER && !defined __MINGW32__
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/types.h>
#endif

#if !defined _MSC_VER && !defined __MINGW32__ && !defined OS_X && defined WAVE_USE_GTK2
#define WAVE_USE_XID
#else
#undef WAVE_USE_XID
#endif


#ifdef HAVE_GETOPT_LONG
#include <getopt.h>
#else
#include "gnu-getopt.h"
#ifndef _MSC_VER
#include <unistd.h>
#else
#define strcasecmp _stricmp
#endif
#endif

#include "symbol.h"
#include "lx2.h"
#include "ae2.h"
#include "vzt.h"
#include "ghw.h"
#include "main.h"
#include "menu.h"
#include "vcd.h"
#include "lxt.h"
#include "lxt2_read.h"
#include "vzt_read.h"
#include "pixmaps.h"
#include "currenttime.h"
#include "fgetdynamic.h"
#include "rc.h"
#include "translate.h"
#include "ptranslate.h"

static void switch_page(GtkNotebook     *notebook,
			GtkNotebookPage *page,
			guint            page_num,
			gpointer         user_data) 
{
int i;
char timestr[32];
struct Global *g_old = GLOBALS;

GLOBALS = (*GLOBALS->contexts)[page_num];

GLOBALS->lxt_clock_compress_to_z = g_old->lxt_clock_compress_to_z;
GLOBALS->autoname_bundles = g_old->autoname_bundles;
GLOBALS->autocoalesce_reversal = g_old->autocoalesce_reversal;
GLOBALS->autocoalesce = g_old->autocoalesce;
GLOBALS->hier_grouping = g_old->hier_grouping;
GLOBALS->wave_scrolling = g_old->wave_scrolling;
GLOBALS->constant_marker_update = g_old->constant_marker_update;
GLOBALS->do_zoom_center = g_old->do_zoom_center;
GLOBALS->use_roundcaps = g_old->use_roundcaps;
GLOBALS->do_resize_signals = g_old->do_resize_signals;
GLOBALS->use_full_precision = g_old->use_full_precision;
GLOBALS->show_base = g_old->show_base;
GLOBALS->display_grid = g_old->display_grid;
GLOBALS->disable_mouseover = g_old->disable_mouseover;
GLOBALS->zoom_pow10_snap = g_old->zoom_pow10_snap;

reformat_time(timestr, GLOBALS->tims.first, GLOBALS->time_dimension);
gtk_entry_set_text(GTK_ENTRY(GLOBALS->from_entry),timestr);
reformat_time(timestr, GLOBALS->tims.last, GLOBALS->time_dimension);
gtk_entry_set_text(GTK_ENTRY(GLOBALS->to_entry),timestr);

update_maxmarker_labels();
update_basetime(GLOBALS->tims.baseline);

if(GLOBALS->second_page_created)
	{
	gtk_window_set_title(GTK_WINDOW(GLOBALS->mainwindow), GLOBALS->winname);

	MaxSignalLength();
	signalarea_configure_event(GLOBALS->signalarea, NULL);
	wavearea_configure_event(GLOBALS->wavearea, NULL);
	}
}


#if !defined _MSC_VER && !defined __MINGW32__
void kill_stems_browser(void)
{
if(GLOBALS->anno_ctx)
	{
	if(GLOBALS->anno_ctx->browser_process)
		{
		kill(GLOBALS->anno_ctx->browser_process, SIGKILL);
		GLOBALS->anno_ctx->browser_process = (pid_t)0;
		}
	GLOBALS->anno_ctx = NULL;
	}
}
#endif


/* for XID plug handling... */
#ifdef WAVE_USE_XID
#else
unsigned int socket_xid = 0;
#endif

static int plug_destroy (GtkWidget *widget, gpointer data)
{
exit(0);

return(FALSE);
}       


static void print_help(char *nam)
{
#if !defined _MSC_VER && !defined __MINGW32__ && !defined __FreeBSD__ && !defined __CYGWIN__
#define WAVE_GETOPT_CPUS "  -c, --cpu=NUMCPUS          specify number of CPUs for parallelizable ops\n"
#else
#define WAVE_GETOPT_CPUS
#endif

#if !defined _MSC_VER && !defined __MINGW32__
#define STEMS_GETOPT	 "  -t, --stems=FILE           specify stems file for source code annotation\n"
#define VCD_GETOPT       "  -o, --optimize             optimize VCD to LXT2\n"
#define DUAL_GETOPT      "  -D, --dualid=WHICH         specify multisession identifier\n"
#define INTR_GETOPT      "  -I, --interactive          interactive VCD mode (filename is shared mem ID)\n"
#else
#define STEMS_GETOPT
#define VCD_GETOPT
#define DUAL_GETOPT
#define INTR_GETOPT
#endif


#ifdef WAVE_USE_XID
#define XID_GETOPT 	 "  -X, --xid=XID              specify XID of window for GtkPlug to connect to\n"
#else
#define XID_GETOPT
#endif

printf(
"Usage: %s [OPTION]... [DUMPFILE] [SAVEFILE] [RCFILE]\n\n"
"  -n, --nocli=DIRPATH        use file requester for dumpfile name\n"
"  -f, --dump=FILE            specify dumpfile name\n"
VCD_GETOPT
"  -a, --save=FILE            specify savefile name\n"
"  -A, --autosavename         assume savefile is suffix modified dumpfile name\n"
"  -r, --rcfile=FILE          specify override .rcfile name\n"
"  -d, --defaultskip          if missing .rcfile, do not use useful defaults\n"
DUAL_GETOPT
"  -i, --indirect=FILE        specify indirect facs file name\n"
"  -l, --logfile=FILE         specify simulation logfile name for time values\n"
"  -s, --start=TIME           specify start time for LXT2/VZT block skip\n"
"  -e, --end=TIME             specify end time for LXT2/VZT block skip\n"  
STEMS_GETOPT
WAVE_GETOPT_CPUS
"  -N, --nowm                 disable window manager for most windows\n"
"  -M, --nomenus              do not render menubar (for making applets)\n"
"  -S, --script=FILE          specify GUI command script file for execution\n"
XID_GETOPT
INTR_GETOPT
"  -L, --legacy               use legacy VCD mode rather than the VCD recoder\n" 
"  -v, --vcd                  use stdin as a VCD dumpfile\n"
"  -V, --version              display version banner then exit\n"
"  -h, --help                 display this help then exit\n"
"  -x  --exit                 exit after loading trace (for loader benchmarks)\n\n"

"VCD files and save files may be compressed with zip or gzip.\n"
"GHW files may be compressed with gzip or bzip2.\n"
"Other formats must remain uncompressed due to their non-linear access.\n"
"Note that DUMPFILE is optional if the --dump or --nocli options are specified.\n"
"SAVEFILE and RCFILE are always optional.\n\n"

"Report bugs to <bybell@nc.rr.com>.\n",nam);

exit(0);
}


/*
 * file selection for -n/--nocli flag 
 */

static void wave_get_filename_cleanup(GtkWidget *widget, gpointer data) { gtk_main_quit(); /* do nothing but exit gtk loop */ }

static char *wave_get_filename(char *dfile)
{
if(dfile) 
	{
	int len = strlen(dfile);
	GLOBALS->ftext_main_main_c_1 = malloc_2(strlen(dfile)+2);
	strcpy(GLOBALS->ftext_main_main_c_1, dfile);
#if !defined _MSC_VER && !defined __MINGW32__
	if((len)&&(dfile[len-1]!='/'))
		{
		strcpy(GLOBALS->ftext_main_main_c_1 + len, "/");
		}
#else
	if((len)&&(dfile[len-1]!='\\'))
		{
		strcpy(GLOBALS->ftext_main_main_c_1 + len, "\\");
		}
#endif
	}
fileselbox_old("GTKWave: Select a dumpfile...",&GLOBALS->ftext_main_main_c_1,GTK_SIGNAL_FUNC(wave_get_filename_cleanup), GTK_SIGNAL_FUNC(wave_get_filename_cleanup), NULL);
gtk_main();

return(GLOBALS->ftext_main_main_c_1);
}


int main(int argc, char *argv[])
{
static char *winprefix="GTKWave - ";
static char *winstd="GTKWave (stdio) ";
static char *vcd_autosave_name="vcd_autosave.sav";

int i;
int c;
char is_vcd=0;
char is_interactive=0;
char is_smartsave = 0;
char is_legacy = 0;
char fast_exit=0;
char opt_errors_encountered=0;

char *wname=NULL;
char *override_rc=NULL;
char *scriptfile=NULL;
FILE *wave = NULL;
FILE *vcd_save_handle_cached = NULL;

GtkWidget *main_vbox, *top_table, *whole_table;
GtkWidget *menubar;
GtkWidget *text1;
GtkWidget *zoombuttons;
GtkWidget *pagebuttons;
GtkWidget *fetchbuttons;
GtkWidget *discardbuttons;
GtkWidget *shiftbuttons;
GtkWidget *entry;
GtkWidget *timebox;
GtkWidget *panedwindow;
GtkWidget *dummy1, *dummy2;
GtkWidget *toolhandle=NULL;

int splash_disable_rc_override = 0;
int mainwindow_already_built;

WAVE_LOCALE_FIX

/* Initialize the GLOBALS structure for the first time... */ 
if(!GLOBALS)
	{
	GLOBALS = initialize_globals();
	mainwindow_already_built = 0;
	}
	else
	{
	struct Global *old_g = GLOBALS;

	GLOBALS = initialize_globals();

	GLOBALS->second_page_created = old_g->second_page_created = 1;

	GLOBALS->notebook = old_g->notebook;
	GLOBALS->num_notebook_pages = old_g->num_notebook_pages;
	GLOBALS->num_notebook_pages_cumulative = old_g->num_notebook_pages_cumulative;
	GLOBALS->contexts = old_g->contexts;

	GLOBALS->mainwindow = old_g->mainwindow;
	splash_disable_rc_override = 1;

	/* busy.c */
	GLOBALS->busycursor_busy_c_1 = old_g->busycursor_busy_c_1;

	/* menu.c */
	GLOBALS->item_factory_menu_c_1 = old_g->item_factory_menu_c_1;
	GLOBALS->vcd_jmp_buf = old_g->vcd_jmp_buf;

	/* currenttime.c */
	GLOBALS->max_or_marker_label_currenttime_c_1 = old_g->max_or_marker_label_currenttime_c_1;
	GLOBALS->maxtext_currenttime_c_1=(char *)malloc_2(40);
	GLOBALS->maxtimewid_currenttime_c_1 = old_g->maxtimewid_currenttime_c_1;
	GLOBALS->curtext_currenttime_c_1 = old_g->curtext_currenttime_c_1;
	GLOBALS->base_or_curtime_label_currenttime_c_1 = old_g->base_or_curtime_label_currenttime_c_1;
	GLOBALS->curtimewid_currenttime_c_1 = old_g->curtimewid_currenttime_c_1;

	/* status.c */
	GLOBALS->text_status_c_2 = old_g->text_status_c_2;
	GLOBALS->vscrollbar_status_c_2 = old_g->vscrollbar_status_c_2;
#if defined(WAVE_USE_GTK2) && !defined(GTK_ENABLE_BROKEN)
	GLOBALS->iter_status_c_3 = old_g->iter_status_c_3;
	GLOBALS->bold_tag_status_c_3 = old_g->bold_tag_status_c_3;
#endif

	/* timeentry.c */
	GLOBALS->from_entry = old_g->from_entry;
	GLOBALS->to_entry = old_g->to_entry;
	
	/* rc.c */
	GLOBALS->ignore_savefile_pos = old_g->ignore_savefile_pos;
	GLOBALS->ignore_savefile_size = old_g->ignore_savefile_size;
	
	GLOBALS->color_back = old_g->color_back;
	GLOBALS->color_baseline = old_g->color_baseline;
	GLOBALS->color_grid = old_g->color_grid;
	GLOBALS->color_high = old_g->color_high;
	GLOBALS->color_low = old_g->color_low;
	GLOBALS->color_1 = old_g->color_1;
	GLOBALS->color_0 = old_g->color_0;
	GLOBALS->color_mark = old_g->color_mark;
	GLOBALS->color_mid = old_g->color_mid;
	GLOBALS->color_time = old_g->color_time;
	GLOBALS->color_timeb = old_g->color_timeb;
	GLOBALS->color_trans = old_g->color_trans;
	GLOBALS->color_umark = old_g->color_umark;
	GLOBALS->color_value = old_g->color_value;
	GLOBALS->color_vbox = old_g->color_vbox;
	GLOBALS->color_vtrans = old_g->color_vtrans;
	GLOBALS->color_x = old_g->color_x;
	GLOBALS->color_xfill = old_g->color_xfill;
	GLOBALS->color_u = old_g->color_u;
	GLOBALS->color_ufill = old_g->color_ufill;
	GLOBALS->color_w = old_g->color_w;
	GLOBALS->color_wfill = old_g->color_wfill;
	GLOBALS->color_dash = old_g->color_dash;
	GLOBALS->color_dashfill = old_g->color_dashfill;
	GLOBALS->color_white = old_g->color_white;
	GLOBALS->color_black = old_g->color_black;
	GLOBALS->color_ltgray = old_g->color_ltgray;
	GLOBALS->color_normal = old_g->color_normal;
	GLOBALS->color_mdgray = old_g->color_mdgray;
	GLOBALS->color_dkgray = old_g->color_dkgray;
	GLOBALS->color_dkblue = old_g->color_dkblue;
	
	GLOBALS->atomic_vectors = old_g->atomic_vectors;
	GLOBALS->autoname_bundles = old_g->autoname_bundles;
	GLOBALS->autocoalesce = old_g->autocoalesce;
	GLOBALS->autocoalesce_reversal = old_g->autocoalesce_reversal;
	GLOBALS->constant_marker_update = old_g->constant_marker_update;
	GLOBALS->convert_to_reals = old_g->convert_to_reals;
	GLOBALS->disable_mouseover = old_g->disable_mouseover;
	GLOBALS->disable_tooltips = old_g->disable_tooltips;
	GLOBALS->do_initial_zoom_fit = old_g->do_initial_zoom_fit;
	GLOBALS->do_resize_signals = old_g->do_resize_signals;
	GLOBALS->enable_fast_exit = old_g->enable_fast_exit;
	GLOBALS->enable_ghost_marker = old_g->enable_ghost_marker;
	GLOBALS->enable_horiz_grid = old_g->enable_horiz_grid;
	GLOBALS->make_vcd_save_file = old_g->make_vcd_save_file;
	GLOBALS->enable_vert_grid = old_g->enable_vert_grid;
	GLOBALS->force_toolbars = old_g->force_toolbars;
	GLOBALS->hide_sst = old_g->hide_sst;
	GLOBALS->sst_expanded = old_g->sst_expanded;
	GLOBALS->hier_grouping = old_g->hier_grouping;
	GLOBALS->hier_max_level = old_g->hier_max_level;
	GLOBALS->paned_pack_semantics = old_g->paned_pack_semantics;
	GLOBALS->left_justify_sigs = old_g->left_justify_sigs;
	GLOBALS->lxt_clock_compress_to_z = old_g->lxt_clock_compress_to_z;
	GLOBALS->ps_maxveclen = old_g->ps_maxveclen;
	GLOBALS->show_base = old_g->show_base;
	GLOBALS->display_grid = old_g->display_grid;
	GLOBALS->use_big_fonts = old_g->use_big_fonts;
	GLOBALS->use_full_precision = old_g->use_full_precision;
	GLOBALS->use_frequency_delta = old_g->use_frequency_delta;
	GLOBALS->use_maxtime_display = old_g->use_maxtime_display;
	GLOBALS->use_nonprop_fonts = old_g->use_nonprop_fonts;
	GLOBALS->use_roundcaps = old_g->use_roundcaps;
	GLOBALS->use_scrollbar_only = old_g->use_scrollbar_only;
	GLOBALS->vcd_explicit_zero_subscripts = old_g->vcd_explicit_zero_subscripts;
	GLOBALS->vcd_preserve_glitches = old_g->vcd_preserve_glitches;
	GLOBALS->vcd_warning_filesize = old_g->vcd_warning_filesize;
	GLOBALS->vector_padding = old_g->vector_padding;
	GLOBALS->vlist_compression_depth = old_g->vlist_compression_depth;
	GLOBALS->wave_scrolling = old_g->wave_scrolling;
	GLOBALS->do_zoom_center = old_g->do_zoom_center;
	GLOBALS->zoom_pow10_snap = old_g->zoom_pow10_snap;
	GLOBALS->alt_hier_delimeter = old_g->alt_hier_delimeter;
	GLOBALS->cursor_snap = old_g->cursor_snap;
	GLOBALS->hier_delimeter = old_g->hier_delimeter;
	GLOBALS->hier_was_explicitly_set = old_g->hier_was_explicitly_set;
	GLOBALS->page_divisor = old_g->page_divisor;
	GLOBALS->ps_maxveclen = old_g->ps_maxveclen;
	GLOBALS->vector_padding = old_g->vector_padding;
	GLOBALS->vlist_compression_depth = old_g->vlist_compression_depth;
	GLOBALS->zoombase = old_g->zoombase;
	GLOBALS->splash_disable = old_g->splash_disable;
	
	strcpy2_into_new_context(GLOBALS, &GLOBALS->fontname_logfile, &old_g->fontname_logfile);
	strcpy2_into_new_context(GLOBALS, &GLOBALS->fontname_signals, &old_g->fontname_signals); 
	strcpy2_into_new_context(GLOBALS, &GLOBALS->fontname_waves, &old_g->fontname_waves);

	mainwindow_already_built = 1;
	}

GLOBALS->whoami=malloc_2(strlen(argv[0])+1);	/* cache name in case we fork later */
strcpy(GLOBALS->whoami, argv[0]);

if(!mainwindow_already_built)
	{
	if(!gtk_init_check(&argc, &argv))
		{
		printf("Could not initialize GTK!  Is DISPLAY env var/xhost set?\n\n");
		print_help(argv[0]);
		}
	}

init_filetrans_data(); /* for file translation splay trees */
init_proctrans_data(); /* for proc translation structs */
if(!mainwindow_already_built) atexit(remove_all_proc_filters);

if(mainwindow_already_built)
	{
	optind = 1;
	}
else
while (1)
        {
        int option_index = 0;

        static struct option long_options[] =
                {
                {"dump", 1, 0, 'f'},
                {"optimize", 0, 0, 'o'},
                {"nocli", 1, 0, 'n'},
                {"save", 1, 0, 'a'},
                {"autosavename", 0, 0, 'A'},
                {"rcfile", 1, 0, 'r'},
                {"defaultskip", 0, 0, 'd'},
                {"indirect", 1, 0, 'i'},
                {"logfile", 1, 0, 'l'},
                {"start", 1, 0, 's'},
                {"end", 1, 0, 'e'},
                {"cpus", 1, 0, 'c'},
                {"stems", 1, 0, 't'},
                {"nowm", 0, 0, 'N'},
                {"script", 1, 0, 'S'},
                {"vcd", 0, 0, 'v'},
                {"version", 0, 0, 'V'},
                {"help", 0, 0, 'h'},
                {"exit", 0, 0, 'x'},
                {"xid", 1, 0, 'X'},
                {"nomenus", 0, 0, 'M'},
                {"dualid", 1, 0, 'D'},
                {"interactive", 0, 0, 'I'},
                {"legacy", 0, 0, 'L'},  
                {0, 0, 0, 0}
                };

        c = getopt_long (argc, argv, "f:on:a:Ar:di:l:s:e:c:t:NS:vVhxX:MD:IL", long_options, &option_index);

        if (c == -1) break;     /* no more args */

        switch (c)
                {
		case 'V':
			printf(
			WAVE_VERSION_INFO"\n\n"
			"This is free software; see the source for copying conditions.  There is NO\n"
			"warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n"
			);
			exit(0);

		case 'I':
#if !defined _MSC_VER && !defined __MINGW32__
			is_interactive = 1;
#endif
			break;

		case 'L':
			is_legacy = 1;
			break;

		case 'D':
#if !defined _MSC_VER && !defined __MINGW32__
			{
			char *s = optarg;
			char *plus = strchr(s, '+');
			if((plus)&&(*(plus+1)))
				{
				sscanf(plus+1, "%x", &GLOBALS->dual_attach_id_main_c_1);
				if(plus != s)
					{
					char p = *(plus-1);

					if(p=='0')
						{
						GLOBALS->dual_id = 0;
						break;
						}
					else
					if(p=='1')
						{
						GLOBALS->dual_id = 1;
						break;
						}
					}
				}

			fprintf(stderr, "Malformed dual session ID.  Must be of form m+nnnnnnnn where m is 0 or 1,\n"
					"and n is a hexadecimal shared memory ID for use with shmat()\n");
			exit(255);
			}
#else
			{
			fprintf(stderr, "Dual operation not implemented for Win32, exiting.\n");
			exit(255);
			}
#endif
			break;

		case 'A':
			is_smartsave = 1;
			break;

                case 'v':
			is_vcd = 1;
			if(GLOBALS->loaded_file_name) free_2(GLOBALS->loaded_file_name);
			GLOBALS->loaded_file_name = malloc_2(4+1);
			strcpy(GLOBALS->loaded_file_name, "-vcd");
                        break;

		case 'o':
			GLOBALS->optimize_vcd = 1;
			break;

		case 'n':
			wave_get_filename(optarg);
			if(GLOBALS->filesel_ok)
				{
				if(GLOBALS->loaded_file_name) free_2(GLOBALS->loaded_file_name);
				GLOBALS->loaded_file_name = GLOBALS->ftext_main_main_c_1;
				GLOBALS->ftext_main_main_c_1 = NULL;
				}
			break;

                case 'h':
			print_help(argv[0]);
                        break;

#ifdef WAVE_USE_XID
                case 'X': 
                        sscanf(optarg, "%x", &GLOBALS->socket_xid);
			splash_disable_rc_override = 1;
                        break;
#endif

		case 'M':
			GLOBALS->disable_menus = 1;
			break;

                case 'x':
			fast_exit = 1;
			splash_disable_rc_override = 1;
                        break;

		case 'd':
			GLOBALS->possibly_use_rc_defaults = 0;
			break;

                case 'f':
			is_vcd = 0;
			if(GLOBALS->loaded_file_name) free_2(GLOBALS->loaded_file_name);
			GLOBALS->loaded_file_name = malloc_2(strlen(optarg)+1);
			strcpy(GLOBALS->loaded_file_name, optarg);
			break;

                case 'a':
			if(wname) free_2(wname);
			wname = malloc_2(strlen(optarg)+1);
			strcpy(wname, optarg);
			break;

                case 'r':
			if(override_rc) free_2(override_rc);
			override_rc = malloc_2(strlen(optarg)+1);
			strcpy(override_rc, optarg);
			break;

                case 'i':
			if(GLOBALS->indirect_fname) free_2(GLOBALS->indirect_fname);
			GLOBALS->indirect_fname = malloc_2(strlen(optarg)+1);
			strcpy(GLOBALS->indirect_fname, optarg);
			break;

                case 's':
			if(GLOBALS->skip_start) free_2(GLOBALS->skip_start);
			GLOBALS->skip_start = malloc_2(strlen(optarg)+1);
			strcpy(GLOBALS->skip_start, optarg);
			break;			

                case 'e':
			if(GLOBALS->skip_end) free_2(GLOBALS->skip_end);
			GLOBALS->skip_end = malloc_2(strlen(optarg)+1);
			strcpy(GLOBALS->skip_end, optarg);
                        break;

		case 't':
#if !defined _MSC_VER && !defined __MINGW32__
			if(GLOBALS->stems_name) free_2(GLOBALS->stems_name);
			GLOBALS->stems_name = malloc_2(strlen(optarg)+1);
			strcpy(GLOBALS->stems_name, optarg);
#endif
			break;

                case 'c':
#if !defined _MSC_VER && !defined __MINGW32__ && !defined __FreeBSD__ && !defined __CYGWIN__
			GLOBALS->num_cpus = atoi(optarg);
			if(GLOBALS->num_cpus<1) GLOBALS->num_cpus = 1;
			if(GLOBALS->num_cpus>8) GLOBALS->num_cpus = 8;
#endif
                        break;

		case 'N':
			GLOBALS->disable_window_manager = 1;
			break;

		case 'S':
			if(scriptfile) free_2(scriptfile);
			scriptfile = malloc_2(strlen(optarg)+1);
			strcpy(scriptfile, optarg);
			splash_disable_rc_override = 1;
			break;

		case 'l':
			{
			struct logfile_chain *l = calloc_2(1, sizeof(struct logfile_chain));
			struct logfile_chain *ltraverse;
			l->name = malloc_2(strlen(optarg)+1);
			strcpy(l->name, optarg);

			if(GLOBALS->logfile)
				{
				ltraverse = GLOBALS->logfile;
				while(ltraverse->next) ltraverse = ltraverse->next;
				ltraverse->next = l;
				}
				else
				{
				GLOBALS->logfile = l;
				}
			}
			break;

                case '?':
			opt_errors_encountered=1;
                        break;

                default:
                        /* unreachable */
                        break;
                }
        } /* ...while(1) */

if(opt_errors_encountered)
	{
	print_help(argv[0]);
	}

if (optind < argc)
        {
        while (optind < argc)
		{
		if(!GLOBALS->loaded_file_name)
			{
			is_vcd = 0;
			GLOBALS->loaded_file_name = malloc_2(strlen(argv[optind])+1);
			strcpy(GLOBALS->loaded_file_name, argv[optind++]);
			}
		else if(!wname)
			{
			wname = malloc_2(strlen(argv[optind])+1);
			strcpy(wname, argv[optind++]);
			}
		else if(!override_rc)
			{
			override_rc = malloc_2(strlen(argv[optind])+1);
			strcpy(override_rc, argv[optind++]);
			break; /* skip any extra args */
			}
		}
        }

if(!GLOBALS->loaded_file_name)
	{
	print_help(argv[0]);
	}
read_rc_file(override_rc);
GLOBALS->splash_disable |= splash_disable_rc_override;


fprintf(stderr, "\n%s\n\n",WAVE_VERSION_INFO);

if((!wname)&&(GLOBALS->make_vcd_save_file))
	{
	vcd_save_handle_cached = GLOBALS->vcd_save_handle=fopen(vcd_autosave_name,"wb");
	errno=0;	/* just in case */
	is_smartsave = (GLOBALS->vcd_save_handle != NULL); /* use smartsave if for some reason can't open auto savefile */
	}

GLOBALS->sym=(struct symbol **)calloc_2(SYMPRIME,sizeof(struct symbol *));

/* load either the vcd or aet file depending on suffix then mode setting */
if(is_vcd)
	{
	GLOBALS->winname=malloc_2(strlen(winstd)+4+1);
	strcpy(GLOBALS->winname,winstd);
	}
	else
	{
	if(!is_interactive)
		{
		GLOBALS->winname=malloc_2(strlen(GLOBALS->loaded_file_name)+strlen(winprefix)+1);
		strcpy(GLOBALS->winname,winprefix);
		}
		else
		{
		char *iact = "GTKWave - Interactive Shared Memory ID ";
		GLOBALS->winname=malloc_2(strlen(GLOBALS->loaded_file_name)+strlen(iact)+1);
		strcpy(GLOBALS->winname,iact);
		}
	}

strcat(GLOBALS->winname,GLOBALS->loaded_file_name);

loader_check_head:

if((strlen(GLOBALS->loaded_file_name)>3)&&((!strcasecmp(GLOBALS->loaded_file_name+strlen(GLOBALS->loaded_file_name)-4,".lxt"))||(!strcasecmp(GLOBALS->loaded_file_name+strlen(GLOBALS->loaded_file_name)-4,".lx2"))))
	{
	FILE *f = fopen(GLOBALS->loaded_file_name, "rb");
	int typ = 0;

	if(f)
		{
		char buf[2];
		unsigned int matchword;

		if(fread(buf, 2, 1, f))
			{
			matchword = (((unsigned int)buf[0])<<8) | ((unsigned int)buf[1]);
			if(matchword == LT_HDRID) typ = 1;
			}

		fclose(f);
		}

	if(typ)
		{
	          GLOBALS->loaded_file_type = LXT_FILE;
		  lxt_main(GLOBALS->loaded_file_name);
		}
		else
		{
#if !defined _MSC_VER && !defined __MINGW32__
		GLOBALS->stems_type = WAVE_ANNO_LXT2;
		GLOBALS->aet_name = malloc_2(strlen(GLOBALS->loaded_file_name)+1);
		strcpy(GLOBALS->aet_name, GLOBALS->loaded_file_name);
#endif
                GLOBALS->loaded_file_type = LX2_FILE;
		lx2_main(GLOBALS->loaded_file_name, GLOBALS->skip_start, GLOBALS->skip_end);
		if(!GLOBALS->lx2_lx2_c_1)
			{
			fprintf(stderr, "Could not initialize '%s', exiting.\n", GLOBALS->loaded_file_name);
			exit(255);
			}
		}	
	}
else
if((strlen(GLOBALS->loaded_file_name)>3)&&(!strcasecmp(GLOBALS->loaded_file_name+strlen(GLOBALS->loaded_file_name)-4,".vzt")))
	{
#if !defined _MSC_VER && !defined __MINGW32__
	GLOBALS->stems_type = WAVE_ANNO_VZT;
	GLOBALS->aet_name = malloc_2(strlen(GLOBALS->loaded_file_name)+1);
	strcpy(GLOBALS->aet_name, GLOBALS->loaded_file_name);
#endif
        GLOBALS->loaded_file_type = VZT_FILE;
	vzt_main(GLOBALS->loaded_file_name, GLOBALS->skip_start, GLOBALS->skip_end);
	if(!GLOBALS->vzt_vzt_c_1)
		{
		fprintf(stderr, "Could not initialize '%s', exiting.\n", GLOBALS->loaded_file_name);
		exit(255);
		}
	}
else if ((strlen(GLOBALS->loaded_file_name)>3)&&((!strcasecmp(GLOBALS->loaded_file_name+strlen(GLOBALS->loaded_file_name)-4,".aet"))||(!strcasecmp(GLOBALS->loaded_file_name+strlen(GLOBALS->loaded_file_name)-4,".ae2"))))
	{
#if !defined _MSC_VER && !defined __MINGW32__
	GLOBALS->stems_type = WAVE_ANNO_AE2;
	GLOBALS->aet_name = malloc_2(strlen(GLOBALS->loaded_file_name)+1);
	strcpy(GLOBALS->aet_name, GLOBALS->loaded_file_name);
#endif
        GLOBALS->loaded_file_type = AE2_FILE;
	ae2_main(GLOBALS->loaded_file_name, GLOBALS->skip_start, GLOBALS->skip_end, GLOBALS->indirect_fname);
	if(!GLOBALS->ae2)
		{
		fprintf(stderr, "Could not initialize '%s', exiting.\n", GLOBALS->loaded_file_name);
		exit(255);
		}
	}
else if (
	((strlen(GLOBALS->loaded_file_name)>3)&&(!strcasecmp(GLOBALS->loaded_file_name+strlen(GLOBALS->loaded_file_name)-4,".ghw"))) ||
	((strlen(GLOBALS->loaded_file_name)>6)&&(!strcasecmp(GLOBALS->loaded_file_name+strlen(GLOBALS->loaded_file_name)-7,".ghw.gz"))) ||
	((strlen(GLOBALS->loaded_file_name)>7)&&(!strcasecmp(GLOBALS->loaded_file_name+strlen(GLOBALS->loaded_file_name)-8,".ghw.bz2")))
	)
	{
          GLOBALS->loaded_file_type = GHW_FILE;
	  if(!ghw_main(GLOBALS->loaded_file_name))
		{
		/* error message printed in ghw_main() */
		exit(255);
		}
	}
else if (strlen(GLOBALS->loaded_file_name)>4)	/* case for .aet? type filenames */
	{
	char sufbuf[5];
	memcpy(sufbuf, GLOBALS->loaded_file_name+strlen(GLOBALS->loaded_file_name)-5, 4);
	sufbuf[4] = 0;
	if(!strcasecmp(sufbuf, ".aet"))	/* strncasecmp() in windows? */
		{
#if !defined _MSC_VER && !defined __MINGW32__
		GLOBALS->stems_type = WAVE_ANNO_AE2;
		GLOBALS->aet_name = malloc_2(strlen(GLOBALS->loaded_file_name)+1);
		strcpy(GLOBALS->aet_name, GLOBALS->loaded_file_name);
#endif
                GLOBALS->loaded_file_type = AE2_FILE;
		ae2_main(GLOBALS->loaded_file_name, GLOBALS->skip_start, GLOBALS->skip_end, GLOBALS->indirect_fname);
		if(!GLOBALS->ae2)
			{
			fprintf(stderr, "Could not initialize '%s', exiting.\n", GLOBALS->loaded_file_name);
			exit(255);
			}
		}
		else
		{
		goto load_vcd;
		}
	}
else	/* nothing else left so default to "something" */
	{
load_vcd:
#if !defined _MSC_VER && !defined __MINGW32__
	if(GLOBALS->optimize_vcd) {
                  GLOBALS->unoptimized_vcd_file_name = calloc_2(1,strlen(GLOBALS->loaded_file_name) + 1);
                  strcpy(GLOBALS->unoptimized_vcd_file_name, GLOBALS->loaded_file_name);  
                  optimize_vcd_file();
                  is_vcd = 0;
                  goto loader_check_head;
        }

#endif

#if !defined _MSC_VER && !defined __MINGW32__
	if(is_interactive)
		{
		GLOBALS->loaded_file_type = NO_FILE;
		vcd_partial_main(GLOBALS->loaded_file_name);
		}
		else
#endif
		{
		if(is_legacy)
			{
			  GLOBALS->loaded_file_type = (strcmp(GLOBALS->loaded_file_name, "-vcd")) ? VCD_FILE : NO_FILE;
			  vcd_main(GLOBALS->loaded_file_name);
			}
			else
			{
			  GLOBALS->loaded_file_type = (strcmp(GLOBALS->loaded_file_name, "-vcd")) ? VCD_RECODER_FILE : NO_FILE;
			  vcd_recoder_main(GLOBALS->loaded_file_name);
			}
		}
	}


for(i=0;i<26;i++) GLOBALS->named_markers[i]=-1;	/* reset all named markers */

GLOBALS->tims.last=GLOBALS->max_time;
GLOBALS->tims.end=GLOBALS->tims.last;		/* until the configure_event of wavearea */
GLOBALS->tims.first=GLOBALS->tims.start=GLOBALS->tims.laststart=GLOBALS->min_time;
GLOBALS->tims.zoom=GLOBALS->tims.prevzoom=0;	/* 1 pixel/ns default */
GLOBALS->tims.marker=GLOBALS->tims.lmbcache=-1;	/* uninitialized at first */
GLOBALS->tims.baseline=-1;		/* middle button toggle marker */

if((wname)||(vcd_save_handle_cached)||(is_smartsave))
	{
	int wave_is_compressed;
        char *str = NULL;

	if(vcd_save_handle_cached)
		{
		wname=vcd_autosave_name;
		GLOBALS->do_initial_zoom_fit=1;
		}
	else
	if((!wname) /* && (is_smartsave) */)
		{
		char *pnt = wave_alloca(strlen(GLOBALS->loaded_file_name) + 1);
		char *pnt2;
		strcpy(pnt, GLOBALS->loaded_file_name);

	        if((strlen(pnt)>2)&&(!strcasecmp(pnt+strlen(pnt)-3,".gz")))
			{
			pnt[strlen(pnt)-3] = 0x00;
			}
		else if ((strlen(pnt)>3)&&(!strcasecmp(pnt+strlen(pnt)-4,".zip")))
			{
			pnt[strlen(pnt)-4] = 0x00;
			}

		pnt2 = pnt + strlen(pnt);
		if(pnt != pnt2)
			{
			do
				{
				if(*pnt2 == '.')
					{
					*pnt2 = 0x00;
					break;
					}
				} while(pnt2-- != pnt);
			}	

		wname = malloc_2(strlen(pnt) + 5);
		strcpy(wname, pnt);
		strcat(wname, ".sav");
		}

	if(((strlen(wname)>2)&&(!strcasecmp(wname+strlen(wname)-3,".gz")))||
	   ((strlen(wname)>3)&&(!strcasecmp(wname+strlen(wname)-4,".zip"))))
	        {
        	int dlen;
        	dlen=strlen(WAVE_DECOMPRESSOR);
	        str=wave_alloca(strlen(wname)+dlen+1);
	        strcpy(str,WAVE_DECOMPRESSOR);
	        strcpy(str+dlen,wname);
	        wave=popen(str,"r");
	        wave_is_compressed=~0;
	        }
	        else
	        {
	        wave=fopen(wname,"rb");
	        wave_is_compressed=0;

		GLOBALS->filesel_writesave = malloc_2(strlen(wname)+1); /* don't handle compressed files */
		strcpy(GLOBALS->filesel_writesave, wname);
	        }

	if(!wave)
	        {
	        fprintf(stderr, "** WARNING: Error opening .sav file '%s', skipping.\n",wname);
	        }	
	        else
	        {
	        char *iline;
		char any_shadow = 0;

		if(GLOBALS->is_lx2)
			{
		        while((iline=fgetmalloc(wave)))
		                {
		                parsewavline_lx2(iline, 0);
				free_2(iline);
		                }

			switch(GLOBALS->is_lx2)
				{
				case LXT2_IS_LXT2: lx2_import_masked(); break;
				case LXT2_IS_AET2: ae2_import_masked(); break;
				case LXT2_IS_VZT:  vzt_import_masked(); break;
				case LXT2_IS_VLIST: vcd_import_masked(); break;
				}

			if(wave_is_compressed)
			        {
				pclose(wave);
			        wave=popen(str,"r");
			        }
			        else
			        {
				fclose(wave);
			        wave=fopen(wname,"rb");
			        }

			if(!wave)
			        {
			        fprintf(stderr, "** WARNING: Error opening .sav file '%s', skipping.\n",wname);
				goto savefile_bail;
			        }	
			}

		GLOBALS->default_flags=TR_RJUSTIFY;
		GLOBALS->shift_timebase_default_for_add=LLDescriptor(0);
	        while((iline=fgetmalloc(wave)))
	                {
	                parsewavline(iline, 0);
			any_shadow |= GLOBALS->shadow_active;
			free_2(iline);
	                }
		GLOBALS->default_flags=TR_RJUSTIFY;
		GLOBALS->shift_timebase_default_for_add=LLDescriptor(0);

		if(wave_is_compressed) pclose(wave); else fclose(wave);

                if(any_shadow)
                        {
                        if(GLOBALS->shadow_straces)
                                {
                                GLOBALS->shadow_active = 1;

                                swap_strace_contexts();
                                strace_maketimetrace(1);
                                swap_strace_contexts();

				GLOBALS->shadow_active = 0;
                                }
                        }
	        }
	}

savefile_bail:
GLOBALS->current_translate_file = 0;

if(fast_exit)
	{
	printf("Exiting early because of --exit request.\n");
	exit(0);
	}

if ((!GLOBALS->zoom_was_explicitly_set)&&
	((GLOBALS->tims.last-GLOBALS->tims.first)<=400)) GLOBALS->do_initial_zoom_fit=1;  /* force zoom on small traces */

calczoom(GLOBALS->tims.zoom);

if(!mainwindow_already_built)
{
#ifdef WAVE_USE_XID
if(!GLOBALS->socket_xid)
#endif
        {
	GLOBALS->mainwindow = gtk_window_new(GLOBALS->disable_window_manager ? GTK_WINDOW_POPUP : GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(GLOBALS->mainwindow), GLOBALS->winname);
	gtk_widget_set_usize(GTK_WIDGET(GLOBALS->mainwindow), GLOBALS->initial_window_x, GLOBALS->initial_window_y);

	if((GLOBALS->initial_window_width>0)&&(GLOBALS->initial_window_height>0))
		{
		gtk_window_set_default_size(GTK_WINDOW (GLOBALS->mainwindow), GLOBALS->initial_window_width, GLOBALS->initial_window_height);
		}

	gtk_signal_connect(GTK_OBJECT(GLOBALS->mainwindow), "delete_event", 	/* formerly was "destroy" */GTK_SIGNAL_FUNC(file_quit_cmd_callback), "WM destroy");

	gtk_widget_show(GLOBALS->mainwindow);
	}
#ifdef WAVE_USE_XID
	else
	{
        GLOBALS->mainwindow = gtk_plug_new(GLOBALS->socket_xid);
        gtk_widget_show(GLOBALS->mainwindow);
                                
        gtk_signal_connect(GTK_OBJECT(GLOBALS->mainwindow), "destroy",   /* formerly was "destroy" */GTK_SIGNAL_FUNC(plug_destroy),"Plug destroy");
	}
#endif
}


make_pixmaps(GLOBALS->mainwindow);


if(!mainwindow_already_built)
{
main_vbox = gtk_vbox_new(FALSE, 5);
gtk_container_border_width(GTK_CONTAINER(main_vbox), 1);
gtk_container_add(GTK_CONTAINER(GLOBALS->mainwindow), main_vbox);
gtk_widget_show(main_vbox);

if(!GLOBALS->disable_menus)
	{
	get_main_menu(GLOBALS->mainwindow, &menubar);
	gtk_widget_show(menubar);

	if(GLOBALS->force_toolbars)
		{
		toolhandle=gtk_handle_box_new();
		gtk_widget_show(toolhandle);
		gtk_container_add(GTK_CONTAINER(toolhandle), menubar);
		gtk_box_pack_start(GTK_BOX(main_vbox), toolhandle, FALSE, TRUE, 0);
		}
		else
		{
		gtk_box_pack_start(GTK_BOX(main_vbox), menubar, FALSE, TRUE, 0);
		}
	}

top_table = gtk_table_new (1, 284, FALSE);

if(GLOBALS->force_toolbars)
	{
	toolhandle=gtk_handle_box_new();
	gtk_widget_show(toolhandle);
	gtk_container_add(GTK_CONTAINER(toolhandle), top_table);
	}

whole_table = gtk_table_new (256, 16, FALSE);

text1 = create_text ();
gtk_table_attach (GTK_TABLE (top_table), text1, 0, 141, 0, 1,
                      	GTK_FILL,
                      	GTK_FILL | GTK_SHRINK, 0, 0);
gtk_widget_set_usize(GTK_WIDGET(text1), 200, -1);
gtk_widget_show (text1);

dummy1=gtk_label_new("");
gtk_table_attach (GTK_TABLE (top_table), dummy1, 141, 171, 0, 1,
                      	GTK_FILL,
                      	GTK_SHRINK, 0, 0);
gtk_widget_show (dummy1);

zoombuttons = create_zoom_buttons ();
gtk_table_attach (GTK_TABLE (top_table), zoombuttons, 171, 173, 0, 1,
                      	GTK_FILL,
                      	GTK_SHRINK, 0, 0);
gtk_widget_show (zoombuttons);

if(!GLOBALS->use_scrollbar_only)
	{
	pagebuttons = create_page_buttons ();
	gtk_table_attach (GTK_TABLE (top_table), pagebuttons, 173, 174, 0, 1,
	                      	GTK_FILL,
	                      	GTK_SHRINK, 0, 0);
	gtk_widget_show (pagebuttons);
	fetchbuttons = create_fetch_buttons ();
	gtk_table_attach (GTK_TABLE (top_table), fetchbuttons, 174, 175, 0, 1,
	                      	GTK_FILL,
	                      	GTK_SHRINK, 0, 0);
	gtk_widget_show (fetchbuttons);
	discardbuttons = create_discard_buttons ();
	gtk_table_attach (GTK_TABLE (top_table), discardbuttons, 175, 176, 0, 1,
	                      	GTK_FILL,
	                      	GTK_SHRINK, 0, 0);
	gtk_widget_show (discardbuttons);
	
	shiftbuttons = create_shift_buttons ();
	gtk_table_attach (GTK_TABLE (top_table), shiftbuttons, 176, 177, 0, 1,
	                      	GTK_FILL,
	                      	GTK_SHRINK, 0, 0);
	gtk_widget_show (shiftbuttons);
	}

dummy2=gtk_label_new("");
gtk_table_attach (GTK_TABLE (top_table), dummy2, 177, 215, 0, 1,
                      	GTK_FILL,
                      	GTK_SHRINK, 0, 0);
gtk_widget_show (dummy2);

entry = create_entry_box();
gtk_table_attach (GTK_TABLE (top_table), entry, 215, 216, 0, 1,
                      	GTK_SHRINK,
                      	GTK_SHRINK, 0, 0);
gtk_widget_show(entry);

timebox = create_time_box();
gtk_table_attach (GTK_TABLE (top_table), timebox, 216, 284, 0, 1,
                      	GTK_FILL | GTK_EXPAND,
                      	GTK_FILL | GTK_EXPAND | GTK_SHRINK, 20, 0);
gtk_widget_show (timebox);

if((GLOBALS->loaded_file_type != NO_FILE)&&(!GLOBALS->disable_menus))
	{
	GtkWidget *r_pixmap = gtk_pixmap_new(GLOBALS->redo_pixmap, GLOBALS->redo_mask);
	GtkWidget *main_vbox;
	GtkWidget *table, *table2;
	GtkWidget *b1, *frame;
	GtkTooltips *tooltips;

	gtk_widget_show(r_pixmap);

	tooltips=gtk_tooltips_new_2();
	gtk_tooltips_set_delay_2(tooltips,1500);

	table = gtk_table_new (1, 1, FALSE);

	main_vbox = gtk_vbox_new (FALSE, 1);
	gtk_container_border_width (GTK_CONTAINER (main_vbox), 1);
	gtk_container_add (GTK_CONTAINER (table), main_vbox);
	
	frame = gtk_frame_new ("Reload ");
	gtk_box_pack_start (GTK_BOX (main_vbox), frame, TRUE, TRUE, 0);
	
	gtk_widget_show (frame);
	gtk_widget_show (main_vbox);

	table2 = gtk_table_new (2, 1, FALSE);
	
	b1 = gtk_button_new();
	gtk_container_add(GTK_CONTAINER(b1), r_pixmap);
	gtk_table_attach (GTK_TABLE (table2), b1, 0, 1, 0, 1,
	                        GTK_FILL | GTK_EXPAND,
	                        GTK_FILL | GTK_EXPAND | GTK_SHRINK, 1, 1);
	gtk_signal_connect_object (GTK_OBJECT (b1), "clicked", GTK_SIGNAL_FUNC(menu_reload_waveform), GTK_OBJECT (table2));
	gtk_tooltips_set_tip_2(tooltips, b1, "Reload waveform", NULL);
	gtk_widget_show(b1);
	gtk_container_add (GTK_CONTAINER (frame), table2);
	gtk_widget_show(table2);

	gtk_table_attach (GTK_TABLE (top_table), table, 284, 285, 0, 1,
                      	0, 
                      	0, 2, 0);

	gtk_widget_show (table);
	}
} /* of ...if(mainwindow_already_built) */

GLOBALS->wavewindow = create_wavewindow();
load_all_fonts(); /* must be done before create_signalwindow() */
gtk_widget_show(GLOBALS->wavewindow);
GLOBALS->signalwindow = create_signalwindow();

if(GLOBALS->do_resize_signals) 
                {
                int os;
                os=GLOBALS->max_signal_name_pixel_width;
                os=(os<48)?48:os;
                gtk_widget_set_usize(GTK_WIDGET(GLOBALS->signalwindow),
                                os+30, -1);
                }

gtk_widget_show(GLOBALS->signalwindow);

#if GTK_CHECK_VERSION(2,4,0)
if(!GLOBALS->hide_sst)
	{
	GLOBALS->toppanedwindow = gtk_hpaned_new();
	GLOBALS->sstpane = treeboxframe("SST", GTK_SIGNAL_FUNC(mkmenu_treesearch_cleanup));
 
	GLOBALS->expanderwindow = gtk_expander_new_with_mnemonic("_SST");
	gtk_expander_set_expanded(GTK_EXPANDER(GLOBALS->expanderwindow), (GLOBALS->sst_expanded==TRUE));
	gtk_container_add(GTK_CONTAINER(GLOBALS->expanderwindow), GLOBALS->sstpane);
	gtk_widget_show(GLOBALS->expanderwindow);
	}
#endif

panedwindow=gtk_hpaned_new();

#ifdef HAVE_PANED_PACK
if(GLOBALS->paned_pack_semantics)
	{
	gtk_paned_pack1(GTK_PANED(panedwindow), GLOBALS->signalwindow, 0, 0); 
	gtk_paned_pack2(GTK_PANED(panedwindow), GLOBALS->wavewindow, ~0, 0);
	}
	else
#endif
	{
	gtk_paned_add1(GTK_PANED(panedwindow), GLOBALS->signalwindow);
	gtk_paned_add2(GTK_PANED(panedwindow), GLOBALS->wavewindow);
	}

gtk_widget_show(panedwindow);

#if GTK_CHECK_VERSION(2,4,0)
if(!GLOBALS->hide_sst)
	{
	gtk_paned_pack1(GTK_PANED(GLOBALS->toppanedwindow), GLOBALS->expanderwindow, 0, 0);
	gtk_paned_pack2(GTK_PANED(GLOBALS->toppanedwindow), panedwindow, ~0, 0);
	gtk_widget_show(GLOBALS->toppanedwindow);
	}
#endif

if(!mainwindow_already_built)
	{
	gtk_widget_show(top_table);

	gtk_table_attach (GTK_TABLE (whole_table), GLOBALS->force_toolbars?toolhandle:top_table, 0, 16, 0, 1,
	                      	GTK_FILL | GTK_EXPAND,
	                      	GTK_FILL | GTK_EXPAND | GTK_SHRINK, 0, 0);
	}

if(!GLOBALS->notebook)
	{
	GLOBALS->num_notebook_pages = 1;
	GLOBALS->this_context_page = 0;
	GLOBALS->contexts = calloc(1, sizeof(struct Globals ***)); /* calloc is deliberate! */
	*GLOBALS->contexts = calloc(1, sizeof(struct Globals **)); /* calloc is deliberate! */
	(*GLOBALS->contexts)[0] = GLOBALS;

	GLOBALS->dead_context = calloc(1, sizeof(struct Globals ***)); /* calloc is deliberate! */
	*GLOBALS->dead_context = calloc(1, sizeof(struct Globals **)); /* calloc is deliberate! */
	*(GLOBALS->dead_context)[0] = NULL;

	GLOBALS->notebook = gtk_notebook_new();
	gtk_notebook_set_tab_pos(GTK_NOTEBOOK(GLOBALS->notebook), GLOBALS->context_tabposition ? GTK_POS_LEFT : GTK_POS_TOP);

	gtk_widget_show(GLOBALS->notebook);
	gtk_notebook_set_show_tabs(GTK_NOTEBOOK(GLOBALS->notebook), 0); /* hide for first time until next tabs */
	gtk_notebook_set_show_border(GTK_NOTEBOOK(GLOBALS->notebook), 0); /* hide for first time until next tabs */
	gtk_signal_connect(GTK_OBJECT(GLOBALS->notebook), "switch-page", GTK_SIGNAL_FUNC(switch_page), NULL);
	}
	else
	{
	unsigned int i;

	GLOBALS->this_context_page = GLOBALS->num_notebook_pages;
	GLOBALS->num_notebook_pages++;
	GLOBALS->num_notebook_pages_cumulative++; /* this never decreases, acts as an incrementing flipper id for side tabs */
	*GLOBALS->contexts = realloc(*GLOBALS->contexts, GLOBALS->num_notebook_pages * sizeof(struct Globals *)); /* realloc is deliberate! */
	(*GLOBALS->contexts)[GLOBALS->this_context_page] = GLOBALS;

	for(i=0;i<GLOBALS->num_notebook_pages;i++)
		{
		(*GLOBALS->contexts)[i]->num_notebook_pages = GLOBALS->num_notebook_pages;
		(*GLOBALS->contexts)[i]->num_notebook_pages_cumulative = GLOBALS->num_notebook_pages_cumulative;
		(*GLOBALS->contexts)[i]->dead_context = (*GLOBALS->contexts)[0]->dead_context;	/* mirroring this is OK as page 0 always has value! */
		}

	gtk_notebook_set_show_tabs(GTK_NOTEBOOK(GLOBALS->notebook), ~0); /* then appear */
	gtk_notebook_set_show_border(GTK_NOTEBOOK(GLOBALS->notebook), ~0); /* then appear */
	gtk_notebook_set_scrollable(GTK_NOTEBOOK(GLOBALS->notebook), ~0);
	}

if(!GLOBALS->context_tabposition)
	{
	gtk_notebook_append_page(GTK_NOTEBOOK(GLOBALS->notebook), GLOBALS->toppanedwindow ? GLOBALS->toppanedwindow : panedwindow, 
		gtk_label_new(GLOBALS->loaded_file_name));
	}
	else
	{
	char buf[40];
	
	sprintf(buf, "%d", GLOBALS->num_notebook_pages_cumulative);

	gtk_notebook_append_page(GTK_NOTEBOOK(GLOBALS->notebook), GLOBALS->toppanedwindow ? GLOBALS->toppanedwindow : panedwindow, 
		gtk_label_new(buf));
	}

if(mainwindow_already_built)
	{
	gtk_notebook_set_current_page(GTK_NOTEBOOK(GLOBALS->notebook), GLOBALS->this_context_page);
	return(0);
	}

gtk_table_attach (GTK_TABLE (whole_table), GLOBALS->notebook, 0, 16, 1, 256,
                      	GTK_FILL | GTK_EXPAND,
                      	GTK_FILL | GTK_EXPAND | GTK_SHRINK, 0, 0);
gtk_widget_show(whole_table);
gtk_container_add (GTK_CONTAINER (main_vbox), whole_table);

update_markertime(time_trunc(GLOBALS->tims.marker));

set_window_xypos(GLOBALS->initial_window_xpos, GLOBALS->initial_window_ypos);
GLOBALS->xy_ignore_main_c_1 = 1;

if(GLOBALS->logfile) 
	{
	struct logfile_chain *lprev;
	char buf[50];
	int which = 1;
	while(GLOBALS->logfile)
		{
		sprintf(buf, "Logfile viewer [%d]", which++);
		logbox(buf, 480, GLOBALS->logfile->name);
		lprev = GLOBALS->logfile;
		GLOBALS->logfile = GLOBALS->logfile->next;
		free_2(lprev->name);
		free_2(lprev);
		}
	}

activate_stems_reader(GLOBALS->stems_name);

while (gtk_events_pending()) gtk_main_iteration();

if(1)	/* here in order to calculate window manager delta if present... window is completely rendered by here */
	{
	int dummy_x, dummy_y;
	get_window_xypos(&dummy_x, &dummy_y);
	}

init_busy();

if(scriptfile)
	{
	execute_script(scriptfile);
	free_2(scriptfile); scriptfile=NULL;
	}

#if !defined _MSC_VER && !defined __MINGW32__
if(GLOBALS->dual_attach_id_main_c_1)
	{
	fprintf(stderr, "GTKWAVE | Attaching %08X as dual head session %d\n", GLOBALS->dual_attach_id_main_c_1, GLOBALS->dual_id);

	GLOBALS->dual_ctx = shmat(GLOBALS->dual_attach_id_main_c_1, NULL, 0);
	if(GLOBALS->dual_ctx)
		{
		if(memcmp(GLOBALS->dual_ctx[GLOBALS->dual_id].matchword, DUAL_MATCHWORD, 4))
			{
			fprintf(stderr, "Not a valid shared memory ID for dual head operation, exiting.\n");
			exit(255);
			}

		GLOBALS->dual_ctx[GLOBALS->dual_id].viewer_is_initialized = 1;
		for(;;)
			{
		        GtkAdjustment *hadj;
		        TimeType pageinc, gt;
			struct timeval tv;

			if(GLOBALS->dual_ctx[1-GLOBALS->dual_id].use_new_times)
				{
				GLOBALS->dual_race_lock = 1;

			        gt = GLOBALS->dual_ctx[GLOBALS->dual_id].left_margin_time = GLOBALS->dual_ctx[1-GLOBALS->dual_id].left_margin_time;

				GLOBALS->dual_ctx[GLOBALS->dual_id].marker = GLOBALS->dual_ctx[1-GLOBALS->dual_id].marker;
				GLOBALS->dual_ctx[GLOBALS->dual_id].baseline = GLOBALS->dual_ctx[1-GLOBALS->dual_id].baseline;
				GLOBALS->dual_ctx[GLOBALS->dual_id].zoom = GLOBALS->dual_ctx[1-GLOBALS->dual_id].zoom;
				GLOBALS->dual_ctx[1-GLOBALS->dual_id].use_new_times = 0;
				GLOBALS->dual_ctx[GLOBALS->dual_id].use_new_times = 0;

				if(GLOBALS->dual_ctx[GLOBALS->dual_id].baseline != GLOBALS->tims.baseline)
					{
					if((GLOBALS->tims.marker != -1) && (GLOBALS->dual_ctx[GLOBALS->dual_id].marker == -1))
						{
				        	Trptr t;
  
        					for(t=GLOBALS->traces.first;t;t=t->t_next)
                					{
                					if(t->asciivalue) { free_2(t->asciivalue); t->asciivalue=NULL; }
                					}

	        				for(t=GLOBALS->traces.buffer;t;t=t->t_next)
	                				{
	                				if(t->asciivalue) { free_2(t->asciivalue); t->asciivalue=NULL; }
	                				}
						}

					GLOBALS->tims.marker = GLOBALS->dual_ctx[GLOBALS->dual_id].marker;
					GLOBALS->tims.baseline = GLOBALS->dual_ctx[GLOBALS->dual_id].baseline;
					update_basetime(GLOBALS->tims.baseline);
					update_markertime(GLOBALS->tims.marker);
					GLOBALS->signalwindow_width_dirty = 1;
					button_press_release_common();
					}
				else
				if(GLOBALS->dual_ctx[GLOBALS->dual_id].marker != GLOBALS->tims.marker)
					{
					if((GLOBALS->tims.marker != -1) && (GLOBALS->dual_ctx[GLOBALS->dual_id].marker == -1))
						{
				        	Trptr t;
  
        					for(t=GLOBALS->traces.first;t;t=t->t_next)
                					{
                					if(t->asciivalue) { free_2(t->asciivalue); t->asciivalue=NULL; }
                					}

	        				for(t=GLOBALS->traces.buffer;t;t=t->t_next)
	                				{
	                				if(t->asciivalue) { free_2(t->asciivalue); t->asciivalue=NULL; }
	                				}
						}

					GLOBALS->tims.marker = GLOBALS->dual_ctx[GLOBALS->dual_id].marker;
					update_markertime(GLOBALS->tims.marker);
					GLOBALS->signalwindow_width_dirty = 1;
					button_press_release_common();
					}

				GLOBALS->tims.prevzoom=GLOBALS->tims.zoom;
				GLOBALS->tims.zoom=GLOBALS->dual_ctx[GLOBALS->dual_id].zoom;

			        if(gt<GLOBALS->tims.first) gt=GLOBALS->tims.first;
			        else if(gt>GLOBALS->tims.last) gt=GLOBALS->tims.last;

			        hadj=GTK_ADJUSTMENT(GLOBALS->wave_hslider);
			        hadj->value=gt;
        
			        pageinc=(TimeType)(((gdouble)GLOBALS->wavewidth)*GLOBALS->nspx);
			        if(gt<(GLOBALS->tims.last-pageinc+1))
			                GLOBALS->tims.timecache=gt;
			                else
			                {
			                GLOBALS->tims.timecache=GLOBALS->tims.last-pageinc+1;
			                if(GLOBALS->tims.timecache<GLOBALS->tims.first) GLOBALS->tims.timecache=GLOBALS->tims.first;
			                }

			        time_update();
				}

			if(is_interactive)
				{
				kick_partial_vcd();
				}
				else
				{
				while (gtk_events_pending()) gtk_main_iteration();
				}

			GLOBALS->dual_race_lock = 0;

			tv.tv_sec = 0;
       			tv.tv_usec = 1000000 / 25;
			select(0, NULL, NULL, NULL, &tv);
			}
		}
		else
		{
		fprintf(stderr, "Could not attach to %08X, exiting.\n", GLOBALS->dual_attach_id_main_c_1);
		exit(255);
		}
	}
else
if(is_interactive)
	{
	for(;;) kick_partial_vcd();
	}
	else
#endif
	{
	/* Jump in to the main program loop */
	gtk_main();
	}

return(0);
}

void
get_window_size (int *x, int *y)
{
#ifdef WAVE_USE_GTK2
  gtk_window_get_size (GTK_WINDOW (GLOBALS->mainwindow), x, y);
#else
  *x = GLOBALS->initial_window_x;
  *y = GLOBALS->initial_window_y;
#endif
}

void
set_window_size (int x, int y)
{
  if (GLOBALS->mainwindow == NULL)
    {
      GLOBALS->initial_window_width = x;
      GLOBALS->initial_window_height = y;
    }
  else
    {
      if(!GLOBALS->socket_xid)
	{
      	gtk_window_set_default_size(GTK_WINDOW (GLOBALS->mainwindow), x, y);
	}
    }
}


void 
get_window_xypos(int *root_x, int *root_y)
{
if(!GLOBALS->mainwindow) return;

#ifdef WAVE_USE_GTK2
gtk_window_get_position(GTK_WINDOW(GLOBALS->mainwindow), root_x, root_y);

if(!GLOBALS->initial_window_get_valid)
	{
	if((GLOBALS->mainwindow->window))
		{
		GLOBALS->initial_window_get_valid = 1;
		GLOBALS->initial_window_xpos_get = *root_x;
		GLOBALS->initial_window_ypos_get = *root_y;

		GLOBALS->xpos_delta = GLOBALS->initial_window_xpos_set - GLOBALS->initial_window_xpos_get;
		GLOBALS->ypos_delta = GLOBALS->initial_window_ypos_set - GLOBALS->initial_window_ypos_get;
		}
	}
#else
*root_x = *root_y = -1;
#endif
}

void 
set_window_xypos(int root_x, int root_y)
{
if(GLOBALS->xy_ignore_main_c_1) return;

#if !defined __MINGW32__ && !defined _MSC_VER
GLOBALS->initial_window_xpos = root_x;
GLOBALS->initial_window_ypos = root_y;

if(!GLOBALS->mainwindow) return;
if((GLOBALS->initial_window_xpos>=0)||(GLOBALS->initial_window_ypos>=0))
	{
	if (GLOBALS->initial_window_xpos<0) { GLOBALS->initial_window_xpos = 0; }
	if (GLOBALS->initial_window_ypos<0) { GLOBALS->initial_window_ypos = 0; }
#ifdef WAVE_USE_GTK2
	gtk_window_move(GTK_WINDOW(GLOBALS->mainwindow), GLOBALS->initial_window_xpos, GLOBALS->initial_window_ypos);
#else
	gtk_window_reposition(GTK_WINDOW(GLOBALS->mainwindow), GLOBALS->initial_window_xpos, GLOBALS->initial_window_ypos);
#endif

	if(!GLOBALS->initial_window_set_valid)
		{
		GLOBALS->initial_window_set_valid = 1;
		GLOBALS->initial_window_xpos_set = GLOBALS->initial_window_xpos;
		GLOBALS->initial_window_ypos_set = GLOBALS->initial_window_ypos;
		}
	}
#endif
}


/*
 * bring up stems browser
 */
#if !defined _MSC_VER && !defined __MINGW32__
int stems_are_active(void)
{
if(GLOBALS->anno_ctx && GLOBALS->anno_ctx->browser_process)
	{
	int stat =0;
	pid_t pid = waitpid(GLOBALS->anno_ctx->browser_process, &stat, WNOHANG);
	if(!pid)
		{
		status_text("Stems reader already active.\n");
		return(1);
		}
		else
		{
		shmdt(GLOBALS->anno_ctx);
		GLOBALS->anno_ctx = NULL;
		}
	}

return(0);
}
#endif

void activate_stems_reader(char *stems_name)
{
#if !defined _MSC_VER && !defined __MINGW32__

#ifdef __CYGWIN__
/* ajb : ok static as this is a one-time warning message... */
static int cyg_called = 0;
#endif

if(!stems_name) return;

#ifdef __CYGWIN__
if(GLOBALS->stems_type != WAVE_ANNO_NONE)
	{
	if(!cyg_called)
		{
		fprintf(stderr, "GTKWAVE | If the viewer crashes with a Bad system call error,\n");
		fprintf(stderr, "GTKWAVE | make sure that Cygserver is enabled.\n");
		cyg_called = 1;
		}
	}
#endif

if(GLOBALS->stems_type != WAVE_ANNO_NONE)
	{
	int shmid = shmget(0, sizeof(struct gtkwave_annotate_ipc_t), IPC_CREAT | 0600 );
	if(shmid >=0)
		{
		struct shmid_ds ds;

		GLOBALS->anno_ctx = shmat(shmid, NULL, 0);
		if(GLOBALS->anno_ctx)
			{
			pid_t pid;

			memset(GLOBALS->anno_ctx, 0, sizeof(struct gtkwave_annotate_ipc_t));

			memcpy(GLOBALS->anno_ctx->matchword, WAVE_MATCHWORD, 4);
			GLOBALS->anno_ctx->aet_type = GLOBALS->stems_type;			
			strcpy(GLOBALS->anno_ctx->aet_name, GLOBALS->aet_name);
			strcpy(GLOBALS->anno_ctx->stems_name, stems_name);

			GLOBALS->anno_ctx->gtkwave_process = getpid();			
			update_markertime(GLOBALS->tims.marker);

#ifdef __linux__
			shmctl(shmid, IPC_RMID, &ds); /* mark for destroy */
#endif

		        pid=fork();

		        if(((int)pid) < 0) 
				{ 
				/* can't do anything about this */
				}
				else
				{
			        if(pid) /* parent==original server_pid */
			                {
					GLOBALS->anno_ctx->browser_process = pid;
					atexit(kill_stems_browser);
#ifndef __linux__
					sleep(2);
					shmctl(shmid, IPC_RMID, &ds); /* mark for destroy */
#endif
			                }
					else
					{
					char buf[64];
					sprintf(buf, "%08x", shmid);
				        execlp("rtlbrowse", "rtlbrowse", buf, NULL);
				        exit(0);        /* control never gets here if successful */
					}
				}
			}
			else
			{
			shmctl(shmid, IPC_RMID, &ds); /* actually destroy */
			GLOBALS->stems_type = WAVE_ANNO_NONE;
			}
		}
	}
	else
	{
	fprintf(stderr, "GTKWAVE | Unsupported dumpfile type for rtlbrowse.\n");
	}
#endif
}


#if !defined _MSC_VER && !defined __MINGW32__
void optimize_vcd_file(void) {
  if(!strcmp("-vcd", GLOBALS->unoptimized_vcd_file_name)) {        
    pid_t pid;
    char *buf = malloc_2(strlen("vcd") + 4 + 1);
    sprintf(buf, "%s.lx2", "vcd");
    pid = fork();
    if(((int)pid) < 0) {
      /* can't do anything about this */
    }
    else {
      if(pid) {
        int stat;
        int rc = waitpid(pid, &stat, 0);
	if(rc > 0) {
	  free_2(GLOBALS->loaded_file_name);
	  GLOBALS->loaded_file_name = buf;
	}					
      }
      else {
        execlp("vcd2lxt2", "vcd2lxt2", "--", "-", buf, NULL);
	exit(255);
      }
    }
  }
  else {
    pid_t pid;
    char *buf = malloc_2(strlen(GLOBALS->unoptimized_vcd_file_name) + 4 + 1);
    sprintf(buf, "%s.lx2", GLOBALS->unoptimized_vcd_file_name);
    pid = fork(); 
    if(((int)pid) < 0) {
      /* can't do anything about this */
    }
    else {
      if(pid) {
	int stat;
	int rc = waitpid(pid, &stat, 0);
        if(rc > 0) {
          free_2(GLOBALS->loaded_file_name);
	  GLOBALS->loaded_file_name = buf;
        }					
      }
      else {
        execlp("vcd2lxt2", "vcd2lxt2", GLOBALS->unoptimized_vcd_file_name, buf, NULL);
	exit(255);
      }
    }
  }	
}
#endif

/*
 * $Id$
 * $Log$
 * Revision 1.12  2007/09/14 14:08:56  gtkwave
 * updating busy handling
 *
 * Revision 1.11  2007/09/12 19:24:45  gtkwave
 * more ctx_swap_watchdog updates
 *
 * Revision 1.10  2007/09/12 17:26:45  gtkwave
 * experimental ctx_swap_watchdog added...still tracking down mouse thrash crashes
 *
 * Revision 1.9  2007/09/11 19:56:39  gtkwave
 * missing dynamic cast added
 *
 * Revision 1.8  2007/09/11 17:17:32  gtkwave
 * allow tab movement with context_tabposition rc variable
 *
 * Revision 1.7  2007/09/11 04:27:44  gtkwave
 * duplicate disable_mouseover copy on ctx switch
 *
 * Revision 1.6  2007/09/11 04:13:25  gtkwave
 * loader hardening for tabbed loads
 *
 * Revision 1.5  2007/09/11 02:20:21  gtkwave
 * migrate busy cursor to new tabbed context
 *
 * Revision 1.4  2007/09/09 20:10:30  gtkwave
 * preliminary support for tabbed viewing of multiple waveforms
 *
 * Revision 1.3  2007/08/29 23:38:47  gtkwave
 * 3.1.0 RC2 minor compatibility/bugfixes
 *
 * Revision 1.2  2007/08/26 21:35:42  gtkwave
 * integrated global context management from SystemOfCode2007 branch
 *
 * Revision 1.1.1.1.2.14  2007/08/26 19:13:30  gtkwave
 * datatype fix
 *
 * Revision 1.1.1.1.2.13  2007/08/26 19:05:55  gtkwave
 * added reload button in main window
 *
 * Revision 1.1.1.1.2.12  2007/08/23 23:28:48  gtkwave
 * reload fail handling and retries
 *
 * Revision 1.1.1.1.2.11  2007/08/23 03:16:03  gtkwave
 * NO_FILE now set on stdin sourced VCDs
 *
 * Revision 1.1.1.1.2.10  2007/08/23 02:42:51  gtkwave
 * convert c++ style comments to c to aid with compiler compatibility
 *
 * Revision 1.1.1.1.2.9  2007/08/19 23:13:53  kermin
 * -o flag will now target the original file (in theory reloaded), compress it to lxt2, and then reload the new compressed file.
 *
 * Revision 1.1.1.1.2.8  2007/08/15 03:26:01  kermin
 * Reload button does not cause a fault, however, state is still somehow incorrect.
 *
 * Revision 1.1.1.1.2.7  2007/08/07 04:54:59  gtkwave
 * slight modifications to global initialization scheme
 *
 * Revision 1.1.1.1.2.6  2007/08/07 03:18:54  kermin
 * Changed to pointer based GLOBAL structure and added initialization function
 *
 * Revision 1.1.1.1.2.5  2007/08/06 03:50:47  gtkwave
 * globals support for ae2, gtk1, cygwin, mingw.  also cleaned up some machine
 * generated structs, etc.
 *
 * Revision 1.1.1.1.2.4  2007/08/05 02:27:21  kermin
 * Semi working global struct
 *
 * Revision 1.1.1.1.2.3  2007/07/31 03:18:01  kermin
 * Merge Complete - I hope
 *
 * Revision 1.1.1.1.2.2  2007/07/28 19:50:39  kermin
 * Merged in the main line
 *
 * Revision 1.1.1.1  2007/05/30 04:27:56  gtkwave
 * Imported sources
 *
 * Revision 1.3  2007/05/28 00:55:06  gtkwave
 * added support for arrays as a first class dumpfile datatype
 *
 * Revision 1.2  2007/04/20 02:08:13  gtkwave
 * initial release
 *
 */

