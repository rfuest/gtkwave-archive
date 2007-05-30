/*
 * A n t l r  T r a n s l a t i o n  H e a d e r
 *
 * Terence Parr, Will Cohen, and Hank Dietz: 1989-2001
 * Purdue University Electrical Engineering
 * With AHPCRC, University of Minnesota
 * ANTLR Version 1.33MR33
 *
 *   ../pccts/antlr/antlr -ga -k 2 -gl ./verilog.g
 *
 */

#define ANTLR_VERSION	13333
#include "pcctscfg.h"
#include "pccts_stdio.h"
#line 1 "./verilog.g"


/*
* $Id$
* $Log$
* Revision 1.1.1.1  2007/05/30 04:25:47  gtkwave
* Imported sources
*
* Revision 1.1  2007/04/21 21:08:51  gtkwave
* changed from vertex to vermin
*
* Revision 1.2  2007/04/20 02:08:11  gtkwave
* initial release
*
*/

#include "attrib.h"
#include "vpp_common.h"

int zzcr_attr(Attrib *attr, int token, char *text);
void vpp_update_yyfilename(const char *str);
void vpp_update_yylineno(const char *str);

  
#define LL_K 2
#define zzSET_SIZE 28
#include "antlr.h"
#include "keyword_tokens.h"
#include "tokens.h"
#include "dlgdef.h"
#include "mode.h"

/* MR23 In order to remove calls to PURIFY use the antlr -nopurify option */

#ifndef PCCTS_PURIFY
#define PCCTS_PURIFY(r,s) memset((char *) &(r),'\0',(s));
#endif

ANTLR_INFO
#line 28 "./verilog.g"

#include "../../config.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/param.h>

#include "wave_locale.h"

#include "inter.h"
#include "tokens.h"
#include "shred.h"
#include "symbol.h"
#include "jrb.h"
#include "vpp_common.h"

#ifndef PATH_MAX
#define PATH_MAX (4096)
#endif

int verilog_keyword_code(const char *s, unsigned int len); /* returns token value from gperf */
void args_expand(int argc, char **argv, int *new_argc, char ***new_argv);

int zzerrors=0;
int zzcomment_start=0, zzcomment_depth=0, zzcomment_entry=0;
char *zzfilename=NULL;	

JRB modname_tree=NULL;
int module_is_duplicate=0;

char *mod_current_name = NULL;
char *mod_current_filename = NULL;
int   mod_start_line = 0;

char *udp_current_name = NULL;
char *udp_current_filename = NULL;
int   udp_start_line = 0;

char *comp_type_name = NULL;

int emit_stems = 0, emit_vars = 0;

static void add_string_to_tree(JRB tree, char *s, int exists)
{
  Jval val;
  JRB node;
  
if(!(node=jrb_find_str(tree, s)))
  {
    val.i = (exists!=0);
    jrb_insert_str(tree, s, val);
  }
  else
  {
    node->val.i |= (exists!=0);
  }
}

char *prevlineroot=NULL;
char *prevprevlineroot=NULL;
char *lineroot=NULL, *linepos=NULL;
int linemaxlen=0;
int zzrewind=0;

/*
* add token to the analyzed verilog buffer..
*/
void addtoken(int is_newline)
{
  if(!is_newline) 
  {
    zzrewind = strlen(zztext);
    
	if((linepos-lineroot)+zzrewind >= linemaxlen)
    {
      char *t=realloc(lineroot, linemaxlen = 2*(linemaxlen+zzrewind)+1);
      prevlineroot=realloc(prevlineroot, linemaxlen);
      prevprevlineroot=realloc(prevprevlineroot, linemaxlen);
      linepos = (linepos-lineroot) + t;
      lineroot = t;
    }
    
	strcpy(linepos,zztext);
    linepos+=zzrewind;
  }
  else
  { 
    char *t=prevprevlineroot;		/* scroll the buffer ptrs to avoid copying */
    prevprevlineroot = prevlineroot;
    prevlineroot = lineroot;
    linepos = lineroot = t;
    *linepos=0; 
    zzline++; 
  }
}

void report_error_context(int tok, char *badtok)
{ 
  char *ch, v;
  int col=0;
  
ch=lineroot;
  
do
  {
    v=*(ch++);
    switch(v)
    {
      case '\t': col=(col+8)&(~0x07); break;
      case '\n': col=2;		break;
      default:   col++;		break;
    }
  } while(v);
  
col-=zzrewind; if(col<2) col=2;
  
if(tok!=zzEOF_TOKEN)
  {
    warn("   Local context near token '%s' in '%s' on line %d.%d:\n",badtok,zzfilename, zzline,col-1);
  }
  else
  {
    warn("   Local context preceeding EOF in '%s' on line %d.%d:\n",zzfilename, zzline,col-1);
  }
  if(zzline>2)
  {
    warn("%6d: %s\n", zzline-2, prevprevlineroot);
  }
  if(zzline>1)
  {
    warn("%6d: %s\n", zzline-1, prevlineroot);
  }
  
if(col>2)
  {
    warn("%6d: %s\n\t%*s^\n", zzline, lineroot, col-2, "");
  }
  else
  {
    warn("%6d: %s\n\t^\n", zzline, lineroot);
  }
}


int do_not_translate = 0, do_not_translate_mask = 0;

void warn_about_translation_off(void)
{
  if(do_not_translate)
  {
    warn("** Warning: source code translation off for { %s%s%s%s} at EOF in '%s'.\n", 
    (do_not_translate&STMODE_XLATEOFF_IFDEF)    ? "ifdef "    : "",
    (do_not_translate&STMODE_XLATEOFF_SYNOPSYS) ? "synopsys " : "",
    (do_not_translate&STMODE_XLATEOFF_VERILINT) ? "verilint " : "",
    (do_not_translate&STMODE_XLATEOFF_VERTEX)   ? "vertex "   : "",
    zzfilename);
    
	do_not_translate = 0;
  }
  
if(ifdef_stack_top)
  {
    struct ifdef_stack_t *is = ifdef_stack_top;
    
		while(ifdef_stack_top)
    {
      is=ifdef_stack_top;
      
			warn("** Warning: pending `if%sdef %s at EOF, start is file '%s' line %d.\n",
      is->polarity ? "" : "n", 
      is->deftext,
      is->zzfilename, is->zzline);
      
		        free(is->zzfilename);
      free(is->deftext);
      ifdef_stack_top = is->next;
      free(is);
    }
    
		include_stack_depth = 0;
  }
  
if(zzcomment_depth)
  {
    mesg("** Error: Unterminated comment starts at line %d in '%s'.\n", zzcomment_start, zzfilename);
    zzcomment_depth = 0;
  }
}


void parsecomment(void)
{
  char *tok=strdup(zztext);
  char *tok_sav=tok;
  strcpy(tok, zztext);
  
tok = strtok(tok,"/ \t");
  if (tok != 0) {
    if (!strcmp("vpp_file", tok))
    {
      tok = strtok(NULL, " \t");
      if(tok) vpp_update_yyfilename(tok);
    }
    else
    if (!strcmp("vpp_line", tok))
    {
      tok = strtok(NULL, "");
      if(tok) vpp_update_yylineno(tok);
    }
    else
    if(!(do_not_translate&STMODE_XLATEOFF_IFDEF))	/* make sure preprocessed block is active */
    {
      if ((!strcmp("synopsys", tok))&&(do_not_translate_mask & STMODE_XLATEOFF_SYNOPSYS))
      {
        tok = strtok(NULL, " \t");
        if(tok) 
        {
          if(!strcmp("translate_on", tok))
          {
            do_not_translate &= ~(STMODE_XLATEOFF_SYNOPSYS);
          }
          else
          if(!strcmp("translate_off", tok))
          {
            do_not_translate |= (do_not_translate_mask & STMODE_XLATEOFF_SYNOPSYS);
          }
          else
          {
            warn("** Warning: unsupported synopsys pragma '%s' on line %d in file '%s', skipping.\n",
            tok, zzline, zzfilename);
          }
        }
      }
      else
      if ((!strcmp("verilint", tok))&&(do_not_translate_mask & STMODE_XLATEOFF_VERILINT))
      {
        tok = strtok(NULL, " \t");
        if(tok) 
        {
          if(!strcmp("translate", tok))
          {
            tok = strtok(NULL, " \t");
            if(tok)
            {
              if(!strcmp("on", tok))
              {
                do_not_translate &= ~(STMODE_XLATEOFF_VERILINT);
              }
              else
              if(!strcmp("off", tok))
              {
                do_not_translate |= (do_not_translate_mask & STMODE_XLATEOFF_VERILINT);
              }
              else
              {
                warn("** Warning: unsupported translate option '%s' on line %d in file '%s', skipping.\n",
                tok, zzline, zzfilename);
              }
            }
          }
        }
      }
      else
      if ((!strcmp("vertex", tok))&&(do_not_translate_mask & STMODE_XLATEOFF_VERTEX))
      {
        tok = strtok(NULL, " \t");
        if(tok) 
        {
          if(!strcmp("translate", tok))
          {
            tok = strtok(NULL, " \t");
            if(tok)
            {
              if(!strcmp("on", tok))
              {
                do_not_translate &= ~(STMODE_XLATEOFF_VERTEX);
              }
              else
              if(!strcmp("off", tok))
              {
                do_not_translate |= (do_not_translate_mask & STMODE_XLATEOFF_VERTEX);
              }
              else
              {
                warn("** Warning: unsupported translate option '%s' on line %d in file '%s', skipping.\n",
                tok, zzline, zzfilename);
              }
            }
          }
        }
      }
    }
  }
  
free(tok_sav);
}


void
zzsyn(char *text, int tok, char *egroup, SetWordType *eset, int etok, int k, char *bad_text)
{
  if(tok!=zzEOF_TOKEN)
  {
    mesg("** Syntax error at \"%s\"", bad_text);
  }
  else
  {
    mesg("** Syntax error at EOF");
  }
  if ( !etok && !eset ) {warn("\n"); return;}
  if ( k==1 ) warn(" missing");
  else
  {
    warn("; \"%s\" not", bad_text);
    if ( zzset_deg(eset)>1 ) warn(" in");
  }
  if ( zzset_deg(eset)>0 ) zzedecode(eset);
  else warn(" %s", zztokens[etok]);
  if ( strlen(egroup) > 0 ) warn(" in %s", egroup);
  warn("\n");
  report_error_context(tok, bad_text);
}


int zzcr_attr(Attrib *attr, int token, char *text)
{
  int len;
  
switch(token)
  {
    case V_FUNCTION_NAME:
    case V_IDENTIFIER:
    case V_IDENTIFIER2:
    case V_IDENDOT:
    attr->symbol=sym_add(text);
    break;
    
	case V_DBASE:
    case V_BBASE:
    case V_OBASE:
    case V_HBASE:
    attr->text=strdup(text);
    break;
    
	case V_STRING:
    len = strlen(text);
    text[len-1]=0;
    strcpy(attr->text = malloc(len-2+1), text+1);
    break;
    
	case V_DECIMAL_NUMBER:
    attr->ival=atoi_with_underscores(text);
    break;
    case V_FLOAT1:
    case V_FLOAT2:
    sscanf(text, "%lf", &attr->rval);	
    break;
    
	default: 
    attr->ival=0;
  }
  return(0);
}


int main(int argc, char **argv)
{
  int v_argc;
  char **v_argv;
  int i;
  struct vpp_filename *head=NULL, *curr=NULL, *t, *e;
  struct vpp_filename *lib_head=NULL, *lib_curr=NULL;
  struct vpp_filename *libext_head=NULL, *libext_curr=NULL;
  JRB node;
  int maxarg = v_argc - 1;
  
WAVE_LOCALE_FIX
  
args_expand(argc, argv, &v_argc, &v_argv);
  maxarg = v_argc - 1;
  
modname_tree=make_jrb();
  define_tree=make_jrb();
  
lineroot=linepos=(char *)calloc(1, linemaxlen=16);
  prevlineroot=(char *)calloc(1, linemaxlen);
  prevprevlineroot=(char *)calloc(1, linemaxlen);
  
if(v_argc==1)
  {
    warn(VERNAME"\n"
    "No files to process (use -h for help), exiting.\n\n");
    exit(0);
  }
  
warn(VERNAME"\n\n");
  
for(i=1;i<v_argc;i++)
  {
  int arglen = strlen(v_argv[i]);
  
	if((!strcmp(v_argv[i], "-y"))||(!strcmp(v_argv[i], "-yi")))
  {
  if(i==maxarg)
  {
  warn("** Missing filename after %s option!\n", v_argv[i]);
}
else
{
t=calloc(1, sizeof(struct vpp_filename));
t->name = strdup(v_argv[++i]);

			if(v_argv[i-1][2])		/* isn't nullchar so it's 'i' */
{
dirscan(t->name, t);	/* it's a case-insensitive directory, so scan it in */
}

			if(!lib_head)
{
lib_head=lib_curr=t;
}
else
{
lib_curr->next = t;
lib_curr = t;
}
}
}
else
if(!strcmp(v_argv[i], "-emitmono"))
{
if(i==maxarg)
{
warn("** Missing filename after -emitmono option!\n");
}
else
{
i++;

			if(mgetchar_fout)
{
warn("** Ignoring extra -emitmono specification for '%s'.\n", v_argv[i]);
}
else
{
mgetchar_fout = fopen(v_argv[i], "wb");
if(!mgetchar_fout)
{
mesg("** Could not open -emitmono file '%s', exiting.\n", v_argv[i]);
perror("Why");
exit(255);
}
}
}
}
else
if(!strcmp(v_argv[i], "-pragma"))
{
if(i==maxarg)
{
warn("** Missing typename after -pragma option!\n");
}
else
{
i++;
if(!strcmp(v_argv[i], "synopsys"))
{
do_not_translate_mask |= STMODE_XLATEOFF_SYNOPSYS; 
}
else
if(!strcmp(v_argv[i], "verilint"))
{
do_not_translate_mask |= STMODE_XLATEOFF_VERILINT; 
}
else
if(!strcmp(v_argv[i], "vertex"))
{
do_not_translate_mask |= STMODE_XLATEOFF_VERTEX; 
}
else
{
warn("** Unknown -pragma type '%s', ignoring.\n", v_argv[i]);
}
}
}
else
if((!strcmp(v_argv[i], "-h"))||(!strcmp(v_argv[i], "-help")))
{
warn(	"Usage:\n"
"------\n"
"%s [input filename(s)] [options]\n\n"
"-h[elp]         prints this screen\n"
"-emitmono fname emit monolithic (parser view of) file to fname\n"
"-emitstems      emit source code stems to stdout\n"
"-emitvars       emit source code variables to stdout\n"
"-Dx=y           equivalent to `define X Y in source\n"
"+define+x=y     equivalent to `define X Y in source\n"
"+incdir+dirname add dirname to include search path\n"
"+libext+ext     add ext to filename when searching for files\n"
"-pragma name    add name (synopsys, verilint, vertex) to accepted pragmas\n"
"-y dirname      add directory to source input path\n"
"-yi dirname     add directory to source input path (case insensitive search)\n"
"-f filename     insert args from filename (does not work recursively)\n"
"\n", v_argv[0]
);
exit(0);
}
else
if(!strcmp(v_argv[i], "-f"))
{
warn("** Cannot nest -f option inside an args file, exiting.\n");
exit(255);
}
if(!strcmp(v_argv[i], "-emitstems"))
{
emit_stems = 1;
}
else
if(!strcmp(v_argv[i], "-emitvars"))
{
emit_vars = 1;
}
else
if((arglen>=8)&&(!strncmp(v_argv[i],"+incdir+",8)))
{
if(arglen==8)
{
warn("** Missing path for +incdir+ in command line argument %d, ignoring.\n", i);
}   
else
{
char *lname=(char *)malloc(arglen-8+1);
char *tok;

                        strcpy(lname, v_argv[i]+8);
tok=strtok(lname,"+");                 

			while(tok)
{
int toklen=strlen(tok);
if(!toklen)
{
/* strtok seems to never hit this */
warn("** Missing path for +incdir+ (consecutive ++) in command line argument %d, ignoring.\n", i);
}
else
{
if(!incchain)
{
struct vpp_filename *l;
l=(struct vpp_filename *)calloc(1,sizeof(struct vpp_filename));
strcpy(l->name=malloc(toklen+1), tok);

		                                incchain=l;
}
else
{
struct vpp_filename *l=incchain;
struct vpp_filename *l2;

		                                while(l->next) l=l->next;

		                                l2=(struct vpp_filename *)calloc(1,sizeof(struct vpp_filename));
strcpy(l2->name=malloc(toklen+1), tok);
l->next=l2;
}
}

				tok=strtok(NULL, "+");
}

			free(lname);                        
}
}
else
if((arglen>=8)&&(!strncmp(v_argv[i],"+define+",8)))
{
if(arglen==8)
{
warn("** Missing value for +define+ in command line argument %d, ignoring.\n", i);
}   
else
{
char *lname=(char *)malloc(arglen-8+1);
char *tok;

                        strcpy(lname, v_argv[i]+8);
tok=strtok(lname,"+");                 

			while(tok)
{
int toklen=strlen(tok);
if(!toklen)
{
/* strtok seems to never hit this */
warn("** Missing value for +define+ (consecutive ++) in command line argument %d, ignoring.\n", i);
}
else
{
char *dname=(char *)malloc(toklen+8+1);
char *pnt;
sprintf(dname, "`define %s", tok);
pnt = dname+8;
while(*pnt)
{
if(*pnt=='=')
{
*pnt=' ';
break;
}
pnt++;
}
store_define(dname);
free(dname);
}

				tok=strtok(NULL, "+");
}

			free(lname);                        
}
}
else
if((arglen>=8)&&(!strncmp(v_argv[i],"+libext+",8)))
{
if(arglen==8)
{
warn("** Missing extension for +libext+ in command line argument %d, ignoring.\n", i);
}   
else
{
char *lname=(char *)malloc(arglen-8+1);
char *tok;
strcpy(lname, v_argv[i]+8);

			tok=strtok(lname,"+");                 
while(tok)
{
int toklen=strlen(tok);
if(!toklen)
{
/* strtok seems to never hit this */
warn("** Missing path for +libext+ (consecutive ++) in command line argument %d, ignoring.\n", i);
}
else
{
if(!libext_curr)
{
struct vpp_filename *l;
l=(struct vpp_filename *)calloc(1,sizeof(struct vpp_filename));
strcpy(l->name=malloc(toklen+1), tok);
libext_head=libext_curr=l;
}
else
{
struct vpp_filename *l;

		                                l=(struct vpp_filename *)calloc(1,sizeof(struct vpp_filename));
strcpy(l->name=malloc(toklen+1), tok);
libext_curr->next=l;
libext_curr=l;
}
}

				tok=strtok(NULL, "+");
}
free(lname);                        
}
}
else
if((arglen>=2)&&(!strncmp(v_argv[i],"-D",2)))
{
if(arglen==2)
{
warn("** Missing define for -D in command line argument %d, ignoring.\n", i);
}
else
{
char *dname=(char *)malloc(arglen-2+8+1);
char *pnt;
sprintf(dname, "`define %s", v_argv[i]+2);
pnt = dname+8;
while(*pnt)
{
if(*pnt=='=')
{
*pnt=' ';
break;
}
pnt++;
}
store_define(dname);
free(dname);
}
}
else	/* filename only.. */
{
t=calloc(1, sizeof(struct vpp_filename));
t->name = strdup(v_argv[i]);

		if(!head)
{
head=curr=t;
vlog_filenames = head;
}
else
{
curr->next = t;
curr = t;
}
}
}

shred_alloc();

prevprevlineroot[0]=prevlineroot[0]=lineroot[0]=0; linepos=lineroot; do_not_translate = 0;
ANTLRf(v_source_text(), mgetchar_fout ? mgetchar_outhandle : mgetchar); 
warn_about_translation_off();
zzerrors+=zzLexErrCount;
if(zzerrors)
{
warn("\n** %d error%s detected, exiting.\n\n",zzerrors,(zzerrors>1)?"s were":" was");
exit(255);
}

do	{
i=0;
jrb_traverse(node, modname_tree)
{
if(node->val.i==0)
{
FILE *hand;
int len = strlen(node->key.s);
int resolve = 0;
char *buff;
t=lib_head;

			while(t)
{
e=libext_head;
do
{
char *ext=e?e->name:"";
buff = (char *)malloc(strlen(t->name)+1+len+strlen(ext)+1);
sprintf(buff, "%s/%s%s", t->name, node->key.s, ext);
hand = fopen(buff, "r");
if(hand)
{
warn("Processing file '%s' ...\n", buff);
vpp_main(hand, buff);
prevprevlineroot[0]=prevlineroot[0]=lineroot[0]=0; linepos=lineroot; do_not_translate = 0;
ANTLRf(v_source_text(), mgetchar_fout ? mgetchar_outhandle : mgetchar);	/* preproc.c will do fclose() */
warn_about_translation_off();
free(buff);
zzerrors+=zzLexErrCount;
if(zzerrors)
{
warn("\n** %d error%s detected, exiting.\n\n",zzerrors,(zzerrors>1)?"s were":" was");
exit(255);
}
i=1;
resolve=1;
goto is_resolved;
}
free(buff);
} while((e)&&(e=e->next));

				t=t->next;
}

			t=lib_head;

			while(t)
{
if(t->numchildren)
{
e=libext_head;
do
{
char **realname;
char *ext=e?e->name:"";
buff = (char *)malloc(len+strlen(ext)+1);
sprintf(buff, "%s%s", node->key.s, ext);
realname = bsearch(buff, t->children, t->numchildren, sizeof(char **), compar_cstarstar_bsearch);
free(buff);
if(realname)
{
buff = (char *)malloc(strlen(t->name)+1+strlen(*realname)+1);
sprintf(buff, "%s/%s", t->name, *realname);

							hand = fopen(buff, "r");
if(hand)
{
warn("Processing file '%s' ...\n", buff);
vpp_main(hand, buff);
prevprevlineroot[0]=prevlineroot[0]=lineroot[0]=0; linepos=lineroot; do_not_translate = 0;
ANTLRf(v_source_text(), mgetchar_fout ? mgetchar_outhandle : mgetchar);	/* preproc.c will do fclose() */
warn_about_translation_off();
free(buff);
zzerrors+=zzLexErrCount;
if(zzerrors)
{
warn("\n** %d error%s detected, exiting.\n\n",zzerrors,(zzerrors>1)?"s were":" was");
exit(255);
}
i=1;
resolve=1;
goto is_resolved;
}	
free(buff);
}
} while((e)&&(e=e->next));
}

				t=t->next;
}

is_resolved:		if(!resolve)
{
warn("** Error: could not find module '%s'\n", node->key.s);
}

			node->val.i=1;
}
}
} while(i==1);

zzerrors+=zzLexErrCount;
if(zzerrors)
{
warn("\n** %d error%s detected, exiting.\n\n",zzerrors,(zzerrors>1)?"s were":" was");
exit(255);
}

shred_free();
return(0);
}



  

void
#ifdef __USE_PROTOS
v_source_text(void)
#else
v_source_text()
#endif
{
#line 1060 "./verilog.g"
  zzRULE;
  zzBLOCK(zztasp1);
  zzMake0;
  {
#line 1060 "./verilog.g"
  {
    zzBLOCK(zztasp2);
    zzMake0;
    {
    while ( (setwd1[LA(1)]&0x1) ) {
#line 1060 "./verilog.g"
      v_description();
      zzLOOP(zztasp2);
    }
    zzEXIT(zztasp2);
    }
  }
#line 1060 "./verilog.g"
  zzmatch(V_EOF); zzCONSUME;
  zzEXIT(zztasp1);
  return;
fail:
  zzEXIT(zztasp1);
  zzsyn(zzMissText, zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk, zzBadText);
  zzresynch(setwd1, 0x2);
  }
}

void
#ifdef __USE_PROTOS
v_description(void)
#else
v_description()
#endif
{
#line 1063 "./verilog.g"
  zzRULE;
  zzBLOCK(zztasp1);
  zzMake0;
  {
  if ( (setwd1[LA(1)]&0x4) ) {
#line 1063 "./verilog.g"
    v_module();
  }
  else {
    if ( (LA(1)==V_PRIMITIVE) ) {
#line 1064 "./verilog.g"
      v_udp();
    }
    else {zzFAIL(1,zzerr1,&zzMissSet,&zzMissText,&zzBadTok,&zzBadText,&zzErrk); goto fail;}
  }
  zzEXIT(zztasp1);
  return;
fail:
  zzEXIT(zztasp1);
  zzsyn(zzMissText, zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk, zzBadText);
  zzresynch(setwd1, 0x8);
  }
}

void
#ifdef __USE_PROTOS
v_module(void)
#else
v_module()
#endif
{
#line 1067 "./verilog.g"
  zzRULE;
  zzBLOCK(zztasp1);
  zzMake0;
  {
#line 1067 "./verilog.g"
  {
    zzBLOCK(zztasp2);
    zzMake0;
    {
    if ( (LA(1)==V_MODULE) ) {
#line 1067 "./verilog.g"
      zzmatch(V_MODULE); zzCONSUME;
    }
    else {
      if ( (LA(1)==V_MACROMODULE)
 ) {
#line 1067 "./verilog.g"
        zzmatch(V_MACROMODULE); zzCONSUME;
      }
      else {zzFAIL(1,zzerr2,&zzMissSet,&zzMissText,&zzBadTok,&zzBadText,&zzErrk); goto fail;}
    }
    zzEXIT(zztasp2);
    }
  }
#line 1068 "./verilog.g"
  {
    struct i_symbol_scope *sb = (struct i_symbol_scope *)calloc(1, sizeof(struct i_symbol_scope)); 
    sb->symtable = make_jrb(); 
    sb->parent = sym_base;
    sym_base = sb;
    
				module_is_duplicate=0; 
    if(mod_current_filename)
    {
      free(mod_current_filename);
    }
    mod_current_filename = strdup(zzfilename);
    
				if(mod_current_name)
    {
      free(mod_current_name);
      mod_current_name = NULL;
    }
    
				mod_start_line = zzline;
  }
#line 1090 "./verilog.g"
  v_identifier_nodot();
#line 1091 "./verilog.g"
  {
    JRB node = jrb_find_str(modname_tree, zzaArg(zztasp1,2 ).symbol->name);
    if((!node)||((node)&&(!node->val.i)))
    {
      add_string_to_tree(modname_tree, zzaArg(zztasp1,2 ).symbol->name, TRUE);
      mod_current_name = strdup(zzaArg(zztasp1,2 ).symbol->name);
      
					}
    else
    {
      warn("** Warning: skipping duplicate module '%s' at in file '%s' line %d\n", zzaArg(zztasp1,2 ).symbol->name, zzfilename, zzline);
      module_is_duplicate = 1;
    }
  }
#line 1107 "./verilog.g"
  v_module_body();
#line 1109 "./verilog.g"
  zzmatch(V_ENDMODULE);
#line 1110 "./verilog.g"
  {
    if(sym_base)
    {
      if(module_is_duplicate)
      {
        JRB symtree = sym_base->symtable;
        struct i_symbol_scope *sb = sym_base->parent;
        JRB node;
        
						jrb_traverse(node, symtree)
        {
          free(((struct i_symbol *)node->val.v)->name);
          free(node->val.v);	/* free up strings for any stray symbols (should be only modname) */
        }
        jrb_free_tree(symtree);						
        free(sym_base);
        sym_base = sb;
      }
      else
      {
        JRB symtree = sym_base->symtable;
        JRB node;
        
						sym_base = sym_base->parent;
        
						if(emit_stems)
        {
          char real_path[PATH_MAX];
          
							printf("++ module %s file %s lines %d - %d\n",
          mod_current_name, realpath(mod_current_filename, real_path), mod_start_line, zzline);
        }
        
						if(emit_vars)
        {
          jrb_traverse(node, symtree)						
          {
            printf("++ var %s module %s\n",
            ((struct i_symbol *)node->val.v)->name,
            mod_current_name);
          }
        }
        
						if(mod_current_filename)
        {
          free(mod_current_filename);
          mod_current_filename = NULL;
        }
        
						if(mod_current_name)
        {
          free(mod_current_name);
          mod_current_name = NULL;
        }
      }
    }
  }
 zzCONSUME;

  zzEXIT(zztasp1);
  return;
fail:
  zzEXIT(zztasp1);
  zzsyn(zzMissText, zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk, zzBadText);
  zzresynch(setwd1, 0x10);
  }
}

void
#ifdef __USE_PROTOS
v_module_body(void)
#else
v_module_body()
#endif
{
#line 1170 "./verilog.g"
  zzRULE;
  zzBLOCK(zztasp1);
  zzMake0;
  {
#line 1170 "./verilog.g"
  v_list_of_ports();
#line 1170 "./verilog.g"
  zzmatch(V_SEMI); zzCONSUME;
#line 1171 "./verilog.g"
  {
    zzBLOCK(zztasp2);
    zzMake0;
    {
    while ( (setwd1[LA(1)]&0x20) ) {
#line 1171 "./verilog.g"
      v_module_item();
      zzLOOP(zztasp2);
    }
    zzEXIT(zztasp2);
    }
  }
  zzEXIT(zztasp1);
  return;
fail:
  zzEXIT(zztasp1);
  zzsyn(zzMissText, zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk, zzBadText);
  zzresynch(setwd1, 0x40);
  }
}

void
#ifdef __USE_PROTOS
v_list_of_ports(void)
#else
v_list_of_ports()
#endif
{
#line 1174 "./verilog.g"
  zzRULE;
  zzBLOCK(zztasp1);
  zzMake0;
  {
  if ( (LA(1)==V_LP) ) {
#line 1174 "./verilog.g"
    zzmatch(V_LP); zzCONSUME;
#line 1174 "./verilog.g"
    v_port();
#line 1174 "./verilog.g"
    {
      zzBLOCK(zztasp2);
      zzMake0;
      {
      while ( (LA(1)==V_COMMA) ) {
#line 1174 "./verilog.g"
        zzmatch(V_COMMA); zzCONSUME;
#line 1174 "./verilog.g"
        v_port();
        zzLOOP(zztasp2);
      }
      zzEXIT(zztasp2);
      }
    }
#line 1174 "./verilog.g"
    zzmatch(V_RP); zzCONSUME;
  }
  else {
    if ( (LA(1)==V_SEMI) ) {
    }
    else {zzFAIL(1,zzerr3,&zzMissSet,&zzMissText,&zzBadTok,&zzBadText,&zzErrk); goto fail;}
  }
  zzEXIT(zztasp1);
  return;
fail:
  zzEXIT(zztasp1);
  zzsyn(zzMissText, zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk, zzBadText);
  zzresynch(setwd1, 0x80);
  }
}

void
#ifdef __USE_PROTOS
v_port(void)
#else
v_port()
#endif
{
#line 1178 "./verilog.g"
  zzRULE;
  zzBLOCK(zztasp1);
  zzMake0;
  {
  if ( (setwd2[LA(1)]&0x1)
 ) {
#line 1178 "./verilog.g"
    v_port_expression();
  }
  else {
    if ( (LA(1)==V_DOT) ) {
#line 1179 "./verilog.g"
      zzmatch(V_DOT); zzCONSUME;
#line 1179 "./verilog.g"
      v_identifier_nodot();
#line 1179 "./verilog.g"
      zzmatch(V_LP); zzCONSUME;
#line 1179 "./verilog.g"
      v_port_expression();
#line 1179 "./verilog.g"
      zzmatch(V_RP); zzCONSUME;
    }
    else {zzFAIL(1,zzerr4,&zzMissSet,&zzMissText,&zzBadTok,&zzBadText,&zzErrk); goto fail;}
  }
  zzEXIT(zztasp1);
  return;
fail:
  zzEXIT(zztasp1);
  zzsyn(zzMissText, zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk, zzBadText);
  zzresynch(setwd2, 0x2);
  }
}

void
#ifdef __USE_PROTOS
v_port_expression(void)
#else
v_port_expression()
#endif
{
#line 1182 "./verilog.g"
  zzRULE;
  zzBLOCK(zztasp1);
  zzMake0;
  {
  if ( (setwd2[LA(1)]&0x4) ) {
#line 1182 "./verilog.g"
    v_port_reference();
  }
  else {
    if ( (LA(1)==V_LBRACE) ) {
#line 1183 "./verilog.g"
      zzmatch(V_LBRACE); zzCONSUME;
#line 1183 "./verilog.g"
      v_port_reference();
#line 1183 "./verilog.g"
      {
        zzBLOCK(zztasp2);
        zzMake0;
        {
        while ( (LA(1)==V_COMMA) ) {
#line 1183 "./verilog.g"
          zzmatch(V_COMMA); zzCONSUME;
#line 1183 "./verilog.g"
          v_port_reference();
          zzLOOP(zztasp2);
        }
        zzEXIT(zztasp2);
        }
      }
#line 1183 "./verilog.g"
      zzmatch(V_RBRACE); zzCONSUME;
    }
    else {
      if ( (setwd2[LA(1)]&0x8)
 ) {
      }
      else {zzFAIL(1,zzerr5,&zzMissSet,&zzMissText,&zzBadTok,&zzBadText,&zzErrk); goto fail;}
    }
  }
  zzEXIT(zztasp1);
  return;
fail:
  zzEXIT(zztasp1);
  zzsyn(zzMissText, zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk, zzBadText);
  zzresynch(setwd2, 0x10);
  }
}

void
#ifdef __USE_PROTOS
v_port_reference(void)
#else
v_port_reference()
#endif
{
#line 1187 "./verilog.g"
  zzRULE;
  zzBLOCK(zztasp1);
  zzMake0;
  {
#line 1187 "./verilog.g"
  v_identifier_nodot();
#line 1189 "./verilog.g"
  {
    zzBLOCK(zztasp2);
    zzMake0;
    {
    if ( (LA(1)==V_LBRACK) ) {
#line 1188 "./verilog.g"
      {
        zzBLOCK(zztasp3);
        zzMake0;
        {
#line 1188 "./verilog.g"
        zzmatch(V_LBRACK); zzCONSUME;
#line 1188 "./verilog.g"
        v_expression();
#line 1188 "./verilog.g"
        {
          zzBLOCK(zztasp4);
          zzMake0;
          {
          if ( (LA(1)==V_COLON) ) {
#line 1188 "./verilog.g"
            zzmatch(V_COLON); zzCONSUME;
#line 1188 "./verilog.g"
            v_expression();
          }
          else {
            if ( (LA(1)==V_RBRACK) ) {
            }
            else {zzFAIL(1,zzerr6,&zzMissSet,&zzMissText,&zzBadTok,&zzBadText,&zzErrk); goto fail;}
          }
          zzEXIT(zztasp4);
          }
        }
#line 1189 "./verilog.g"
        zzmatch(V_RBRACK); zzCONSUME;
        zzEXIT(zztasp3);
        }
      }
    }
    else {
      if ( (setwd2[LA(1)]&0x20) ) {
      }
      else {zzFAIL(1,zzerr7,&zzMissSet,&zzMissText,&zzBadTok,&zzBadText,&zzErrk); goto fail;}
    }
    zzEXIT(zztasp2);
    }
  }
  zzEXIT(zztasp1);
  return;
fail:
  zzEXIT(zztasp1);
  zzsyn(zzMissText, zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk, zzBadText);
  zzresynch(setwd2, 0x40);
  }
}

void
#ifdef __USE_PROTOS
v_module_item(void)
#else
v_module_item()
#endif
{
#line 1192 "./verilog.g"
  zzRULE;
  zzBLOCK(zztasp1);
  zzMake0;
  {
  if ( (LA(1)==V_PARAMETER)
 ) {
#line 1192 "./verilog.g"
    v_parameter_declaration();
  }
  else {
    if ( (LA(1)==V_INPUT) ) {
#line 1193 "./verilog.g"
      v_input_declaration();
    }
    else {
      if ( (LA(1)==V_OUTPUT) ) {
#line 1194 "./verilog.g"
        v_output_declaration();
      }
      else {
        if ( (LA(1)==V_INOUT) ) {
#line 1195 "./verilog.g"
          v_inout_declaration();
        }
        else {
          if ( (LA(1)==V_REG) ) {
#line 1196 "./verilog.g"
            v_reg_declaration();
          }
          else {
            if ( (LA(1)==V_TIME)
 ) {
#line 1197 "./verilog.g"
              v_time_declaration();
            }
            else {
              if ( (LA(1)==V_INTEGER) ) {
#line 1198 "./verilog.g"
                v_integer_declaration();
              }
              else {
                if ( (LA(1)==V_REAL) ) {
#line 1199 "./verilog.g"
                  v_real_declaration();
                }
                else {
                  if ( (LA(1)==V_EVENT) ) {
#line 1200 "./verilog.g"
                    v_event_declaration();
                  }
                  else {
                    if ( (setwd2[LA(1)]&0x80) ) {
#line 1201 "./verilog.g"
                      v_gate_declaration();
                    }
                    else {
                      if ( (LA(1)==V_PRIMITIVE)
 ) {
#line 1202 "./verilog.g"
                        v_udp();
                      }
                      else {
                        if ( (setwd3[LA(1)]&0x1) ) {
#line 1203 "./verilog.g"
                          v_module_instantiation();
                        }
                        else {
                          if ( (LA(1)==V_DEFPARAM) ) {
#line 1204 "./verilog.g"
                            v_parameter_override();
                          }
                          else {
                            if ( (setwd3[LA(1)]&0x2) ) {
#line 1205 "./verilog.g"
                              v_continuous_assign();
                            }
                            else {
                              if ( (LA(1)==V_INITIAL) ) {
#line 1206 "./verilog.g"
                                v_initial_statement();
                              }
                              else {
                                if ( (LA(1)==V_ALWAYS)
 ) {
#line 1207 "./verilog.g"
                                  v_always_statement();
                                }
                                else {
                                  if ( (LA(1)==V_TASK) ) {
#line 1208 "./verilog.g"
                                    v_task();
                                  }
                                  else {
                                    if ( (LA(1)==V_FUNCTION) ) {
#line 1209 "./verilog.g"
                                      v_function();
                                    }
                                    else {
                                      if ( (LA(1)==V_SPECIFY) ) {
#line 1210 "./verilog.g"
                                        v_specify_block();
                                      }
                                      else {zzFAIL(1,zzerr8,&zzMissSet,&zzMissText,&zzBadTok,&zzBadText,&zzErrk); goto fail;}
                                    }
                                  }
                                }
                              }
                            }
                          }
                        }
                      }
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
  }
  zzEXIT(zztasp1);
  return;
fail:
  zzEXIT(zztasp1);
  zzsyn(zzMissText, zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk, zzBadText);
  zzresynch(setwd3, 0x4);
  }
}

void
#ifdef __USE_PROTOS
v_udp(void)
#else
v_udp()
#endif
{
#line 1213 "./verilog.g"
  zzRULE;
  zzBLOCK(zztasp1);
  zzMake0;
  {
#line 1213 "./verilog.g"
  zzmatch(V_PRIMITIVE);
#line 1214 "./verilog.g"
  { 
    struct i_symbol_scope *sb = (struct i_symbol_scope *)calloc(1, sizeof(struct i_symbol_scope)); 
    sb->symtable = make_jrb(); 
    sb->parent = sym_base;
    sym_base = sb;
    
			module_is_duplicate = 0;
    
			if(udp_current_filename)
    {
      free(udp_current_filename);
    }
    udp_current_filename = strdup(zzfilename);
    
			if(udp_current_name)
    {
      free(udp_current_name);
      udp_current_name = NULL;
    }
    
			udp_start_line = zzline;
  }
 zzCONSUME;

#line 1237 "./verilog.g"
  v_name_of_udp();
#line 1238 "./verilog.g"
  
  {
    JRB node = jrb_find_str(modname_tree, zzaArg(zztasp1,2 ).symbol->name);
    if((!node)||((node)&&(!node->val.i)))
    {
      add_string_to_tree(modname_tree, zzaArg(zztasp1,2 ).symbol->name, TRUE);
      udp_current_name = strdup(zzaArg(zztasp1,2 ).symbol->name);
    }
    else
    {
      warn("** Warning: skipping duplicate UDP '%s' at in file '%s' line %d\n", zzaArg(zztasp1,2 ).symbol->name, zzfilename, zzline);
      module_is_duplicate = 1;
    }
  }
#line 1254 "./verilog.g"
  zzmatch(V_LP); zzCONSUME;
#line 1254 "./verilog.g"
  v_name_of_variable();
#line 1255 "./verilog.g"
  {
    zzBLOCK(zztasp2);
    zzMake0;
    {
    while ( (LA(1)==V_COMMA) ) {
#line 1255 "./verilog.g"
      zzmatch(V_COMMA); zzCONSUME;
#line 1255 "./verilog.g"
      v_name_of_variable();
      zzLOOP(zztasp2);
    }
    zzEXIT(zztasp2);
    }
  }
#line 1255 "./verilog.g"
  zzmatch(V_RP); zzCONSUME;
#line 1255 "./verilog.g"
  zzmatch(V_SEMI); zzCONSUME;
#line 1256 "./verilog.g"
  {
    zzBLOCK(zztasp2);
    int zzcnt=1;
    zzMake0;
    {
    do {
#line 1256 "./verilog.g"
      v_udp_declaration();
      zzLOOP(zztasp2);
    } while ( (setwd3[LA(1)]&0x8)
 );
    zzEXIT(zztasp2);
    }
  }
#line 1257 "./verilog.g"
  v_udp_initial_statement();
#line 1258 "./verilog.g"
  v_table_definition();
#line 1259 "./verilog.g"
  zzmatch(V_ENDPRIMITIVE);
#line 1260 "./verilog.g"
  
  {
    if(sym_base)
    {
      if(module_is_duplicate)
      {
        JRB symtree = sym_base->symtable;
        struct i_symbol_scope *sb = sym_base->parent;
        JRB node;
        
						jrb_traverse(node, symtree)
        {
          free(((struct i_symbol *)node->val.v)->name);
          free(node->val.v);	/* free up strings for any stray symbols (should be only modname) */
        }
        jrb_free_tree(symtree);						
        free(sym_base);
        sym_base = sb;
      }
      else
      {
        sym_base = sym_base->parent;
        
						if(emit_stems)
        {
          printf("++ udp %s file %s lines %d - %d\n",
          udp_current_name, udp_current_filename, udp_start_line, zzline);
        }
        
						if(udp_current_filename)
        {
          free(udp_current_filename);
          udp_current_filename = NULL;
        }
        
						if(udp_current_name)
        {
          free(udp_current_name);
          udp_current_name = NULL;
        }
      }
    }
  }
 zzCONSUME;

  zzEXIT(zztasp1);
  return;
fail:
  zzEXIT(zztasp1);
  zzsyn(zzMissText, zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk, zzBadText);
  zzresynch(setwd3, 0x10);
  }
}

void
#ifdef __USE_PROTOS
v_udp_declaration(void)
#else
v_udp_declaration()
#endif
{
#line 1306 "./verilog.g"
  zzRULE;
  zzBLOCK(zztasp1);
  zzMake0;
  {
  if ( (LA(1)==V_OUTPUT) ) {
#line 1306 "./verilog.g"
    v_output_declaration();
  }
  else {
    if ( (LA(1)==V_REG) ) {
#line 1307 "./verilog.g"
      v_reg_declaration();
    }
    else {
      if ( (LA(1)==V_INPUT) ) {
#line 1308 "./verilog.g"
        v_input_declaration();
      }
      else {zzFAIL(1,zzerr9,&zzMissSet,&zzMissText,&zzBadTok,&zzBadText,&zzErrk); goto fail;}
    }
  }
  zzEXIT(zztasp1);
  return;
fail:
  zzEXIT(zztasp1);
  zzsyn(zzMissText, zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk, zzBadText);
  zzresynch(setwd3, 0x20);
  }
}

void
#ifdef __USE_PROTOS
v_udp_initial_statement(void)
#else
v_udp_initial_statement()
#endif
{
#line 1311 "./verilog.g"
  zzRULE;
  zzBLOCK(zztasp1);
  zzMake0;
  {
  if ( (LA(1)==V_INITIAL) ) {
#line 1311 "./verilog.g"
    zzmatch(V_INITIAL); zzCONSUME;
#line 1311 "./verilog.g"
    v_output_terminal_name();
#line 1311 "./verilog.g"
    zzmatch(V_EQ); zzCONSUME;
#line 1311 "./verilog.g"
    v_init_val();
#line 1311 "./verilog.g"
    zzmatch(V_SEMI); zzCONSUME;
  }
  else {
    if ( (LA(1)==V_TABLE)
 ) {
    }
    else {zzFAIL(1,zzerr10,&zzMissSet,&zzMissText,&zzBadTok,&zzBadText,&zzErrk); goto fail;}
  }
  zzEXIT(zztasp1);
  return;
fail:
  zzEXIT(zztasp1);
  zzsyn(zzMissText, zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk, zzBadText);
  zzresynch(setwd3, 0x40);
  }
}

void
#ifdef __USE_PROTOS
v_init_val(void)
#else
v_init_val()
#endif
{
#line 1316 "./verilog.g"
  zzRULE;
  zzBLOCK(zztasp1);
  zzMake0;
  {
  if ( (LA(1)==V_DECIMAL_NUMBER) ) {
#line 1316 "./verilog.g"
    zzmatch(V_DECIMAL_NUMBER); zzCONSUME;
  }
  else {
    if ( (LA(1)==V_BBASE) ) {
#line 1317 "./verilog.g"
      zzmatch(V_BBASE);
#line 1317 "./verilog.g"
      zzaRet.num = i_number_basemake(NV_BBASE, zzaArg(zztasp1,1 ).text);
 zzCONSUME;

    }
    else {zzFAIL(1,zzerr11,&zzMissSet,&zzMissText,&zzBadTok,&zzBadText,&zzErrk); goto fail;}
  }
  zzEXIT(zztasp1);
  return;
fail:
  zzEXIT(zztasp1);
  zzsyn(zzMissText, zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk, zzBadText);
  zzresynch(setwd3, 0x80);
  }
}

void
#ifdef __USE_PROTOS
v_output_terminal_name(void)
#else
v_output_terminal_name()
#endif
{
#line 1320 "./verilog.g"
  zzRULE;
  zzBLOCK(zztasp1);
  zzMake0;
  {
#line 1320 "./verilog.g"
  v_name_of_variable();
  zzEXIT(zztasp1);
  return;
fail:
  zzEXIT(zztasp1);
  zzsyn(zzMissText, zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk, zzBadText);
  zzresynch(setwd4, 0x1);
  }
}

void
#ifdef __USE_PROTOS
v_table_definition(void)
#else
v_table_definition()
#endif
{
#line 1323 "./verilog.g"
  zzRULE;
  zzBLOCK(zztasp1);
  zzMake0;
  {
#line 1323 "./verilog.g"
  zzmatch(V_TABLE); zzCONSUME;
#line 1323 "./verilog.g"
  v_table_entries();
#line 1323 "./verilog.g"
  zzmatch(V_ENDTABLE); zzCONSUME;
  zzEXIT(zztasp1);
  return;
fail:
  zzEXIT(zztasp1);
  zzsyn(zzMissText, zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk, zzBadText);
  zzresynch(setwd4, 0x2);
  }
}

void
#ifdef __USE_PROTOS
v_table_entries(void)
#else
v_table_entries()
#endif
{
#line 1326 "./verilog.g"
  zzRULE;
  zzBLOCK(zztasp1);
  zzMake0;
  {
#line 1326 "./verilog.g"
  {
    zzBLOCK(zztasp2);
    int zzcnt=1;
    zzMake0;
    {
    do {
#line 1326 "./verilog.g"
      v_com_seq_entry();
      zzLOOP(zztasp2);
    } while ( (setwd4[LA(1)]&0x4) );
    zzEXIT(zztasp2);
    }
  }
  zzEXIT(zztasp1);
  return;
fail:
  zzEXIT(zztasp1);
  zzsyn(zzMissText, zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk, zzBadText);
  zzresynch(setwd4, 0x8);
  }
}

void
#ifdef __USE_PROTOS
v_com_seq_entry(void)
#else
v_com_seq_entry()
#endif
{
#line 1329 "./verilog.g"
  zzRULE;
  zzBLOCK(zztasp1);
  zzMake0;
  {
#line 1329 "./verilog.g"
  v_input_list();
#line 1329 "./verilog.g"
  zzmatch(V_COLON); zzCONSUME;
#line 1329 "./verilog.g"
  {
    zzBLOCK(zztasp2);
    zzMake0;
    {
    if ( (setwd4[LA(1)]&0x10) && (LA(2)==V_COLON) ) {
#line 1329 "./verilog.g"
      v_state();
#line 1329 "./verilog.g"
      zzmatch(V_COLON); zzCONSUME;
#line 1329 "./verilog.g"
      v_next_state();
    }
    else {
      if ( (LA(1)==V_OUTPUT_SYMBOL) && 
(LA(2)==V_SEMI) ) {
#line 1330 "./verilog.g"
        zzmatch(V_OUTPUT_SYMBOL); zzCONSUME;
      }
      else {zzFAIL(2,zzerr12,zzerr13,&zzMissSet,&zzMissText,&zzBadTok,&zzBadText,&zzErrk); goto fail;}
    }
    zzEXIT(zztasp2);
    }
  }
#line 1330 "./verilog.g"
  zzmatch(V_SEMI); zzCONSUME;
  zzEXIT(zztasp1);
  return;
fail:
  zzEXIT(zztasp1);
  zzsyn(zzMissText, zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk, zzBadText);
  zzresynch(setwd4, 0x20);
  }
}

void
#ifdef __USE_PROTOS
v_level_symbol(void)
#else
v_level_symbol()
#endif
{
#line 1333 "./verilog.g"
  zzRULE;
  zzBLOCK(zztasp1);
  zzMake0;
  {
  if ( (LA(1)==V_OUTPUT_SYMBOL) ) {
#line 1333 "./verilog.g"
    zzmatch(V_OUTPUT_SYMBOL); zzCONSUME;
  }
  else {
    if ( (LA(1)==V_LEVEL_SYMBOL_EXTRA) ) {
#line 1334 "./verilog.g"
      zzmatch(V_LEVEL_SYMBOL_EXTRA); zzCONSUME;
    }
    else {zzFAIL(1,zzerr14,&zzMissSet,&zzMissText,&zzBadTok,&zzBadText,&zzErrk); goto fail;}
  }
  zzEXIT(zztasp1);
  return;
fail:
  zzEXIT(zztasp1);
  zzsyn(zzMissText, zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk, zzBadText);
  zzresynch(setwd4, 0x40);
  }
}

void
#ifdef __USE_PROTOS
v_edge(void)
#else
v_edge()
#endif
{
#line 1337 "./verilog.g"
  zzRULE;
  zzBLOCK(zztasp1);
  zzMake0;
  {
  if ( (LA(1)==V_LP) ) {
#line 1337 "./verilog.g"
    zzmatch(V_LP); zzCONSUME;
#line 1337 "./verilog.g"
    v_fake_edge();
#line 1337 "./verilog.g"
    zzmatch(V_RP); zzCONSUME;
  }
  else {
    if ( (LA(1)==V_EDGE_SYMBOL) ) {
#line 1338 "./verilog.g"
      zzmatch(V_EDGE_SYMBOL); zzCONSUME;
    }
    else {zzFAIL(1,zzerr15,&zzMissSet,&zzMissText,&zzBadTok,&zzBadText,&zzErrk); goto fail;}
  }
  zzEXIT(zztasp1);
  return;
fail:
  zzEXIT(zztasp1);
  zzsyn(zzMissText, zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk, zzBadText);
  zzresynch(setwd4, 0x80);
  }
}

void
#ifdef __USE_PROTOS
v_fake_edge(void)
#else
v_fake_edge()
#endif
{
#line 1342 "./verilog.g"
  zzRULE;
  zzBLOCK(zztasp1);
  zzMake0;
  {
  if ( (setwd5[LA(1)]&0x1)
 ) {
#line 1342 "./verilog.g"
    v_level_symbol();
#line 1342 "./verilog.g"
    v_level_symbol();
  }
  else {
    if ( (LA(1)==V_IDENTIFIER) ) {
#line 1343 "./verilog.g"
      zzmatch(V_IDENTIFIER); zzCONSUME;
    }
    else {zzFAIL(1,zzerr16,&zzMissSet,&zzMissText,&zzBadTok,&zzBadText,&zzErrk); goto fail;}
  }
  zzEXIT(zztasp1);
  return;
fail:
  zzEXIT(zztasp1);
  zzsyn(zzMissText, zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk, zzBadText);
  zzresynch(setwd5, 0x2);
  }
}

void
#ifdef __USE_PROTOS
v_level_symbol2(void)
#else
v_level_symbol2()
#endif
{
#line 1346 "./verilog.g"
  zzRULE;
  zzBLOCK(zztasp1);
  zzMake0;
  {
  if ( (setwd5[LA(1)]&0x4) ) {
#line 1346 "./verilog.g"
    v_level_symbol();
  }
  else {
    if ( (setwd5[LA(1)]&0x8) ) {
#line 1347 "./verilog.g"
      v_edge();
    }
    else {zzFAIL(1,zzerr17,&zzMissSet,&zzMissText,&zzBadTok,&zzBadText,&zzErrk); goto fail;}
  }
  zzEXIT(zztasp1);
  return;
fail:
  zzEXIT(zztasp1);
  zzsyn(zzMissText, zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk, zzBadText);
  zzresynch(setwd5, 0x10);
  }
}

void
#ifdef __USE_PROTOS
v_input_list(void)
#else
v_input_list()
#endif
{
#line 1350 "./verilog.g"
  zzRULE;
  zzBLOCK(zztasp1);
  zzMake0;
  {
#line 1350 "./verilog.g"
  {
    zzBLOCK(zztasp2);
    zzMake0;
    {
    while ( (setwd5[LA(1)]&0x20) ) {
#line 1350 "./verilog.g"
      v_level_symbol2();
      zzLOOP(zztasp2);
    }
    zzEXIT(zztasp2);
    }
  }
  zzEXIT(zztasp1);
  return;
fail:
  zzEXIT(zztasp1);
  zzsyn(zzMissText, zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk, zzBadText);
  zzresynch(setwd5, 0x40);
  }
}

void
#ifdef __USE_PROTOS
v_state(void)
#else
v_state()
#endif
{
#line 1354 "./verilog.g"
  zzRULE;
  zzBLOCK(zztasp1);
  zzMake0;
  {
#line 1354 "./verilog.g"
  v_level_symbol();
  zzEXIT(zztasp1);
  return;
fail:
  zzEXIT(zztasp1);
  zzsyn(zzMissText, zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk, zzBadText);
  zzresynch(setwd5, 0x80);
  }
}

void
#ifdef __USE_PROTOS
v_next_state(void)
#else
v_next_state()
#endif
{
#line 1357 "./verilog.g"
  zzRULE;
  zzBLOCK(zztasp1);
  zzMake0;
  {
  if ( (LA(1)==V_OUTPUT_SYMBOL)
 ) {
#line 1357 "./verilog.g"
    zzmatch(V_OUTPUT_SYMBOL); zzCONSUME;
  }
  else {
    if ( (LA(1)==V_HYPHEN) ) {
#line 1358 "./verilog.g"
      zzmatch(V_HYPHEN); zzCONSUME;
    }
    else {zzFAIL(1,zzerr18,&zzMissSet,&zzMissText,&zzBadTok,&zzBadText,&zzErrk); goto fail;}
  }
  zzEXIT(zztasp1);
  return;
fail:
  zzEXIT(zztasp1);
  zzsyn(zzMissText, zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk, zzBadText);
  zzresynch(setwd6, 0x1);
  }
}

void
#ifdef __USE_PROTOS
v_task(void)
#else
v_task()
#endif
{
#line 1361 "./verilog.g"
  zzRULE;
  zzBLOCK(zztasp1);
  zzMake0;
  {
#line 1361 "./verilog.g"
  zzmatch(V_TASK);
#line 1362 "./verilog.g"
  { 
    struct i_symbol_scope *sb = (struct i_symbol_scope *)calloc(1, sizeof(struct i_symbol_scope)); 
    sb->symtable = make_jrb(); 
    sb->parent = sym_base;
    sym_base = sb;
  }
 zzCONSUME;

#line 1369 "./verilog.g"
  v_identifier_nodot();
#line 1369 "./verilog.g"
  zzmatch(V_SEMI); zzCONSUME;
#line 1370 "./verilog.g"
  {
    zzBLOCK(zztasp2);
    zzMake0;
    {
    while ( (setwd6[LA(1)]&0x2) ) {
#line 1370 "./verilog.g"
      v_tf_declaration();
      zzLOOP(zztasp2);
    }
    zzEXIT(zztasp2);
    }
  }
#line 1371 "./verilog.g"
  v_statement_or_null();
#line 1371 "./verilog.g"
  zzmatch(V_ENDTASK);
#line 1372 "./verilog.g"
  if(sym_base) sym_base = sym_base->parent;
 zzCONSUME;

  zzEXIT(zztasp1);
  return;
fail:
  zzEXIT(zztasp1);
  zzsyn(zzMissText, zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk, zzBadText);
  zzresynch(setwd6, 0x4);
  }
}

void
#ifdef __USE_PROTOS
v_function(void)
#else
v_function()
#endif
{
#line 1375 "./verilog.g"
  zzRULE;
  zzBLOCK(zztasp1);
  zzMake0;
  {
#line 1375 "./verilog.g"
  zzmatch(V_FUNCTION);
#line 1376 "./verilog.g"
  { 
    struct i_symbol_scope *sb = (struct i_symbol_scope *)calloc(1, sizeof(struct i_symbol_scope)); 
    sb->symtable = make_jrb(); 
    sb->parent = sym_base;
    sym_base = sb;
  }
 zzCONSUME;

#line 1383 "./verilog.g"
  {
    zzBLOCK(zztasp2);
    zzMake0;
    {
    if ( (setwd6[LA(1)]&0x8) ) {
#line 1383 "./verilog.g"
      v_range_or_type();
    }
    else {
      if ( (setwd6[LA(1)]&0x10) ) {
      }
      else {zzFAIL(1,zzerr19,&zzMissSet,&zzMissText,&zzBadTok,&zzBadText,&zzErrk); goto fail;}
    }
    zzEXIT(zztasp2);
    }
  }
#line 1383 "./verilog.g"
  v_identifier_nodot();
#line 1383 "./verilog.g"
  zzmatch(V_SEMI); zzCONSUME;
#line 1384 "./verilog.g"
  {
    zzBLOCK(zztasp2);
    int zzcnt=1;
    zzMake0;
    {
    do {
#line 1384 "./verilog.g"
      v_tf_declaration();
      zzLOOP(zztasp2);
    } while ( (setwd6[LA(1)]&0x20)
 );
    zzEXIT(zztasp2);
    }
  }
#line 1385 "./verilog.g"
  v_statement();
#line 1385 "./verilog.g"
  zzmatch(V_ENDFUNCTION);
#line 1386 "./verilog.g"
  if(sym_base) sym_base = sym_base->parent;
 zzCONSUME;

  zzEXIT(zztasp1);
  return;
fail:
  zzEXIT(zztasp1);
  zzsyn(zzMissText, zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk, zzBadText);
  zzresynch(setwd6, 0x40);
  }
}

void
#ifdef __USE_PROTOS
v_range_or_type(void)
#else
v_range_or_type()
#endif
{
#line 1389 "./verilog.g"
  zzRULE;
  zzBLOCK(zztasp1);
  zzMake0;
  {
  if ( (LA(1)==V_LBRACK) ) {
#line 1389 "./verilog.g"
    v_range();
  }
  else {
    if ( (LA(1)==V_INTEGER) ) {
#line 1390 "./verilog.g"
      zzmatch(V_INTEGER); zzCONSUME;
    }
    else {
      if ( (LA(1)==V_REAL) ) {
#line 1391 "./verilog.g"
        zzmatch(V_REAL); zzCONSUME;
      }
      else {zzFAIL(1,zzerr20,&zzMissSet,&zzMissText,&zzBadTok,&zzBadText,&zzErrk); goto fail;}
    }
  }
  zzEXIT(zztasp1);
  return;
fail:
  zzEXIT(zztasp1);
  zzsyn(zzMissText, zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk, zzBadText);
  zzresynch(setwd6, 0x80);
  }
}

void
#ifdef __USE_PROTOS
v_tf_declaration(void)
#else
v_tf_declaration()
#endif
{
#line 1394 "./verilog.g"
  zzRULE;
  zzBLOCK(zztasp1);
  zzMake0;
  {
  if ( (LA(1)==V_PARAMETER) ) {
#line 1394 "./verilog.g"
    v_parameter_declaration();
  }
  else {
    if ( (LA(1)==V_INPUT)
 ) {
#line 1395 "./verilog.g"
      v_input_declaration();
    }
    else {
      if ( (LA(1)==V_OUTPUT) ) {
#line 1396 "./verilog.g"
        v_output_declaration();
      }
      else {
        if ( (LA(1)==V_INOUT) ) {
#line 1397 "./verilog.g"
          v_inout_declaration();
        }
        else {
          if ( (LA(1)==V_REG) ) {
#line 1398 "./verilog.g"
            v_reg_declaration();
          }
          else {
            if ( (LA(1)==V_TIME) ) {
#line 1399 "./verilog.g"
              v_time_declaration();
            }
            else {
              if ( (LA(1)==V_INTEGER)
 ) {
#line 1400 "./verilog.g"
                v_integer_declaration();
              }
              else {
                if ( (LA(1)==V_REAL) ) {
#line 1401 "./verilog.g"
                  v_real_declaration();
                }
                else {zzFAIL(1,zzerr21,&zzMissSet,&zzMissText,&zzBadTok,&zzBadText,&zzErrk); goto fail;}
              }
            }
          }
        }
      }
    }
  }
  zzEXIT(zztasp1);
  return;
fail:
  zzEXIT(zztasp1);
  zzsyn(zzMissText, zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk, zzBadText);
  zzresynch(setwd7, 0x1);
  }
}

void
#ifdef __USE_PROTOS
v_parameter_declaration(void)
#else
v_parameter_declaration()
#endif
{
#line 1408 "./verilog.g"
  zzRULE;
  zzBLOCK(zztasp1);
  zzMake0;
  {
#line 1408 "./verilog.g"
  zzmatch(V_PARAMETER); zzCONSUME;
#line 1408 "./verilog.g"
  {
    zzBLOCK(zztasp2);
    zzMake0;
    {
    if ( (LA(1)==V_LBRACK) ) {
#line 1408 "./verilog.g"
      v_range();
    }
    else {
      if ( (setwd7[LA(1)]&0x2) ) {
      }
      else {zzFAIL(1,zzerr22,&zzMissSet,&zzMissText,&zzBadTok,&zzBadText,&zzErrk); goto fail;}
    }
    zzEXIT(zztasp2);
    }
  }
#line 1408 "./verilog.g"
  v_list_of_param_assignments();
#line 1408 "./verilog.g"
  zzmatch(V_SEMI); zzCONSUME;
  zzEXIT(zztasp1);
  return;
fail:
  zzEXIT(zztasp1);
  zzsyn(zzMissText, zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk, zzBadText);
  zzresynch(setwd7, 0x4);
  }
}

void
#ifdef __USE_PROTOS
v_param_assignment(void)
#else
v_param_assignment()
#endif
{
#line 1411 "./verilog.g"
  zzRULE;
  zzBLOCK(zztasp1);
  zzMake0;
  {
#line 1411 "./verilog.g"
  v_identifier();
#line 1411 "./verilog.g"
  zzmatch(V_EQ); zzCONSUME;
#line 1411 "./verilog.g"
  v_constant_expression();
  zzEXIT(zztasp1);
  return;
fail:
  zzEXIT(zztasp1);
  zzsyn(zzMissText, zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk, zzBadText);
  zzresynch(setwd7, 0x8);
  }
}

void
#ifdef __USE_PROTOS
v_list_of_param_assignments(void)
#else
v_list_of_param_assignments()
#endif
{
#line 1414 "./verilog.g"
  zzRULE;
  zzBLOCK(zztasp1);
  zzMake0;
  {
#line 1414 "./verilog.g"
  v_param_assignment();
#line 1415 "./verilog.g"
  {
    zzBLOCK(zztasp2);
    zzMake0;
    {
    while ( (LA(1)==V_COMMA) ) {
#line 1415 "./verilog.g"
      zzmatch(V_COMMA); zzCONSUME;
#line 1415 "./verilog.g"
      v_param_assignment();
      zzLOOP(zztasp2);
    }
    zzEXIT(zztasp2);
    }
  }
  zzEXIT(zztasp1);
  return;
fail:
  zzEXIT(zztasp1);
  zzsyn(zzMissText, zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk, zzBadText);
  zzresynch(setwd7, 0x10);
  }
}

void
#ifdef __USE_PROTOS
v_input_declaration(void)
#else
v_input_declaration()
#endif
{
#line 1418 "./verilog.g"
  zzRULE;
  zzBLOCK(zztasp1);
  zzMake0;
  {
#line 1418 "./verilog.g"
  zzmatch(V_INPUT); zzCONSUME;
#line 1418 "./verilog.g"
  {
    zzBLOCK(zztasp2);
    zzMake0;
    {
    if ( (setwd7[LA(1)]&0x20)
 ) {
#line 1418 "./verilog.g"
      v_list_of_variables();
#line 1418 "./verilog.g"
      zzmatch(V_SEMI); zzCONSUME;
    }
    else {
      if ( (LA(1)==V_LBRACK) ) {
#line 1419 "./verilog.g"
        v_range();
#line 1419 "./verilog.g"
        v_list_of_variables();
#line 1419 "./verilog.g"
        zzmatch(V_SEMI); zzCONSUME;
      }
      else {zzFAIL(1,zzerr23,&zzMissSet,&zzMissText,&zzBadTok,&zzBadText,&zzErrk); goto fail;}
    }
    zzEXIT(zztasp2);
    }
  }
  zzEXIT(zztasp1);
  return;
fail:
  zzEXIT(zztasp1);
  zzsyn(zzMissText, zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk, zzBadText);
  zzresynch(setwd7, 0x40);
  }
}

void
#ifdef __USE_PROTOS
v_output_declaration(void)
#else
v_output_declaration()
#endif
{
#line 1422 "./verilog.g"
  zzRULE;
  zzBLOCK(zztasp1);
  zzMake0;
  {
#line 1422 "./verilog.g"
  zzmatch(V_OUTPUT); zzCONSUME;
#line 1422 "./verilog.g"
  {
    zzBLOCK(zztasp2);
    zzMake0;
    {
    if ( (setwd7[LA(1)]&0x80) ) {
#line 1422 "./verilog.g"
      v_list_of_variables();
#line 1422 "./verilog.g"
      zzmatch(V_SEMI); zzCONSUME;
    }
    else {
      if ( (LA(1)==V_LBRACK) ) {
#line 1423 "./verilog.g"
        v_range();
#line 1423 "./verilog.g"
        v_list_of_variables();
#line 1423 "./verilog.g"
        zzmatch(V_SEMI); zzCONSUME;
      }
      else {zzFAIL(1,zzerr24,&zzMissSet,&zzMissText,&zzBadTok,&zzBadText,&zzErrk); goto fail;}
    }
    zzEXIT(zztasp2);
    }
  }
  zzEXIT(zztasp1);
  return;
fail:
  zzEXIT(zztasp1);
  zzsyn(zzMissText, zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk, zzBadText);
  zzresynch(setwd8, 0x1);
  }
}

void
#ifdef __USE_PROTOS
v_inout_declaration(void)
#else
v_inout_declaration()
#endif
{
#line 1426 "./verilog.g"
  zzRULE;
  zzBLOCK(zztasp1);
  zzMake0;
  {
#line 1426 "./verilog.g"
  zzmatch(V_INOUT); zzCONSUME;
#line 1426 "./verilog.g"
  {
    zzBLOCK(zztasp2);
    zzMake0;
    {
    if ( (setwd8[LA(1)]&0x2) ) {
#line 1426 "./verilog.g"
      v_list_of_variables();
#line 1426 "./verilog.g"
      zzmatch(V_SEMI); zzCONSUME;
    }
    else {
      if ( (LA(1)==V_LBRACK)
 ) {
#line 1427 "./verilog.g"
        v_range();
#line 1427 "./verilog.g"
        v_list_of_variables();
#line 1427 "./verilog.g"
        zzmatch(V_SEMI); zzCONSUME;
      }
      else {zzFAIL(1,zzerr25,&zzMissSet,&zzMissText,&zzBadTok,&zzBadText,&zzErrk); goto fail;}
    }
    zzEXIT(zztasp2);
    }
  }
  zzEXIT(zztasp1);
  return;
fail:
  zzEXIT(zztasp1);
  zzsyn(zzMissText, zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk, zzBadText);
  zzresynch(setwd8, 0x4);
  }
}

void
#ifdef __USE_PROTOS
v_net_chg(void)
#else
v_net_chg()
#endif
{
#line 1430 "./verilog.g"
  zzRULE;
  zzBLOCK(zztasp1);
  zzMake0;
  {
  if ( (LA(1)==V_LP) ) {
#line 1430 "./verilog.g"
    v_charge_strength();
  }
  else {
    if ( (setwd8[LA(1)]&0x8) ) {
    }
    else {zzFAIL(1,zzerr26,&zzMissSet,&zzMissText,&zzBadTok,&zzBadText,&zzErrk); goto fail;}
  }
  zzEXIT(zztasp1);
  return;
fail:
  zzEXIT(zztasp1);
  zzsyn(zzMissText, zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk, zzBadText);
  zzresynch(setwd8, 0x10);
  }
}

void
#ifdef __USE_PROTOS
v_nettype(void)
#else
v_nettype()
#endif
{
#line 1434 "./verilog.g"
  zzRULE;
  zzBLOCK(zztasp1);
  zzMake0;
  {
  if ( (LA(1)==V_WIRE) ) {
#line 1434 "./verilog.g"
    zzmatch(V_WIRE); zzCONSUME;
  }
  else {
    if ( (LA(1)==V_TRI) ) {
#line 1435 "./verilog.g"
      zzmatch(V_TRI); zzCONSUME;
    }
    else {
      if ( (LA(1)==V_TRI1)
 ) {
#line 1436 "./verilog.g"
        zzmatch(V_TRI1); zzCONSUME;
      }
      else {
        if ( (LA(1)==V_SUPPLY0) ) {
#line 1437 "./verilog.g"
          zzmatch(V_SUPPLY0); zzCONSUME;
        }
        else {
          if ( (LA(1)==V_WAND) ) {
#line 1438 "./verilog.g"
            zzmatch(V_WAND); zzCONSUME;
          }
          else {
            if ( (LA(1)==V_TRIAND) ) {
#line 1439 "./verilog.g"
              zzmatch(V_TRIAND); zzCONSUME;
            }
            else {
              if ( (LA(1)==V_TRI0) ) {
#line 1440 "./verilog.g"
                zzmatch(V_TRI0); zzCONSUME;
              }
              else {
                if ( (LA(1)==V_SUPPLY1)
 ) {
#line 1441 "./verilog.g"
                  zzmatch(V_SUPPLY1); zzCONSUME;
                }
                else {
                  if ( (LA(1)==V_WOR) ) {
#line 1442 "./verilog.g"
                    zzmatch(V_WOR); zzCONSUME;
                  }
                  else {
                    if ( (LA(1)==V_TRIOR) ) {
#line 1443 "./verilog.g"
                      zzmatch(V_TRIOR); zzCONSUME;
                    }
                    else {
                      if ( (LA(1)==V_TRIREG) ) {
#line 1444 "./verilog.g"
                        zzmatch(V_TRIREG); zzCONSUME;
                      }
                      else {zzFAIL(1,zzerr27,&zzMissSet,&zzMissText,&zzBadTok,&zzBadText,&zzErrk); goto fail;}
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
  }
  zzEXIT(zztasp1);
  return;
fail:
  zzEXIT(zztasp1);
  zzsyn(zzMissText, zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk, zzBadText);
  zzresynch(setwd8, 0x20);
  }
}

void
#ifdef __USE_PROTOS
v_expandrange(void)
#else
v_expandrange()
#endif
{
#line 1447 "./verilog.g"
  zzRULE;
  zzBLOCK(zztasp1);
  zzMake0;
  {
  if ( (LA(1)==V_LBRACK) ) {
#line 1447 "./verilog.g"
    v_range();
  }
  else {
    if ( (LA(1)==V_SCALARED)
 ) {
#line 1448 "./verilog.g"
      zzmatch(V_SCALARED); zzCONSUME;
#line 1448 "./verilog.g"
      v_range();
    }
    else {
      if ( (LA(1)==V_VECTORED) ) {
#line 1449 "./verilog.g"
        zzmatch(V_VECTORED); zzCONSUME;
#line 1449 "./verilog.g"
        v_range();
      }
      else {zzFAIL(1,zzerr28,&zzMissSet,&zzMissText,&zzBadTok,&zzBadText,&zzErrk); goto fail;}
    }
  }
  zzEXIT(zztasp1);
  return;
fail:
  zzEXIT(zztasp1);
  zzsyn(zzMissText, zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk, zzBadText);
  zzresynch(setwd8, 0x40);
  }
}

void
#ifdef __USE_PROTOS
v_reg_declaration(void)
#else
v_reg_declaration()
#endif
{
#line 1452 "./verilog.g"
  zzRULE;
  zzBLOCK(zztasp1);
  zzMake0;
  {
#line 1452 "./verilog.g"
  zzmatch(V_REG); zzCONSUME;
#line 1452 "./verilog.g"
  v_reg_range();
#line 1452 "./verilog.g"
  v_list_of_register_variables();
#line 1452 "./verilog.g"
  zzmatch(V_SEMI); zzCONSUME;
  zzEXIT(zztasp1);
  return;
fail:
  zzEXIT(zztasp1);
  zzsyn(zzMissText, zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk, zzBadText);
  zzresynch(setwd8, 0x80);
  }
}

void
#ifdef __USE_PROTOS
v_reg_range(void)
#else
v_reg_range()
#endif
{
#line 1455 "./verilog.g"
  zzRULE;
  zzBLOCK(zztasp1);
  zzMake0;
  {
  if ( (LA(1)==V_LBRACK) ) {
#line 1455 "./verilog.g"
    v_range();
  }
  else {
    if ( (setwd9[LA(1)]&0x1) ) {
    }
    else {zzFAIL(1,zzerr29,&zzMissSet,&zzMissText,&zzBadTok,&zzBadText,&zzErrk); goto fail;}
  }
  zzEXIT(zztasp1);
  return;
fail:
  zzEXIT(zztasp1);
  zzsyn(zzMissText, zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk, zzBadText);
  zzresynch(setwd9, 0x2);
  }
}

void
#ifdef __USE_PROTOS
v_time_declaration(void)
#else
v_time_declaration()
#endif
{
#line 1459 "./verilog.g"
  zzRULE;
  zzBLOCK(zztasp1);
  zzMake0;
  {
#line 1459 "./verilog.g"
  zzmatch(V_TIME); zzCONSUME;
#line 1459 "./verilog.g"
  {
    zzBLOCK(zztasp2);
    zzMake0;
    {
    if ( (LA(1)==V_LBRACK) ) {
#line 1459 "./verilog.g"
      v_range();
    }
    else {
      if ( (setwd9[LA(1)]&0x4)
 ) {
      }
      else {zzFAIL(1,zzerr30,&zzMissSet,&zzMissText,&zzBadTok,&zzBadText,&zzErrk); goto fail;}
    }
    zzEXIT(zztasp2);
    }
  }
#line 1459 "./verilog.g"
  v_list_of_register_variables();
#line 1459 "./verilog.g"
  zzmatch(V_SEMI); zzCONSUME;
  zzEXIT(zztasp1);
  return;
fail:
  zzEXIT(zztasp1);
  zzsyn(zzMissText, zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk, zzBadText);
  zzresynch(setwd9, 0x8);
  }
}

void
#ifdef __USE_PROTOS
v_integer_declaration(void)
#else
v_integer_declaration()
#endif
{
#line 1462 "./verilog.g"
  zzRULE;
  zzBLOCK(zztasp1);
  zzMake0;
  {
#line 1462 "./verilog.g"
  zzmatch(V_INTEGER); zzCONSUME;
#line 1462 "./verilog.g"
  {
    zzBLOCK(zztasp2);
    zzMake0;
    {
    if ( (LA(1)==V_LBRACK) ) {
#line 1462 "./verilog.g"
      v_range();
    }
    else {
      if ( (setwd9[LA(1)]&0x10) ) {
      }
      else {zzFAIL(1,zzerr31,&zzMissSet,&zzMissText,&zzBadTok,&zzBadText,&zzErrk); goto fail;}
    }
    zzEXIT(zztasp2);
    }
  }
#line 1462 "./verilog.g"
  v_list_of_register_variables();
#line 1462 "./verilog.g"
  zzmatch(V_SEMI); zzCONSUME;
  zzEXIT(zztasp1);
  return;
fail:
  zzEXIT(zztasp1);
  zzsyn(zzMissText, zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk, zzBadText);
  zzresynch(setwd9, 0x20);
  }
}

void
#ifdef __USE_PROTOS
v_real_declaration(void)
#else
v_real_declaration()
#endif
{
#line 1465 "./verilog.g"
  zzRULE;
  zzBLOCK(zztasp1);
  zzMake0;
  {
#line 1465 "./verilog.g"
  zzmatch(V_REAL); zzCONSUME;
#line 1465 "./verilog.g"
  {
    zzBLOCK(zztasp2);
    zzMake0;
    {
    if ( (LA(1)==V_LBRACK) ) {
#line 1465 "./verilog.g"
      v_range();
    }
    else {
      if ( (setwd9[LA(1)]&0x40) ) {
      }
      else {zzFAIL(1,zzerr32,&zzMissSet,&zzMissText,&zzBadTok,&zzBadText,&zzErrk); goto fail;}
    }
    zzEXIT(zztasp2);
    }
  }
#line 1465 "./verilog.g"
  v_list_of_variables();
#line 1465 "./verilog.g"
  zzmatch(V_SEMI); zzCONSUME;
  zzEXIT(zztasp1);
  return;
fail:
  zzEXIT(zztasp1);
  zzsyn(zzMissText, zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk, zzBadText);
  zzresynch(setwd9, 0x80);
  }
}

void
#ifdef __USE_PROTOS
v_event_declaration(void)
#else
v_event_declaration()
#endif
{
#line 1468 "./verilog.g"
  zzRULE;
  zzBLOCK(zztasp1);
  zzMake0;
  {
#line 1468 "./verilog.g"
  zzmatch(V_EVENT); zzCONSUME;
#line 1468 "./verilog.g"
  v_name_of_event();
#line 1468 "./verilog.g"
  {
    zzBLOCK(zztasp2);
    zzMake0;
    {
    while ( (LA(1)==V_COMMA)
 ) {
#line 1468 "./verilog.g"
      zzmatch(V_COMMA); zzCONSUME;
#line 1468 "./verilog.g"
      v_name_of_event();
      zzLOOP(zztasp2);
    }
    zzEXIT(zztasp2);
    }
  }
#line 1468 "./verilog.g"
  zzmatch(V_SEMI); zzCONSUME;
  zzEXIT(zztasp1);
  return;
fail:
  zzEXIT(zztasp1);
  zzsyn(zzMissText, zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk, zzBadText);
  zzresynch(setwd10, 0x1);
  }
}

void
#ifdef __USE_PROTOS
v_continuous_assign(void)
#else
v_continuous_assign()
#endif
{
#line 1471 "./verilog.g"
  zzRULE;
  zzBLOCK(zztasp1);
  zzMake0;
  {
  if ( (LA(1)==V_ASSIGN) ) {
#line 1471 "./verilog.g"
    zzmatch(V_ASSIGN); zzCONSUME;
#line 1471 "./verilog.g"
    v_cont_drv();
#line 1471 "./verilog.g"
    v_cont_dly();
#line 1471 "./verilog.g"
    v_list_of_assignments();
#line 1471 "./verilog.g"
    zzmatch(V_SEMI); zzCONSUME;
  }
  else {
    if ( (setwd10[LA(1)]&0x2) ) {
#line 1472 "./verilog.g"
      v_nettype();
#line 1473 "./verilog.g"
      v_net_chg();
#line 1474 "./verilog.g"
      v_cont_exr();
#line 1474 "./verilog.g"
      v_cont_dly();
#line 1475 "./verilog.g"
      {
        zzBLOCK(zztasp2);
        zzMake0;
        {
        if ( (setwd10[LA(1)]&0x4) && (setwd10[LA(2)]&0x8) && !(
 LA(1)==V_IDENTIFIER && LA(2)==V_COMMA
|| LA(1)==V_IDENTIFIER2 && LA(2)==V_COMMA
|| LA(1)==V_FUNCTION_NAME && LA(2)==V_COMMA
|| LA(1)==V_IDENDOT && LA(2)==V_COMMA
) ) {
#line 1475 "./verilog.g"
          v_list_of_assignments();
        }
        else {
          if ( (setwd10[LA(1)]&0x10) && (setwd10[LA(2)]&0x20) ) {
#line 1475 "./verilog.g"
            v_list_of_variables();
          }
          else {zzFAIL(2,zzerr33,zzerr34,&zzMissSet,&zzMissText,&zzBadTok,&zzBadText,&zzErrk); goto fail;}
        }
        zzEXIT(zztasp2);
        }
      }
#line 1475 "./verilog.g"
      zzmatch(V_SEMI); zzCONSUME;
    }
    else {zzFAIL(1,zzerr35,&zzMissSet,&zzMissText,&zzBadTok,&zzBadText,&zzErrk); goto fail;}
  }
  zzEXIT(zztasp1);
  return;
fail:
  zzEXIT(zztasp1);
  zzsyn(zzMissText, zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk, zzBadText);
  zzresynch(setwd10, 0x40);
  }
}

void
#ifdef __USE_PROTOS
v_cont_drv(void)
#else
v_cont_drv()
#endif
{
#line 1478 "./verilog.g"
  zzRULE;
  zzBLOCK(zztasp1);
  zzMake0;
  {
  if ( (LA(1)==V_LP)
 ) {
#line 1478 "./verilog.g"
    v_drive_strength();
  }
  else {
    if ( (setwd10[LA(1)]&0x80) ) {
    }
    else {zzFAIL(1,zzerr36,&zzMissSet,&zzMissText,&zzBadTok,&zzBadText,&zzErrk); goto fail;}
  }
  zzEXIT(zztasp1);
  return;
fail:
  zzEXIT(zztasp1);
  zzsyn(zzMissText, zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk, zzBadText);
  zzresynch(setwd11, 0x1);
  }
}

void
#ifdef __USE_PROTOS
v_cont_exr(void)
#else
v_cont_exr()
#endif
{
#line 1482 "./verilog.g"
  zzRULE;
  zzBLOCK(zztasp1);
  zzMake0;
  {
  if ( (setwd11[LA(1)]&0x2) ) {
#line 1482 "./verilog.g"
    v_expandrange();
  }
  else {
    if ( (setwd11[LA(1)]&0x4) ) {
    }
    else {zzFAIL(1,zzerr37,&zzMissSet,&zzMissText,&zzBadTok,&zzBadText,&zzErrk); goto fail;}
  }
  zzEXIT(zztasp1);
  return;
fail:
  zzEXIT(zztasp1);
  zzsyn(zzMissText, zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk, zzBadText);
  zzresynch(setwd11, 0x8);
  }
}

void
#ifdef __USE_PROTOS
v_cont_dly(void)
#else
v_cont_dly()
#endif
{
#line 1486 "./verilog.g"
  zzRULE;
  zzBLOCK(zztasp1);
  zzMake0;
  {
  if ( (LA(1)==V_POUND) ) {
#line 1486 "./verilog.g"
    v_delay();
  }
  else {
    if ( (setwd11[LA(1)]&0x10)
 ) {
    }
    else {zzFAIL(1,zzerr38,&zzMissSet,&zzMissText,&zzBadTok,&zzBadText,&zzErrk); goto fail;}
  }
  zzEXIT(zztasp1);
  return;
fail:
  zzEXIT(zztasp1);
  zzsyn(zzMissText, zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk, zzBadText);
  zzresynch(setwd11, 0x20);
  }
}

void
#ifdef __USE_PROTOS
v_parameter_override(void)
#else
v_parameter_override()
#endif
{
#line 1491 "./verilog.g"
  zzRULE;
  zzBLOCK(zztasp1);
  zzMake0;
  {
#line 1491 "./verilog.g"
  zzmatch(V_DEFPARAM); zzCONSUME;
#line 1491 "./verilog.g"
  v_list_of_param_assignments();
#line 1491 "./verilog.g"
  zzmatch(V_SEMI); zzCONSUME;
  zzEXIT(zztasp1);
  return;
fail:
  zzEXIT(zztasp1);
  zzsyn(zzMissText, zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk, zzBadText);
  zzresynch(setwd11, 0x40);
  }
}

void
#ifdef __USE_PROTOS
v_list_of_variables(void)
#else
v_list_of_variables()
#endif
{
#line 1494 "./verilog.g"
  zzRULE;
  zzBLOCK(zztasp1);
  zzMake0;
  {
#line 1494 "./verilog.g"
  v_name_of_variable();
#line 1495 "./verilog.g"
  {
    zzBLOCK(zztasp2);
    zzMake0;
    {
    while ( (LA(1)==V_COMMA) ) {
#line 1495 "./verilog.g"
      zzmatch(V_COMMA); zzCONSUME;
#line 1495 "./verilog.g"
      v_name_of_variable();
      zzLOOP(zztasp2);
    }
    zzEXIT(zztasp2);
    }
  }
  zzEXIT(zztasp1);
  return;
fail:
  zzEXIT(zztasp1);
  zzsyn(zzMissText, zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk, zzBadText);
  zzresynch(setwd11, 0x80);
  }
}

void
#ifdef __USE_PROTOS
v_name_of_variable(void)
#else
v_name_of_variable()
#endif
{
#line 1498 "./verilog.g"
  zzRULE;
  zzBLOCK(zztasp1);
  zzMake0;
  {
#line 1498 "./verilog.g"
  v_identifier();
  zzEXIT(zztasp1);
  return;
fail:
  zzEXIT(zztasp1);
  zzsyn(zzMissText, zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk, zzBadText);
  zzresynch(setwd12, 0x1);
  }
}

void
#ifdef __USE_PROTOS
v_list_of_register_variables(void)
#else
v_list_of_register_variables()
#endif
{
#line 1501 "./verilog.g"
  zzRULE;
  zzBLOCK(zztasp1);
  zzMake0;
  {
#line 1501 "./verilog.g"
  v_register_variable();
#line 1502 "./verilog.g"
  {
    zzBLOCK(zztasp2);
    zzMake0;
    {
    while ( (LA(1)==V_COMMA) ) {
#line 1502 "./verilog.g"
      zzmatch(V_COMMA); zzCONSUME;
#line 1502 "./verilog.g"
      v_register_variable();
      zzLOOP(zztasp2);
    }
    zzEXIT(zztasp2);
    }
  }
  zzEXIT(zztasp1);
  return;
fail:
  zzEXIT(zztasp1);
  zzsyn(zzMissText, zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk, zzBadText);
  zzresynch(setwd12, 0x2);
  }
}

void
#ifdef __USE_PROTOS
v_register_variable(void)
#else
v_register_variable()
#endif
{
#line 1505 "./verilog.g"
  zzRULE;
  zzBLOCK(zztasp1);
  zzMake0;
  {
  if ( (setwd12[LA(1)]&0x4) && (setwd12[LA(2)]&0x8) ) {
#line 1505 "./verilog.g"
    v_name_of_register();
  }
  else {
    if ( (setwd12[LA(1)]&0x10) && (LA(2)==V_LBRACK) ) {
#line 1506 "./verilog.g"
      v_name_of_memory();
#line 1506 "./verilog.g"
      zzmatch(V_LBRACK); zzCONSUME;
#line 1506 "./verilog.g"
      v_expression();
#line 1506 "./verilog.g"
      zzmatch(V_COLON); zzCONSUME;
#line 1507 "./verilog.g"
      v_expression();
#line 1507 "./verilog.g"
      zzmatch(V_RBRACK); zzCONSUME;
    }
    else {zzFAIL(2,zzerr39,zzerr40,&zzMissSet,&zzMissText,&zzBadTok,&zzBadText,&zzErrk); goto fail;}
  }
  zzEXIT(zztasp1);
  return;
fail:
  zzEXIT(zztasp1);
  zzsyn(zzMissText, zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk, zzBadText);
  zzresynch(setwd12, 0x20);
  }
}

void
#ifdef __USE_PROTOS
v_name_of_register(void)
#else
v_name_of_register()
#endif
{
#line 1510 "./verilog.g"
  zzRULE;
  zzBLOCK(zztasp1);
  zzMake0;
  {
#line 1510 "./verilog.g"
  v_identifier();
  zzEXIT(zztasp1);
  return;
fail:
  zzEXIT(zztasp1);
  zzsyn(zzMissText, zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk, zzBadText);
  zzresynch(setwd12, 0x40);
  }
}

void
#ifdef __USE_PROTOS
v_name_of_memory(void)
#else
v_name_of_memory()
#endif
{
#line 1513 "./verilog.g"
  zzRULE;
  zzBLOCK(zztasp1);
  zzMake0;
  {
#line 1513 "./verilog.g"
  v_identifier();
  zzEXIT(zztasp1);
  return;
fail:
  zzEXIT(zztasp1);
  zzsyn(zzMissText, zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk, zzBadText);
  zzresynch(setwd12, 0x80);
  }
}

void
#ifdef __USE_PROTOS
v_name_of_event(void)
#else
v_name_of_event()
#endif
{
#line 1516 "./verilog.g"
  zzRULE;
  zzBLOCK(zztasp1);
  zzMake0;
  {
#line 1516 "./verilog.g"
  v_identifier();
  zzEXIT(zztasp1);
  return;
fail:
  zzEXIT(zztasp1);
  zzsyn(zzMissText, zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk, zzBadText);
  zzresynch(setwd13, 0x1);
  }
}

void
#ifdef __USE_PROTOS
v_charge_strength(void)
#else
v_charge_strength()
#endif
{
#line 1519 "./verilog.g"
  zzRULE;
  zzBLOCK(zztasp1);
  zzMake0;
  {
#line 1519 "./verilog.g"
  zzmatch(V_LP); zzCONSUME;
#line 1519 "./verilog.g"
  {
    zzBLOCK(zztasp2);
    zzMake0;
    {
    if ( (LA(1)==V_SMALL)
 ) {
#line 1519 "./verilog.g"
      zzmatch(V_SMALL); zzCONSUME;
    }
    else {
      if ( (LA(1)==V_MEDIUM) ) {
#line 1520 "./verilog.g"
        zzmatch(V_MEDIUM); zzCONSUME;
      }
      else {
        if ( (LA(1)==V_LARGE) ) {
#line 1521 "./verilog.g"
          zzmatch(V_LARGE); zzCONSUME;
        }
        else {
          if ( (setwd13[LA(1)]&0x2) ) {
#line 1522 "./verilog.g"
            v_strength0();
#line 1522 "./verilog.g"
            zzmatch(V_COMMA); zzCONSUME;
#line 1522 "./verilog.g"
            v_strength1();
          }
          else {
            if ( (setwd13[LA(1)]&0x4) ) {
#line 1523 "./verilog.g"
              v_strength1();
#line 1523 "./verilog.g"
              zzmatch(V_COMMA); zzCONSUME;
#line 1523 "./verilog.g"
              v_strength0();
            }
            else {zzFAIL(1,zzerr41,&zzMissSet,&zzMissText,&zzBadTok,&zzBadText,&zzErrk); goto fail;}
          }
        }
      }
    }
    zzEXIT(zztasp2);
    }
  }
#line 1523 "./verilog.g"
  zzmatch(V_RP); zzCONSUME;
  zzEXIT(zztasp1);
  return;
fail:
  zzEXIT(zztasp1);
  zzsyn(zzMissText, zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk, zzBadText);
  zzresynch(setwd13, 0x8);
  }
}

void
#ifdef __USE_PROTOS
v_drive_strength(void)
#else
v_drive_strength()
#endif
{
#line 1526 "./verilog.g"
  zzRULE;
  zzBLOCK(zztasp1);
  zzMake0;
  {
#line 1526 "./verilog.g"
  zzmatch(V_LP); zzCONSUME;
#line 1526 "./verilog.g"
  {
    zzBLOCK(zztasp2);
    zzMake0;
    {
    if ( (setwd13[LA(1)]&0x10)
 ) {
#line 1526 "./verilog.g"
      v_strength0();
#line 1526 "./verilog.g"
      zzmatch(V_COMMA); zzCONSUME;
#line 1526 "./verilog.g"
      v_strength1();
    }
    else {
      if ( (setwd13[LA(1)]&0x20) ) {
#line 1527 "./verilog.g"
        v_strength1();
#line 1527 "./verilog.g"
        zzmatch(V_COMMA); zzCONSUME;
#line 1527 "./verilog.g"
        v_strength0();
      }
      else {zzFAIL(1,zzerr42,&zzMissSet,&zzMissText,&zzBadTok,&zzBadText,&zzErrk); goto fail;}
    }
    zzEXIT(zztasp2);
    }
  }
#line 1527 "./verilog.g"
  zzmatch(V_RP); zzCONSUME;
  zzEXIT(zztasp1);
  return;
fail:
  zzEXIT(zztasp1);
  zzsyn(zzMissText, zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk, zzBadText);
  zzresynch(setwd13, 0x40);
  }
}

void
#ifdef __USE_PROTOS
v_strength0(void)
#else
v_strength0()
#endif
{
#line 1530 "./verilog.g"
  zzRULE;
  zzBLOCK(zztasp1);
  zzMake0;
  {
  if ( (LA(1)==V_SUPPLY0) ) {
#line 1530 "./verilog.g"
    zzmatch(V_SUPPLY0); zzCONSUME;
  }
  else {
    if ( (LA(1)==V_STRONG0) ) {
#line 1531 "./verilog.g"
      zzmatch(V_STRONG0); zzCONSUME;
    }
    else {
      if ( (LA(1)==V_PULL0) ) {
#line 1532 "./verilog.g"
        zzmatch(V_PULL0); zzCONSUME;
      }
      else {
        if ( (LA(1)==V_WEAK0)
 ) {
#line 1533 "./verilog.g"
          zzmatch(V_WEAK0); zzCONSUME;
        }
        else {
          if ( (LA(1)==V_HIGHZ0) ) {
#line 1534 "./verilog.g"
            zzmatch(V_HIGHZ0); zzCONSUME;
          }
          else {zzFAIL(1,zzerr43,&zzMissSet,&zzMissText,&zzBadTok,&zzBadText,&zzErrk); goto fail;}
        }
      }
    }
  }
  zzEXIT(zztasp1);
  return;
fail:
  zzEXIT(zztasp1);
  zzsyn(zzMissText, zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk, zzBadText);
  zzresynch(setwd13, 0x80);
  }
}

void
#ifdef __USE_PROTOS
v_strength1(void)
#else
v_strength1()
#endif
{
#line 1537 "./verilog.g"
  zzRULE;
  zzBLOCK(zztasp1);
  zzMake0;
  {
  if ( (LA(1)==V_SUPPLY1) ) {
#line 1537 "./verilog.g"
    zzmatch(V_SUPPLY1); zzCONSUME;
  }
  else {
    if ( (LA(1)==V_STRONG1) ) {
#line 1538 "./verilog.g"
      zzmatch(V_STRONG1); zzCONSUME;
    }
    else {
      if ( (LA(1)==V_PULL1) ) {
#line 1539 "./verilog.g"
        zzmatch(V_PULL1); zzCONSUME;
      }
      else {
        if ( (LA(1)==V_WEAK1)
 ) {
#line 1540 "./verilog.g"
          zzmatch(V_WEAK1); zzCONSUME;
        }
        else {
          if ( (LA(1)==V_HIGHZ1) ) {
#line 1541 "./verilog.g"
            zzmatch(V_HIGHZ1); zzCONSUME;
          }
          else {zzFAIL(1,zzerr44,&zzMissSet,&zzMissText,&zzBadTok,&zzBadText,&zzErrk); goto fail;}
        }
      }
    }
  }
  zzEXIT(zztasp1);
  return;
fail:
  zzEXIT(zztasp1);
  zzsyn(zzMissText, zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk, zzBadText);
  zzresynch(setwd14, 0x1);
  }
}

void
#ifdef __USE_PROTOS
v_range(void)
#else
v_range()
#endif
{
#line 1544 "./verilog.g"
  zzRULE;
  zzBLOCK(zztasp1);
  zzMake0;
  {
#line 1544 "./verilog.g"
  zzmatch(V_LBRACK); zzCONSUME;
#line 1544 "./verilog.g"
  v_expression();
#line 1545 "./verilog.g"
  zzmatch(V_COLON); zzCONSUME;
#line 1545 "./verilog.g"
  v_expression();
#line 1545 "./verilog.g"
  zzmatch(V_RBRACK); zzCONSUME;
  zzEXIT(zztasp1);
  return;
fail:
  zzEXIT(zztasp1);
  zzsyn(zzMissText, zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk, zzBadText);
  zzresynch(setwd14, 0x2);
  }
}

void
#ifdef __USE_PROTOS
v_list_of_assignments(void)
#else
v_list_of_assignments()
#endif
{
#line 1548 "./verilog.g"
  zzRULE;
  zzBLOCK(zztasp1);
  zzMake0;
  {
#line 1548 "./verilog.g"
  v_assignment();
#line 1548 "./verilog.g"
  {
    zzBLOCK(zztasp2);
    zzMake0;
    {
    while ( (LA(1)==V_COMMA) ) {
#line 1548 "./verilog.g"
      zzmatch(V_COMMA); zzCONSUME;
#line 1548 "./verilog.g"
      v_assignment();
      zzLOOP(zztasp2);
    }
    zzEXIT(zztasp2);
    }
  }
  zzEXIT(zztasp1);
  return;
fail:
  zzEXIT(zztasp1);
  zzsyn(zzMissText, zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk, zzBadText);
  zzresynch(setwd14, 0x4);
  }
}

void
#ifdef __USE_PROTOS
v_gate_declaration(void)
#else
v_gate_declaration()
#endif
{
#line 1556 "./verilog.g"
  zzRULE;
  zzBLOCK(zztasp1);
  zzMake0;
  {
#line 1556 "./verilog.g"
  v_gatetype();
#line 1556 "./verilog.g"
  v_gate_drv();
#line 1556 "./verilog.g"
  v_gate_dly();
#line 1556 "./verilog.g"
  v_gate_instance();
#line 1557 "./verilog.g"
  {
    zzBLOCK(zztasp2);
    zzMake0;
    {
    while ( (LA(1)==V_COMMA) ) {
#line 1557 "./verilog.g"
      zzmatch(V_COMMA); zzCONSUME;
#line 1557 "./verilog.g"
      v_gate_instance();
      zzLOOP(zztasp2);
    }
    zzEXIT(zztasp2);
    }
  }
#line 1557 "./verilog.g"
  zzmatch(V_SEMI); zzCONSUME;
  zzEXIT(zztasp1);
  return;
fail:
  zzEXIT(zztasp1);
  zzsyn(zzMissText, zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk, zzBadText);
  zzresynch(setwd14, 0x8);
  }
}

void
#ifdef __USE_PROTOS
v_gatetype(void)
#else
v_gatetype()
#endif
{
#line 1559 "./verilog.g"
  zzRULE;
  zzBLOCK(zztasp1);
  zzMake0;
  {
  if ( (LA(1)==V_ANDLIT) ) {
#line 1560 "./verilog.g"
    zzmatch(V_ANDLIT); zzCONSUME;
  }
  else {
    if ( (LA(1)==V_NANDLIT)
 ) {
#line 1560 "./verilog.g"
      zzmatch(V_NANDLIT); zzCONSUME;
    }
    else {
      if ( (LA(1)==V_ORLIT) ) {
#line 1560 "./verilog.g"
        zzmatch(V_ORLIT); zzCONSUME;
      }
      else {
        if ( (LA(1)==V_NORLIT) ) {
#line 1560 "./verilog.g"
          zzmatch(V_NORLIT); zzCONSUME;
        }
        else {
          if ( (LA(1)==V_XORLIT) ) {
#line 1560 "./verilog.g"
            zzmatch(V_XORLIT); zzCONSUME;
          }
          else {
            if ( (LA(1)==V_XNORLIT) ) {
#line 1560 "./verilog.g"
              zzmatch(V_XNORLIT); zzCONSUME;
            }
            else {
              if ( (LA(1)==V_BUF)
 ) {
#line 1561 "./verilog.g"
                zzmatch(V_BUF); zzCONSUME;
              }
              else {
                if ( (LA(1)==V_BUFIF0) ) {
#line 1561 "./verilog.g"
                  zzmatch(V_BUFIF0); zzCONSUME;
                }
                else {
                  if ( (LA(1)==V_BUFIF1) ) {
#line 1561 "./verilog.g"
                    zzmatch(V_BUFIF1); zzCONSUME;
                  }
                  else {
                    if ( (LA(1)==V_NOTLIT) ) {
#line 1561 "./verilog.g"
                      zzmatch(V_NOTLIT); zzCONSUME;
                    }
                    else {
                      if ( (LA(1)==V_NOTIF0) ) {
#line 1561 "./verilog.g"
                        zzmatch(V_NOTIF0); zzCONSUME;
                      }
                      else {
                        if ( (LA(1)==V_NOTIF1)
 ) {
#line 1561 "./verilog.g"
                          zzmatch(V_NOTIF1); zzCONSUME;
                        }
                        else {
                          if ( (LA(1)==V_PULLDOWN) ) {
#line 1561 "./verilog.g"
                            zzmatch(V_PULLDOWN); zzCONSUME;
                          }
                          else {
                            if ( (LA(1)==V_PULLUP) ) {
#line 1561 "./verilog.g"
                              zzmatch(V_PULLUP); zzCONSUME;
                            }
                            else {
                              if ( (LA(1)==V_NMOS) ) {
#line 1562 "./verilog.g"
                                zzmatch(V_NMOS); zzCONSUME;
                              }
                              else {
                                if ( (LA(1)==V_RNMOS) ) {
#line 1562 "./verilog.g"
                                  zzmatch(V_RNMOS); zzCONSUME;
                                }
                                else {
                                  if ( (LA(1)==V_PMOS)
 ) {
#line 1562 "./verilog.g"
                                    zzmatch(V_PMOS); zzCONSUME;
                                  }
                                  else {
                                    if ( (LA(1)==V_RPMOS) ) {
#line 1562 "./verilog.g"
                                      zzmatch(V_RPMOS); zzCONSUME;
                                    }
                                    else {
                                      if ( (LA(1)==V_CMOS) ) {
#line 1562 "./verilog.g"
                                        zzmatch(V_CMOS); zzCONSUME;
                                      }
                                      else {
                                        if ( (LA(1)==V_RCMOS) ) {
#line 1562 "./verilog.g"
                                          zzmatch(V_RCMOS); zzCONSUME;
                                        }
                                        else {
                                          if ( (LA(1)==V_TRAN) ) {
#line 1562 "./verilog.g"
                                            zzmatch(V_TRAN); zzCONSUME;
                                          }
                                          else {
                                            if ( (LA(1)==V_RTRAN)
 ) {
#line 1562 "./verilog.g"
                                              zzmatch(V_RTRAN); zzCONSUME;
                                            }
                                            else {
                                              if ( (LA(1)==V_TRANIF0) ) {
#line 1563 "./verilog.g"
                                                zzmatch(V_TRANIF0); zzCONSUME;
                                              }
                                              else {
                                                if ( (LA(1)==V_RTRANIF0) ) {
#line 1563 "./verilog.g"
                                                  zzmatch(V_RTRANIF0); zzCONSUME;
                                                }
                                                else {
                                                  if ( (LA(1)==V_TRANIF1) ) {
#line 1563 "./verilog.g"
                                                    zzmatch(V_TRANIF1); zzCONSUME;
                                                  }
                                                  else {
                                                    if ( (LA(1)==V_RTRANIF1) ) {
#line 1563 "./verilog.g"
                                                      zzmatch(V_RTRANIF1); zzCONSUME;
                                                    }
                                                    else {zzFAIL(1,zzerr45,&zzMissSet,&zzMissText,&zzBadTok,&zzBadText,&zzErrk); goto fail;}
                                                  }
                                                }
                                              }
                                            }
                                          }
                                        }
                                      }
                                    }
                                  }
                                }
                              }
                            }
                          }
                        }
                      }
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
  }
  zzEXIT(zztasp1);
  return;
fail:
  zzEXIT(zztasp1);
  zzsyn(zzMissText, zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk, zzBadText);
  zzresynch(setwd14, 0x10);
  }
}

void
#ifdef __USE_PROTOS
v_gate_drv(void)
#else
v_gate_drv()
#endif
{
#line 1566 "./verilog.g"
  zzRULE;
  zzBLOCK(zztasp1);
  zzMake0;
  {
  if ( (LA(1)==V_LP) && 
(setwd14[LA(2)]&0x20) ) {
#line 1566 "./verilog.g"
    v_drive_strength();
  }
  else {
    if ( (setwd14[LA(1)]&0x40) && (setwd14[LA(2)]&0x80) ) {
    }
    else {zzFAIL(2,zzerr46,zzerr47,&zzMissSet,&zzMissText,&zzBadTok,&zzBadText,&zzErrk); goto fail;}
  }
  zzEXIT(zztasp1);
  return;
fail:
  zzEXIT(zztasp1);
  zzsyn(zzMissText, zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk, zzBadText);
  zzresynch(setwd15, 0x1);
  }
}

void
#ifdef __USE_PROTOS
v_gate_dly(void)
#else
v_gate_dly()
#endif
{
#line 1570 "./verilog.g"
  zzRULE;
  zzBLOCK(zztasp1);
  zzMake0;
  {
  if ( (LA(1)==V_POUND) ) {
#line 1570 "./verilog.g"
    v_delay();
  }
  else {
    if ( (setwd15[LA(1)]&0x2) ) {
    }
    else {zzFAIL(1,zzerr48,&zzMissSet,&zzMissText,&zzBadTok,&zzBadText,&zzErrk); goto fail;}
  }
  zzEXIT(zztasp1);
  return;
fail:
  zzEXIT(zztasp1);
  zzsyn(zzMissText, zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk, zzBadText);
  zzresynch(setwd15, 0x4);
  }
}

void
#ifdef __USE_PROTOS
v_gate_range(void)
#else
v_gate_range()
#endif
{
#line 1574 "./verilog.g"
  zzRULE;
  zzBLOCK(zztasp1);
  zzMake0;
  {
  if ( (LA(1)==V_LBRACK) ) {
#line 1574 "./verilog.g"
    v_range();
  }
  else {
    if ( (LA(1)==V_LP)
 ) {
    }
    else {zzFAIL(1,zzerr49,&zzMissSet,&zzMissText,&zzBadTok,&zzBadText,&zzErrk); goto fail;}
  }
  zzEXIT(zztasp1);
  return;
fail:
  zzEXIT(zztasp1);
  zzsyn(zzMissText, zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk, zzBadText);
  zzresynch(setwd15, 0x8);
  }
}

void
#ifdef __USE_PROTOS
v_gate_instance(void)
#else
v_gate_instance()
#endif
{
#line 1579 "./verilog.g"
  zzRULE;
  zzBLOCK(zztasp1);
  zzMake0;
  {
#line 1579 "./verilog.g"
  v_name_of_gate_instance();
#line 1579 "./verilog.g"
  zzmatch(V_LP); zzCONSUME;
#line 1579 "./verilog.g"
  v_terminal();
#line 1580 "./verilog.g"
  {
    zzBLOCK(zztasp2);
    zzMake0;
    {
    while ( (LA(1)==V_COMMA) ) {
#line 1580 "./verilog.g"
      zzmatch(V_COMMA); zzCONSUME;
#line 1580 "./verilog.g"
      v_terminal();
      zzLOOP(zztasp2);
    }
    zzEXIT(zztasp2);
    }
  }
#line 1580 "./verilog.g"
  zzmatch(V_RP); zzCONSUME;
  zzEXIT(zztasp1);
  return;
fail:
  zzEXIT(zztasp1);
  zzsyn(zzMissText, zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk, zzBadText);
  zzresynch(setwd15, 0x10);
  }
}

void
#ifdef __USE_PROTOS
v_name_of_gate_instance(void)
#else
v_name_of_gate_instance()
#endif
{
#line 1583 "./verilog.g"
  zzRULE;
  zzBLOCK(zztasp1);
  zzMake0;
  {
  if ( (setwd15[LA(1)]&0x20) ) {
#line 1583 "./verilog.g"
    v_identifier_nodot();
#line 1583 "./verilog.g"
    v_gate_range();
  }
  else {
    if ( (LA(1)==V_LP) ) {
    }
    else {zzFAIL(1,zzerr50,&zzMissSet,&zzMissText,&zzBadTok,&zzBadText,&zzErrk); goto fail;}
  }
  zzEXIT(zztasp1);
  return;
fail:
  zzEXIT(zztasp1);
  zzsyn(zzMissText, zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk, zzBadText);
  zzresynch(setwd15, 0x40);
  }
}

void
#ifdef __USE_PROTOS
v_terminal(void)
#else
v_terminal()
#endif
{
#line 1587 "./verilog.g"
  zzRULE;
  zzBLOCK(zztasp1);
  zzMake0;
  {
  if ( (setwd15[LA(1)]&0x80) ) {
#line 1587 "./verilog.g"
    v_expression();
  }
  else {
    if ( (setwd16[LA(1)]&0x1)
 ) {
    }
    else {zzFAIL(1,zzerr51,&zzMissSet,&zzMissText,&zzBadTok,&zzBadText,&zzErrk); goto fail;}
  }
  zzEXIT(zztasp1);
  return;
fail:
  zzEXIT(zztasp1);
  zzsyn(zzMissText, zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk, zzBadText);
  zzresynch(setwd16, 0x2);
  }
}

void
#ifdef __USE_PROTOS
v_udp_instantiation(void)
#else
v_udp_instantiation()
#endif
{
#line 1591 "./verilog.g"
  zzRULE;
  zzBLOCK(zztasp1);
  zzMake0;
  {
#line 1591 "./verilog.g"
  v_name_of_udp();
#line 1591 "./verilog.g"
  v_gate_drv();
#line 1591 "./verilog.g"
  v_gate_dly();
#line 1591 "./verilog.g"
  v_udp_instance();
#line 1592 "./verilog.g"
  {
    zzBLOCK(zztasp2);
    zzMake0;
    {
    while ( (LA(1)==V_COMMA) ) {
#line 1592 "./verilog.g"
      zzmatch(V_COMMA); zzCONSUME;
#line 1592 "./verilog.g"
      v_udp_instance();
      zzLOOP(zztasp2);
    }
    zzEXIT(zztasp2);
    }
  }
#line 1592 "./verilog.g"
  zzmatch(V_SEMI); zzCONSUME;
  zzEXIT(zztasp1);
  return;
fail:
  zzEXIT(zztasp1);
  zzsyn(zzMissText, zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk, zzBadText);
  zzresynch(setwd16, 0x4);
  }
}

void
#ifdef __USE_PROTOS
v_name_of_udp(void)
#else
v_name_of_udp()
#endif
{
#line 1595 "./verilog.g"
  zzRULE;
  zzBLOCK(zztasp1);
  zzMake0;
  {
#line 1595 "./verilog.g"
  v_identifier_nodot();
#line 1596 "./verilog.g"
  zzaRet.symbol = zzaArg(zztasp1,1 ).symbol;
  zzEXIT(zztasp1);
  return;
fail:
  zzEXIT(zztasp1);
  zzsyn(zzMissText, zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk, zzBadText);
  zzresynch(setwd16, 0x8);
  }
}

void
#ifdef __USE_PROTOS
v_udp_instance(void)
#else
v_udp_instance()
#endif
{
#line 1599 "./verilog.g"
  zzRULE;
  zzBLOCK(zztasp1);
  zzMake0;
  {
#line 1599 "./verilog.g"
  v_name_of_udp_instance();
#line 1599 "./verilog.g"
  zzmatch(V_LP); zzCONSUME;
#line 1599 "./verilog.g"
  v_terminal();
#line 1600 "./verilog.g"
  {
    zzBLOCK(zztasp2);
    zzMake0;
    {
    while ( (LA(1)==V_COMMA) ) {
#line 1600 "./verilog.g"
      zzmatch(V_COMMA); zzCONSUME;
#line 1600 "./verilog.g"
      v_terminal();
      zzLOOP(zztasp2);
    }
    zzEXIT(zztasp2);
    }
  }
#line 1600 "./verilog.g"
  zzmatch(V_RP); zzCONSUME;
  zzEXIT(zztasp1);
  return;
fail:
  zzEXIT(zztasp1);
  zzsyn(zzMissText, zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk, zzBadText);
  zzresynch(setwd16, 0x10);
  }
}

void
#ifdef __USE_PROTOS
v_name_of_udp_instance(void)
#else
v_name_of_udp_instance()
#endif
{
#line 1603 "./verilog.g"
  zzRULE;
  zzBLOCK(zztasp1);
  zzMake0;
  {
  if ( (setwd16[LA(1)]&0x20) ) {
#line 1603 "./verilog.g"
    v_identifier_nodot();
#line 1603 "./verilog.g"
    v_gate_range();
  }
  else {
    if ( (LA(1)==V_LP) ) {
    }
    else {zzFAIL(1,zzerr52,&zzMissSet,&zzMissText,&zzBadTok,&zzBadText,&zzErrk); goto fail;}
  }
  zzEXIT(zztasp1);
  return;
fail:
  zzEXIT(zztasp1);
  zzsyn(zzMissText, zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk, zzBadText);
  zzresynch(setwd16, 0x40);
  }
}

void
#ifdef __USE_PROTOS
v_module_instantiation(void)
#else
v_module_instantiation()
#endif
{
#line 1611 "./verilog.g"
  zzRULE;
  zzBLOCK(zztasp1);
  zzMake0;
  {
#line 1611 "./verilog.g"
  v_name_of_module();
#line 1612 "./verilog.g"
  
  if(!module_is_duplicate)
  {
    add_string_to_tree(modname_tree, zzaArg(zztasp1,1 ).symbol->name, FALSE);
    if(comp_type_name)
    {
      free(comp_type_name);
    }
    comp_type_name = strdup(zzaArg(zztasp1,1 ).symbol->name);
  } /* to keep transitive closure from looking for nonexistant modules if duplicate modules differ! */
#line 1623 "./verilog.g"
  v_parameter_value_assignment();
#line 1624 "./verilog.g"
  v_module_instance();
#line 1625 "./verilog.g"
  {
    zzBLOCK(zztasp2);
    zzMake0;
    {
    while ( (LA(1)==V_COMMA)
 ) {
#line 1625 "./verilog.g"
      zzmatch(V_COMMA); zzCONSUME;
#line 1625 "./verilog.g"
      v_module_instance();
      zzLOOP(zztasp2);
    }
    zzEXIT(zztasp2);
    }
  }
#line 1625 "./verilog.g"
  zzmatch(V_SEMI);
#line 1626 "./verilog.g"
  
  if(comp_type_name)
  {
    free(comp_type_name);
    comp_type_name = NULL;
  }
 zzCONSUME;

  zzEXIT(zztasp1);
  return;
fail:
  zzEXIT(zztasp1);
  zzsyn(zzMissText, zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk, zzBadText);
  zzresynch(setwd16, 0x80);
  }
}

void
#ifdef __USE_PROTOS
v_name_of_module(void)
#else
v_name_of_module()
#endif
{
#line 1635 "./verilog.g"
  zzRULE;
  zzBLOCK(zztasp1);
  zzMake0;
  {
#line 1635 "./verilog.g"
  v_identifier_nodot();
#line 1635 "./verilog.g"
  zzaRet.symbol = zzaArg(zztasp1,1 ).symbol;
  zzEXIT(zztasp1);
  return;
fail:
  zzEXIT(zztasp1);
  zzsyn(zzMissText, zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk, zzBadText);
  zzresynch(setwd17, 0x1);
  }
}

void
#ifdef __USE_PROTOS
v_parameter_value_assignment(void)
#else
v_parameter_value_assignment()
#endif
{
#line 1638 "./verilog.g"
  zzRULE;
  zzBLOCK(zztasp1);
  zzMake0;
  {
  if ( (LA(1)==V_POUND) && (LA(2)==V_LP) ) {
#line 1638 "./verilog.g"
    zzmatch(V_POUND); zzCONSUME;
#line 1638 "./verilog.g"
    zzmatch(V_LP); zzCONSUME;
#line 1638 "./verilog.g"
    v_explist();
#line 1638 "./verilog.g"
    zzmatch(V_RP); zzCONSUME;
  }
  else {
    if ( (LA(1)==V_POUND) && (setwd17[LA(2)]&0x2) ) {
#line 1639 "./verilog.g"
      zzmatch(V_POUND); zzCONSUME;
#line 1639 "./verilog.g"
      v_number();
    }
    else {
      if ( (setwd17[LA(1)]&0x4) ) {
      }
      else {zzFAIL(2,zzerr53,zzerr54,&zzMissSet,&zzMissText,&zzBadTok,&zzBadText,&zzErrk); goto fail;}
    }
  }
  zzEXIT(zztasp1);
  return;
fail:
  zzEXIT(zztasp1);
  zzsyn(zzMissText, zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk, zzBadText);
  zzresynch(setwd17, 0x8);
  }
}

void
#ifdef __USE_PROTOS
v_module_instance(void)
#else
v_module_instance()
#endif
{
#line 1643 "./verilog.g"
  zzRULE;
  zzBLOCK(zztasp1);
  zzMake0;
  {
#line 1643 "./verilog.g"
  v_name_of_instance_opt();
#line 1643 "./verilog.g"
  zzmatch(V_LP); zzCONSUME;
#line 1643 "./verilog.g"
  v_list_of_module_connections();
#line 1643 "./verilog.g"
  zzmatch(V_RP); zzCONSUME;
  zzEXIT(zztasp1);
  return;
fail:
  zzEXIT(zztasp1);
  zzsyn(zzMissText, zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk, zzBadText);
  zzresynch(setwd17, 0x10);
  }
}

void
#ifdef __USE_PROTOS
v_name_of_instance_opt(void)
#else
v_name_of_instance_opt()
#endif
{
#line 1647 "./verilog.g"
  zzRULE;
  zzBLOCK(zztasp1);
  zzMake0;
  {
  if ( (setwd17[LA(1)]&0x20) ) {
#line 1647 "./verilog.g"
    v_name_of_instance();
  }
  else {
    if ( (LA(1)==V_LP)
 ) {
    }
    else {zzFAIL(1,zzerr55,&zzMissSet,&zzMissText,&zzBadTok,&zzBadText,&zzErrk); goto fail;}
  }
  zzEXIT(zztasp1);
  return;
fail:
  zzEXIT(zztasp1);
  zzsyn(zzMissText, zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk, zzBadText);
  zzresynch(setwd17, 0x40);
  }
}

void
#ifdef __USE_PROTOS
v_name_of_instance(void)
#else
v_name_of_instance()
#endif
{
#line 1651 "./verilog.g"
  zzRULE;
  zzBLOCK(zztasp1);
  zzMake0;
  {
#line 1651 "./verilog.g"
  v_identifier_nodot();
#line 1651 "./verilog.g"
  v_mod_range();
#line 1652 "./verilog.g"
  
  if(!module_is_duplicate)
  {
    if(emit_stems)
    {
      printf("++ comp %s type %s parent %s\n", zzaArg(zztasp1,1 ).symbol->name, comp_type_name, mod_current_name);
    }	
  }
  zzEXIT(zztasp1);
  return;
fail:
  zzEXIT(zztasp1);
  zzsyn(zzMissText, zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk, zzBadText);
  zzresynch(setwd17, 0x80);
  }
}

void
#ifdef __USE_PROTOS
v_mod_range(void)
#else
v_mod_range()
#endif
{
#line 1663 "./verilog.g"
  zzRULE;
  zzBLOCK(zztasp1);
  zzMake0;
  {
  if ( (LA(1)==V_LBRACK) ) {
#line 1663 "./verilog.g"
    v_range();
  }
  else {
    if ( (LA(1)==V_LP) ) {
    }
    else {zzFAIL(1,zzerr56,&zzMissSet,&zzMissText,&zzBadTok,&zzBadText,&zzErrk); goto fail;}
  }
  zzEXIT(zztasp1);
  return;
fail:
  zzEXIT(zztasp1);
  zzsyn(zzMissText, zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk, zzBadText);
  zzresynch(setwd18, 0x1);
  }
}

void
#ifdef __USE_PROTOS
v_list_of_module_connections(void)
#else
v_list_of_module_connections()
#endif
{
#line 1667 "./verilog.g"
  zzRULE;
  zzBLOCK(zztasp1);
  zzMake0;
  {
  if ( (setwd18[LA(1)]&0x2) ) {
#line 1667 "./verilog.g"
    v_module_port_connection();
#line 1667 "./verilog.g"
    {
      zzBLOCK(zztasp2);
      zzMake0;
      {
      while ( (LA(1)==V_COMMA) ) {
#line 1667 "./verilog.g"
        zzmatch(V_COMMA); zzCONSUME;
#line 1667 "./verilog.g"
        v_module_port_connection();
        zzLOOP(zztasp2);
      }
      zzEXIT(zztasp2);
      }
    }
  }
  else {
    if ( (LA(1)==V_DOT)
 ) {
#line 1668 "./verilog.g"
      v_named_port_connection();
#line 1668 "./verilog.g"
      {
        zzBLOCK(zztasp2);
        zzMake0;
        {
        while ( (LA(1)==V_COMMA) ) {
#line 1668 "./verilog.g"
          zzmatch(V_COMMA); zzCONSUME;
#line 1668 "./verilog.g"
          v_named_port_connection();
          zzLOOP(zztasp2);
        }
        zzEXIT(zztasp2);
        }
      }
    }
    else {zzFAIL(1,zzerr57,&zzMissSet,&zzMissText,&zzBadTok,&zzBadText,&zzErrk); goto fail;}
  }
  zzEXIT(zztasp1);
  return;
fail:
  zzEXIT(zztasp1);
  zzsyn(zzMissText, zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk, zzBadText);
  zzresynch(setwd18, 0x4);
  }
}

void
#ifdef __USE_PROTOS
v_module_port_connection(void)
#else
v_module_port_connection()
#endif
{
#line 1671 "./verilog.g"
  zzRULE;
  zzBLOCK(zztasp1);
  zzMake0;
  {
  if ( (setwd18[LA(1)]&0x8) ) {
#line 1671 "./verilog.g"
    v_expression();
  }
  else {
    if ( (setwd18[LA(1)]&0x10) ) {
    }
    else {zzFAIL(1,zzerr58,&zzMissSet,&zzMissText,&zzBadTok,&zzBadText,&zzErrk); goto fail;}
  }
  zzEXIT(zztasp1);
  return;
fail:
  zzEXIT(zztasp1);
  zzsyn(zzMissText, zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk, zzBadText);
  zzresynch(setwd18, 0x20);
  }
}

void
#ifdef __USE_PROTOS
v_named_port_connection(void)
#else
v_named_port_connection()
#endif
{
#line 1675 "./verilog.g"
  zzRULE;
  zzBLOCK(zztasp1);
  zzMake0;
  {
#line 1675 "./verilog.g"
  zzmatch(V_DOT); zzCONSUME;
#line 1675 "./verilog.g"
  v_identifier_nodot();
#line 1675 "./verilog.g"
  zzmatch(V_LP); zzCONSUME;
#line 1675 "./verilog.g"
  {
    zzBLOCK(zztasp2);
    zzMake0;
    {
    if ( (setwd18[LA(1)]&0x40) ) {
#line 1675 "./verilog.g"
      v_expression();
    }
    else {
      if ( (LA(1)==V_RP)
 ) {
      }
      else {zzFAIL(1,zzerr59,&zzMissSet,&zzMissText,&zzBadTok,&zzBadText,&zzErrk); goto fail;}
    }
    zzEXIT(zztasp2);
    }
  }
#line 1675 "./verilog.g"
  zzmatch(V_RP); zzCONSUME;
  zzEXIT(zztasp1);
  return;
fail:
  zzEXIT(zztasp1);
  zzsyn(zzMissText, zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk, zzBadText);
  zzresynch(setwd18, 0x80);
  }
}

void
#ifdef __USE_PROTOS
v_initial_statement(void)
#else
v_initial_statement()
#endif
{
#line 1683 "./verilog.g"
  zzRULE;
  zzBLOCK(zztasp1);
  zzMake0;
  {
#line 1683 "./verilog.g"
  zzmatch(V_INITIAL); zzCONSUME;
#line 1683 "./verilog.g"
  v_statement();
  zzEXIT(zztasp1);
  return;
fail:
  zzEXIT(zztasp1);
  zzsyn(zzMissText, zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk, zzBadText);
  zzresynch(setwd19, 0x1);
  }
}

void
#ifdef __USE_PROTOS
v_always_statement(void)
#else
v_always_statement()
#endif
{
#line 1686 "./verilog.g"
  zzRULE;
  zzBLOCK(zztasp1);
  zzMake0;
  {
#line 1686 "./verilog.g"
  zzmatch(V_ALWAYS); zzCONSUME;
#line 1686 "./verilog.g"
  v_statement();
  zzEXIT(zztasp1);
  return;
fail:
  zzEXIT(zztasp1);
  zzsyn(zzMissText, zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk, zzBadText);
  zzresynch(setwd19, 0x2);
  }
}

void
#ifdef __USE_PROTOS
v_statement_or_null(void)
#else
v_statement_or_null()
#endif
{
#line 1689 "./verilog.g"
  zzRULE;
  zzBLOCK(zztasp1);
  zzMake0;
  {
  if ( (setwd19[LA(1)]&0x4) ) {
#line 1689 "./verilog.g"
    v_statement();
  }
  else {
    if ( (LA(1)==V_SEMI) ) {
#line 1690 "./verilog.g"
      zzmatch(V_SEMI); zzCONSUME;
    }
    else {zzFAIL(1,zzerr60,&zzMissSet,&zzMissText,&zzBadTok,&zzBadText,&zzErrk); goto fail;}
  }
  zzEXIT(zztasp1);
  return;
fail:
  zzEXIT(zztasp1);
  zzsyn(zzMissText, zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk, zzBadText);
  zzresynch(setwd19, 0x8);
  }
}

void
#ifdef __USE_PROTOS
v_statement(void)
#else
v_statement()
#endif
{
#line 1693 "./verilog.g"
  zzRULE;
  zzBLOCK(zztasp1);
  zzMake0;
  {
  if ( (setwd19[LA(1)]&0x10) && (setwd19[LA(2)]&0x20) && !(
 LA(1)==V_IDENTIFIER && LA(2)==V_LP
|| LA(1)==V_IDENTIFIER2 && LA(2)==V_LP
|| LA(1)==V_FUNCTION_NAME && LA(2)==V_LP
) ) {
#line 1693 "./verilog.g"
    v_block_or_non_assignment();
#line 1693 "./verilog.g"
    zzmatch(V_SEMI); zzCONSUME;
  }
  else {
    if ( (LA(1)==V_IF) ) {
#line 1694 "./verilog.g"
      zzmatch(V_IF); zzCONSUME;
#line 1694 "./verilog.g"
      zzmatch(V_LP); zzCONSUME;
#line 1694 "./verilog.g"
      v_expression();
#line 1694 "./verilog.g"
      zzmatch(V_RP); zzCONSUME;
#line 1694 "./verilog.g"
      v_statement_or_null();
#line 1695 "./verilog.g"
      {
        zzBLOCK(zztasp2);
        zzMake0;
        {
        if ( (LA(1)==V_ELSE) && 
(setwd19[LA(2)]&0x40) ) {
#line 1695 "./verilog.g"
          zzmatch(V_ELSE); zzCONSUME;
#line 1695 "./verilog.g"
          v_statement_or_null();
        }
        else {
          if ( (setwd19[LA(1)]&0x80) && (setwd20[LA(2)]&0x1) ) {
          }
          else {zzFAIL(2,zzerr61,zzerr62,&zzMissSet,&zzMissText,&zzBadTok,&zzBadText,&zzErrk); goto fail;}
        }
        zzEXIT(zztasp2);
        }
      }
    }
    else {
      if ( (LA(1)==V_CASE) ) {
#line 1696 "./verilog.g"
        zzmatch(V_CASE); zzCONSUME;
#line 1696 "./verilog.g"
        zzmatch(V_LP); zzCONSUME;
#line 1696 "./verilog.g"
        v_expression();
#line 1696 "./verilog.g"
        zzmatch(V_RP); zzCONSUME;
#line 1696 "./verilog.g"
        {
          zzBLOCK(zztasp2);
          int zzcnt=1;
          zzMake0;
          {
          do {
#line 1696 "./verilog.g"
            v_case_item();
            zzLOOP(zztasp2);
          } while ( (setwd20[LA(1)]&0x2) );
          zzEXIT(zztasp2);
          }
        }
#line 1696 "./verilog.g"
        zzmatch(V_ENDCASE); zzCONSUME;
      }
      else {
        if ( (LA(1)==V_CASEX) ) {
#line 1697 "./verilog.g"
          zzmatch(V_CASEX); zzCONSUME;
#line 1697 "./verilog.g"
          zzmatch(V_LP); zzCONSUME;
#line 1697 "./verilog.g"
          v_expression();
#line 1697 "./verilog.g"
          zzmatch(V_RP); zzCONSUME;
#line 1697 "./verilog.g"
          {
            zzBLOCK(zztasp2);
            int zzcnt=1;
            zzMake0;
            {
            do {
#line 1697 "./verilog.g"
              v_case_item();
              zzLOOP(zztasp2);
            } while ( (setwd20[LA(1)]&0x4)
 );
            zzEXIT(zztasp2);
            }
          }
#line 1697 "./verilog.g"
          zzmatch(V_ENDCASE); zzCONSUME;
        }
        else {
          if ( (LA(1)==V_CASEZ) ) {
#line 1698 "./verilog.g"
            zzmatch(V_CASEZ); zzCONSUME;
#line 1698 "./verilog.g"
            zzmatch(V_LP); zzCONSUME;
#line 1698 "./verilog.g"
            v_expression();
#line 1698 "./verilog.g"
            zzmatch(V_RP); zzCONSUME;
#line 1698 "./verilog.g"
            {
              zzBLOCK(zztasp2);
              int zzcnt=1;
              zzMake0;
              {
              do {
#line 1698 "./verilog.g"
                v_case_item();
                zzLOOP(zztasp2);
              } while ( (setwd20[LA(1)]&0x8) );
              zzEXIT(zztasp2);
              }
            }
#line 1698 "./verilog.g"
            zzmatch(V_ENDCASE); zzCONSUME;
          }
          else {
            if ( (LA(1)==V_FOREVER) ) {
#line 1699 "./verilog.g"
              zzmatch(V_FOREVER); zzCONSUME;
#line 1699 "./verilog.g"
              v_statement();
            }
            else {
              if ( (LA(1)==V_REPEAT) ) {
#line 1700 "./verilog.g"
                zzmatch(V_REPEAT); zzCONSUME;
#line 1700 "./verilog.g"
                zzmatch(V_LP); zzCONSUME;
#line 1700 "./verilog.g"
                v_expression();
#line 1700 "./verilog.g"
                zzmatch(V_RP); zzCONSUME;
#line 1700 "./verilog.g"
                v_statement();
              }
              else {
                if ( (LA(1)==V_WHILE)
 ) {
#line 1701 "./verilog.g"
                  zzmatch(V_WHILE); zzCONSUME;
#line 1701 "./verilog.g"
                  zzmatch(V_LP); zzCONSUME;
#line 1701 "./verilog.g"
                  v_expression();
#line 1701 "./verilog.g"
                  zzmatch(V_RP); zzCONSUME;
#line 1701 "./verilog.g"
                  v_statement();
                }
                else {
                  if ( (LA(1)==V_FOR) ) {
#line 1702 "./verilog.g"
                    zzmatch(V_FOR); zzCONSUME;
#line 1702 "./verilog.g"
                    zzmatch(V_LP); zzCONSUME;
#line 1702 "./verilog.g"
                    v_assignment();
#line 1702 "./verilog.g"
                    zzmatch(V_SEMI); zzCONSUME;
#line 1702 "./verilog.g"
                    v_expression();
#line 1702 "./verilog.g"
                    zzmatch(V_SEMI); zzCONSUME;
#line 1703 "./verilog.g"
                    v_assignment();
#line 1703 "./verilog.g"
                    zzmatch(V_RP); zzCONSUME;
#line 1703 "./verilog.g"
                    v_statement();
                  }
                  else {
                    if ( (setwd20[LA(1)]&0x10) ) {
#line 1704 "./verilog.g"
                      v_delay_or_event_control_stmt();
#line 1704 "./verilog.g"
                      v_statement_or_null();
                    }
                    else {
                      if ( (LA(1)==V_WAIT) ) {
#line 1705 "./verilog.g"
                        zzmatch(V_WAIT); zzCONSUME;
#line 1705 "./verilog.g"
                        zzmatch(V_LP); zzCONSUME;
#line 1705 "./verilog.g"
                        v_expression();
#line 1705 "./verilog.g"
                        zzmatch(V_RP); zzCONSUME;
#line 1705 "./verilog.g"
                        v_statement_or_null();
                      }
                      else {
                        if ( (LA(1)==V_RARROW) ) {
#line 1706 "./verilog.g"
                          zzmatch(V_RARROW); zzCONSUME;
#line 1706 "./verilog.g"
                          v_name_of_event();
#line 1706 "./verilog.g"
                          zzmatch(V_SEMI); zzCONSUME;
                        }
                        else {
                          if ( (LA(1)==V_BEGIN)
 ) {
#line 1707 "./verilog.g"
                            v_seq_block();
                          }
                          else {
                            if ( (LA(1)==V_FORK) ) {
#line 1708 "./verilog.g"
                              v_par_block();
                            }
                            else {
                              if ( (setwd20[LA(1)]&0x20) && (setwd20[LA(2)]&0x40) ) {
#line 1709 "./verilog.g"
                                v_task_enable();
                              }
                              else {
                                if ( (LA(1)==V_DISABLE) ) {
#line 1710 "./verilog.g"
                                  zzmatch(V_DISABLE); zzCONSUME;
#line 1710 "./verilog.g"
                                  {
                                    zzBLOCK(zztasp2);
                                    zzMake0;
                                    {
#line 1710 "./verilog.g"
                                    v_name_of_task_or_block();
                                    zzEXIT(zztasp2);
                                    }
                                  }
#line 1710 "./verilog.g"
                                  zzmatch(V_SEMI); zzCONSUME;
                                }
                                else {
                                  if ( (LA(1)==V_ASSIGN) ) {
#line 1711 "./verilog.g"
                                    zzmatch(V_ASSIGN); zzCONSUME;
#line 1711 "./verilog.g"
                                    v_assignment();
#line 1711 "./verilog.g"
                                    zzmatch(V_SEMI); zzCONSUME;
                                  }
                                  else {
                                    if ( (LA(1)==V_DEASSIGN)
 ) {
#line 1712 "./verilog.g"
                                      zzmatch(V_DEASSIGN); zzCONSUME;
#line 1712 "./verilog.g"
                                      v_lvalue();
#line 1712 "./verilog.g"
                                      zzmatch(V_SEMI); zzCONSUME;
                                    }
                                    else {
                                      if ( (LA(1)==V_FORCE) ) {
#line 1713 "./verilog.g"
                                        zzmatch(V_FORCE); zzCONSUME;
#line 1713 "./verilog.g"
                                        v_assignment();
#line 1713 "./verilog.g"
                                        zzmatch(V_SEMI); zzCONSUME;
                                      }
                                      else {
                                        if ( (LA(1)==V_RELEASE) ) {
#line 1714 "./verilog.g"
                                          zzmatch(V_RELEASE); zzCONSUME;
#line 1714 "./verilog.g"
                                          v_lvalue();
#line 1714 "./verilog.g"
                                          zzmatch(V_SEMI); zzCONSUME;
                                        }
                                        else {zzFAIL(2,zzerr63,zzerr64,&zzMissSet,&zzMissText,&zzBadTok,&zzBadText,&zzErrk); goto fail;}
                                      }
                                    }
                                  }
                                }
                              }
                            }
                          }
                        }
                      }
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
  }
  zzEXIT(zztasp1);
  return;
fail:
  zzEXIT(zztasp1);
  zzsyn(zzMissText, zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk, zzBadText);
  zzresynch(setwd20, 0x80);
  }
}

void
#ifdef __USE_PROTOS
v_assignment(void)
#else
v_assignment()
#endif
{
#line 1717 "./verilog.g"
  zzRULE;
  zzBLOCK(zztasp1);
  zzMake0;
  {
#line 1717 "./verilog.g"
  v_lvalue();
#line 1717 "./verilog.g"
  zzmatch(V_EQ); zzCONSUME;
#line 1717 "./verilog.g"
  v_expression();
  zzEXIT(zztasp1);
  return;
fail:
  zzEXIT(zztasp1);
  zzsyn(zzMissText, zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk, zzBadText);
  zzresynch(setwd21, 0x1);
  }
}

void
#ifdef __USE_PROTOS
v_block_or_non_assignment(void)
#else
v_block_or_non_assignment()
#endif
{
#line 1720 "./verilog.g"
  zzRULE;
  zzBLOCK(zztasp1);
  zzMake0;
  {
#line 1720 "./verilog.g"
  v_lvalue();
#line 1720 "./verilog.g"
  {
    zzBLOCK(zztasp2);
    zzMake0;
    {
    if ( (LA(1)==V_EQ) ) {
#line 1720 "./verilog.g"
      v_blocking_assignment();
    }
    else {
      if ( (LA(1)==V_LEQ) ) {
#line 1721 "./verilog.g"
        v_non_blocking_assignment();
      }
      else {zzFAIL(1,zzerr65,&zzMissSet,&zzMissText,&zzBadTok,&zzBadText,&zzErrk); goto fail;}
    }
    zzEXIT(zztasp2);
    }
  }
  zzEXIT(zztasp1);
  return;
fail:
  zzEXIT(zztasp1);
  zzsyn(zzMissText, zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk, zzBadText);
  zzresynch(setwd21, 0x2);
  }
}

void
#ifdef __USE_PROTOS
v_blocking_assignment(void)
#else
v_blocking_assignment()
#endif
{
#line 1724 "./verilog.g"
  zzRULE;
  zzBLOCK(zztasp1);
  zzMake0;
  {
#line 1724 "./verilog.g"
  zzmatch(V_EQ); zzCONSUME;
#line 1724 "./verilog.g"
  {
    zzBLOCK(zztasp2);
    zzMake0;
    {
    if ( (setwd21[LA(1)]&0x4)
 ) {
#line 1724 "./verilog.g"
      v_expression();
    }
    else {
      if ( (setwd21[LA(1)]&0x8) ) {
#line 1725 "./verilog.g"
        v_delay_or_event_control();
#line 1725 "./verilog.g"
        v_expression();
      }
      else {zzFAIL(1,zzerr66,&zzMissSet,&zzMissText,&zzBadTok,&zzBadText,&zzErrk); goto fail;}
    }
    zzEXIT(zztasp2);
    }
  }
  zzEXIT(zztasp1);
  return;
fail:
  zzEXIT(zztasp1);
  zzsyn(zzMissText, zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk, zzBadText);
  zzresynch(setwd21, 0x10);
  }
}

void
#ifdef __USE_PROTOS
v_non_blocking_assignment(void)
#else
v_non_blocking_assignment()
#endif
{
#line 1728 "./verilog.g"
  zzRULE;
  zzBLOCK(zztasp1);
  zzMake0;
  {
#line 1728 "./verilog.g"
  zzmatch(V_LEQ); zzCONSUME;
#line 1728 "./verilog.g"
  {
    zzBLOCK(zztasp2);
    zzMake0;
    {
    if ( (setwd21[LA(1)]&0x20) ) {
#line 1728 "./verilog.g"
      v_expression();
    }
    else {
      if ( (setwd21[LA(1)]&0x40) ) {
#line 1729 "./verilog.g"
        v_delay_or_event_control();
#line 1729 "./verilog.g"
        v_expression();
      }
      else {zzFAIL(1,zzerr67,&zzMissSet,&zzMissText,&zzBadTok,&zzBadText,&zzErrk); goto fail;}
    }
    zzEXIT(zztasp2);
    }
  }
  zzEXIT(zztasp1);
  return;
fail:
  zzEXIT(zztasp1);
  zzsyn(zzMissText, zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk, zzBadText);
  zzresynch(setwd21, 0x80);
  }
}

void
#ifdef __USE_PROTOS
v_delay_or_event_control(void)
#else
v_delay_or_event_control()
#endif
{
#line 1732 "./verilog.g"
  zzRULE;
  zzBLOCK(zztasp1);
  zzMake0;
  {
  if ( (LA(1)==V_POUND) ) {
#line 1732 "./verilog.g"
    v_delay_control();
  }
  else {
    if ( (LA(1)==V_AT)
 ) {
#line 1733 "./verilog.g"
      v_event_control();
    }
    else {
      if ( (LA(1)==V_REPEAT) ) {
#line 1734 "./verilog.g"
        zzmatch(V_REPEAT); zzCONSUME;
#line 1734 "./verilog.g"
        zzmatch(V_LP); zzCONSUME;
#line 1734 "./verilog.g"
        v_expression();
#line 1734 "./verilog.g"
        zzmatch(V_RP); zzCONSUME;
#line 1734 "./verilog.g"
        v_event_control();
      }
      else {zzFAIL(1,zzerr68,&zzMissSet,&zzMissText,&zzBadTok,&zzBadText,&zzErrk); goto fail;}
    }
  }
  zzEXIT(zztasp1);
  return;
fail:
  zzEXIT(zztasp1);
  zzsyn(zzMissText, zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk, zzBadText);
  zzresynch(setwd22, 0x1);
  }
}

void
#ifdef __USE_PROTOS
v_delay_or_event_control_stmt(void)
#else
v_delay_or_event_control_stmt()
#endif
{
#line 1737 "./verilog.g"
  zzRULE;
  zzBLOCK(zztasp1);
  zzMake0;
  {
  if ( (LA(1)==V_POUND) ) {
#line 1737 "./verilog.g"
    v_delay_control();
  }
  else {
    if ( (LA(1)==V_AT) ) {
#line 1738 "./verilog.g"
      v_event_control();
    }
    else {zzFAIL(1,zzerr69,&zzMissSet,&zzMissText,&zzBadTok,&zzBadText,&zzErrk); goto fail;}
  }
  zzEXIT(zztasp1);
  return;
fail:
  zzEXIT(zztasp1);
  zzsyn(zzMissText, zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk, zzBadText);
  zzresynch(setwd22, 0x2);
  }
}

void
#ifdef __USE_PROTOS
v_case_item(void)
#else
v_case_item()
#endif
{
#line 1741 "./verilog.g"
  zzRULE;
  zzBLOCK(zztasp1);
  zzMake0;
  {
  if ( (setwd22[LA(1)]&0x4) ) {
#line 1741 "./verilog.g"
    v_explist();
#line 1742 "./verilog.g"
    zzmatch(V_COLON); zzCONSUME;
#line 1742 "./verilog.g"
    v_statement_or_null();
  }
  else {
    if ( (LA(1)==V_DEFAULT)
 ) {
#line 1743 "./verilog.g"
      zzmatch(V_DEFAULT); zzCONSUME;
#line 1743 "./verilog.g"
      {
        zzBLOCK(zztasp2);
        zzMake0;
        {
        if ( (LA(1)==V_COLON) ) {
#line 1743 "./verilog.g"
          zzmatch(V_COLON); zzCONSUME;
#line 1743 "./verilog.g"
          v_statement_or_null();
        }
        else {
          if ( (setwd22[LA(1)]&0x8) ) {
#line 1744 "./verilog.g"
            v_statement_or_null();
          }
          else {zzFAIL(1,zzerr70,&zzMissSet,&zzMissText,&zzBadTok,&zzBadText,&zzErrk); goto fail;}
        }
        zzEXIT(zztasp2);
        }
      }
    }
    else {zzFAIL(1,zzerr71,&zzMissSet,&zzMissText,&zzBadTok,&zzBadText,&zzErrk); goto fail;}
  }
  zzEXIT(zztasp1);
  return;
fail:
  zzEXIT(zztasp1);
  zzsyn(zzMissText, zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk, zzBadText);
  zzresynch(setwd22, 0x10);
  }
}

void
#ifdef __USE_PROTOS
v_seq_block(void)
#else
v_seq_block()
#endif
{
#line 1747 "./verilog.g"
  zzRULE;
  zzBLOCK(zztasp1);
  zzMake0;
  {
#line 1747 "./verilog.g"
  zzmatch(V_BEGIN);
#line 1748 "./verilog.g"
  { 
    struct i_symbol_scope *sb = (struct i_symbol_scope *)calloc(1, sizeof(struct i_symbol_scope)); 
    sb->symtable = make_jrb(); 
    sb->parent = sym_base;
    sym_base = sb;
  }
 zzCONSUME;

#line 1755 "./verilog.g"
  {
    zzBLOCK(zztasp2);
    zzMake0;
    {
    if ( (setwd22[LA(1)]&0x20) ) {
#line 1755 "./verilog.g"
      {
        zzBLOCK(zztasp3);
        zzMake0;
        {
        while ( (setwd22[LA(1)]&0x40) ) {
#line 1755 "./verilog.g"
          v_statement();
          zzLOOP(zztasp3);
        }
        zzEXIT(zztasp3);
        }
      }
    }
    else {
      if ( (LA(1)==V_COLON)
 ) {
#line 1756 "./verilog.g"
        zzmatch(V_COLON); zzCONSUME;
#line 1756 "./verilog.g"
        v_name_of_block();
#line 1756 "./verilog.g"
        {
          zzBLOCK(zztasp3);
          zzMake0;
          {
          while ( (setwd22[LA(1)]&0x80) ) {
#line 1756 "./verilog.g"
            v_block_declaration();
            zzLOOP(zztasp3);
          }
          zzEXIT(zztasp3);
          }
        }
#line 1757 "./verilog.g"
        {
          zzBLOCK(zztasp3);
          zzMake0;
          {
          while ( (setwd23[LA(1)]&0x1) ) {
#line 1757 "./verilog.g"
            v_statement();
            zzLOOP(zztasp3);
          }
          zzEXIT(zztasp3);
          }
        }
      }
      else {zzFAIL(1,zzerr72,&zzMissSet,&zzMissText,&zzBadTok,&zzBadText,&zzErrk); goto fail;}
    }
    zzEXIT(zztasp2);
    }
  }
#line 1758 "./verilog.g"
  zzmatch(V_END);
#line 1759 "./verilog.g"
  if(sym_base) sym_base = sym_base->parent;
 zzCONSUME;

  zzEXIT(zztasp1);
  return;
fail:
  zzEXIT(zztasp1);
  zzsyn(zzMissText, zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk, zzBadText);
  zzresynch(setwd23, 0x2);
  }
}

void
#ifdef __USE_PROTOS
v_par_block(void)
#else
v_par_block()
#endif
{
#line 1762 "./verilog.g"
  zzRULE;
  zzBLOCK(zztasp1);
  zzMake0;
  {
#line 1762 "./verilog.g"
  zzmatch(V_FORK);
#line 1763 "./verilog.g"
  { 
    struct i_symbol_scope *sb = (struct i_symbol_scope *)calloc(1, sizeof(struct i_symbol_scope)); 
    sb->symtable = make_jrb(); 
    sb->parent = sym_base;
    sym_base = sb;
  }
 zzCONSUME;

#line 1770 "./verilog.g"
  {
    zzBLOCK(zztasp2);
    zzMake0;
    {
    if ( (setwd23[LA(1)]&0x4) ) {
#line 1770 "./verilog.g"
      {
        zzBLOCK(zztasp3);
        zzMake0;
        {
        while ( (setwd23[LA(1)]&0x8) ) {
#line 1770 "./verilog.g"
          v_statement();
          zzLOOP(zztasp3);
        }
        zzEXIT(zztasp3);
        }
      }
    }
    else {
      if ( (LA(1)==V_COLON)
 ) {
#line 1771 "./verilog.g"
        zzmatch(V_COLON); zzCONSUME;
#line 1771 "./verilog.g"
        v_name_of_block();
#line 1771 "./verilog.g"
        {
          zzBLOCK(zztasp3);
          zzMake0;
          {
          while ( (setwd23[LA(1)]&0x10) ) {
#line 1771 "./verilog.g"
            v_block_declaration();
            zzLOOP(zztasp3);
          }
          zzEXIT(zztasp3);
          }
        }
#line 1772 "./verilog.g"
        {
          zzBLOCK(zztasp3);
          zzMake0;
          {
          while ( (setwd23[LA(1)]&0x20) ) {
#line 1772 "./verilog.g"
            v_statement();
            zzLOOP(zztasp3);
          }
          zzEXIT(zztasp3);
          }
        }
      }
      else {zzFAIL(1,zzerr73,&zzMissSet,&zzMissText,&zzBadTok,&zzBadText,&zzErrk); goto fail;}
    }
    zzEXIT(zztasp2);
    }
  }
#line 1772 "./verilog.g"
  zzmatch(V_JOIN);
#line 1773 "./verilog.g"
  if(sym_base) sym_base = sym_base->parent;
 zzCONSUME;

  zzEXIT(zztasp1);
  return;
fail:
  zzEXIT(zztasp1);
  zzsyn(zzMissText, zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk, zzBadText);
  zzresynch(setwd23, 0x40);
  }
}

void
#ifdef __USE_PROTOS
v_name_of_block(void)
#else
v_name_of_block()
#endif
{
#line 1776 "./verilog.g"
  zzRULE;
  zzBLOCK(zztasp1);
  zzMake0;
  {
#line 1776 "./verilog.g"
  v_identifier_nodot();
  zzEXIT(zztasp1);
  return;
fail:
  zzEXIT(zztasp1);
  zzsyn(zzMissText, zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk, zzBadText);
  zzresynch(setwd23, 0x80);
  }
}

void
#ifdef __USE_PROTOS
v_block_declaration(void)
#else
v_block_declaration()
#endif
{
#line 1779 "./verilog.g"
  zzRULE;
  zzBLOCK(zztasp1);
  zzMake0;
  {
  if ( (LA(1)==V_PARAMETER) ) {
#line 1779 "./verilog.g"
    v_parameter_declaration();
  }
  else {
    if ( (LA(1)==V_REG) ) {
#line 1780 "./verilog.g"
      v_reg_declaration();
    }
    else {
      if ( (LA(1)==V_INTEGER)
 ) {
#line 1781 "./verilog.g"
        v_integer_declaration();
      }
      else {
        if ( (LA(1)==V_REAL) ) {
#line 1782 "./verilog.g"
          v_real_declaration();
        }
        else {
          if ( (LA(1)==V_TIME) ) {
#line 1783 "./verilog.g"
            v_time_declaration();
          }
          else {
            if ( (LA(1)==V_EVENT) ) {
#line 1784 "./verilog.g"
              v_event_declaration();
            }
            else {zzFAIL(1,zzerr74,&zzMissSet,&zzMissText,&zzBadTok,&zzBadText,&zzErrk); goto fail;}
          }
        }
      }
    }
  }
  zzEXIT(zztasp1);
  return;
fail:
  zzEXIT(zztasp1);
  zzsyn(zzMissText, zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk, zzBadText);
  zzresynch(setwd24, 0x1);
  }
}

void
#ifdef __USE_PROTOS
v_task_enable(void)
#else
v_task_enable()
#endif
{
#line 1787 "./verilog.g"
  zzRULE;
  zzBLOCK(zztasp1);
  zzMake0;
  {
  if ( (setwd24[LA(1)]&0x2) && (LA(2)==V_SEMI) ) {
#line 1787 "./verilog.g"
    v_name_of_task();
#line 1787 "./verilog.g"
    zzmatch(V_SEMI); zzCONSUME;
  }
  else {
    if ( (setwd24[LA(1)]&0x4) && 
(LA(2)==V_LP) ) {
#line 1788 "./verilog.g"
      v_name_of_task();
#line 1788 "./verilog.g"
      zzmatch(V_LP); zzCONSUME;
#line 1788 "./verilog.g"
      v_explist();
#line 1788 "./verilog.g"
      zzmatch(V_RP); zzCONSUME;
#line 1788 "./verilog.g"
      zzmatch(V_SEMI); zzCONSUME;
    }
    else {zzFAIL(2,zzerr75,zzerr76,&zzMissSet,&zzMissText,&zzBadTok,&zzBadText,&zzErrk); goto fail;}
  }
  zzEXIT(zztasp1);
  return;
fail:
  zzEXIT(zztasp1);
  zzsyn(zzMissText, zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk, zzBadText);
  zzresynch(setwd24, 0x8);
  }
}

void
#ifdef __USE_PROTOS
v_name_of_task(void)
#else
v_name_of_task()
#endif
{
#line 1791 "./verilog.g"
  zzRULE;
  zzBLOCK(zztasp1);
  zzMake0;
  {
#line 1791 "./verilog.g"
  v_identifier_nodot();
  zzEXIT(zztasp1);
  return;
fail:
  zzEXIT(zztasp1);
  zzsyn(zzMissText, zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk, zzBadText);
  zzresynch(setwd24, 0x10);
  }
}

void
#ifdef __USE_PROTOS
v_name_of_task_or_block(void)
#else
v_name_of_task_or_block()
#endif
{
#line 1795 "./verilog.g"
  zzRULE;
  zzBLOCK(zztasp1);
  zzMake0;
  {
#line 1795 "./verilog.g"
  v_identifier_nodot();
  zzEXIT(zztasp1);
  return;
fail:
  zzEXIT(zztasp1);
  zzsyn(zzMissText, zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk, zzBadText);
  zzresynch(setwd24, 0x20);
  }
}

void
#ifdef __USE_PROTOS
v_specify_block(void)
#else
v_specify_block()
#endif
{
#line 1802 "./verilog.g"
  zzRULE;
  zzBLOCK(zztasp1);
  zzMake0;
  {
#line 1802 "./verilog.g"
  zzmatch(V_SPECIFY); zzCONSUME;
#line 1802 "./verilog.g"
  {
    zzBLOCK(zztasp2);
    zzMake0;
    {
    while ( (setwd24[LA(1)]&0x40) ) {
#line 1802 "./verilog.g"
      zzsetmatch(zzerr77, zzerr78); zzCONSUME;
      zzLOOP(zztasp2);
    }
    zzEXIT(zztasp2);
    }
  }
#line 1802 "./verilog.g"
  zzmatch(V_ENDSPECIFY); zzCONSUME;
  zzEXIT(zztasp1);
  return;
fail:
  zzEXIT(zztasp1);
  zzsyn(zzMissText, zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk, zzBadText);
  zzresynch(setwd24, 0x80);
  }
}

void
#ifdef __USE_PROTOS
v_constant_expression(void)
#else
v_constant_expression()
#endif
{
#line 1805 "./verilog.g"
  zzRULE;
  zzBLOCK(zztasp1);
  zzMake0;
  {
#line 1805 "./verilog.g"
  v_expression();
#line 1806 "./verilog.g"
  zzaRet.prim = zzaArg(zztasp1,1 ).prim;
  zzEXIT(zztasp1);
  return;
fail:
  zzEXIT(zztasp1);
  zzsyn(zzMissText, zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk, zzBadText);
  zzresynch(setwd25, 0x1);
  }
}

void
#ifdef __USE_PROTOS
v_lvalue(void)
#else
v_lvalue()
#endif
{
#line 1813 "./verilog.g"
  zzRULE;
  zzBLOCK(zztasp1);
  zzMake0;
  {
  if ( (setwd25[LA(1)]&0x2) ) {
#line 1813 "./verilog.g"
    v_identifier();
#line 1815 "./verilog.g"
    {
      zzBLOCK(zztasp2);
      zzMake0;
      {
      if ( (LA(1)==V_LBRACK) ) {
#line 1815 "./verilog.g"
        zzmatch(V_LBRACK); zzCONSUME;
#line 1815 "./verilog.g"
        v_expression();
#line 1816 "./verilog.g"
        {
          zzBLOCK(zztasp3);
          zzMake0;
          {
          if ( (LA(1)==V_COLON) ) {
#line 1816 "./verilog.g"
            zzmatch(V_COLON); zzCONSUME;
#line 1816 "./verilog.g"
            v_expression();
#line 1817 "./verilog.g"
            zzaRet.prim = i_primary_symrange_make(zzaArg(zztasp1,1).symbol,zzaArg(zztasp2,2).prim,zzaArg(zztasp3,2).prim);
          }
          else {
            if ( (LA(1)==V_RBRACK)
 ) {
#line 1818 "./verilog.g"
              zzaRet.prim = i_primary_symbit_make(zzaArg(zztasp1,1).symbol,zzaArg(zztasp2,2).prim);
            }
            else {zzFAIL(1,zzerr79,&zzMissSet,&zzMissText,&zzBadTok,&zzBadText,&zzErrk); goto fail;}
          }
          zzEXIT(zztasp3);
          }
        }
#line 1819 "./verilog.g"
        zzmatch(V_RBRACK); zzCONSUME;
      }
      else {
        if ( (setwd25[LA(1)]&0x4) ) {
#line 1819 "./verilog.g"
          zzaRet.prim = i_primary_make(PRIM_SYMBOL,zzaArg(zztasp1,1).symbol);
        }
        else {zzFAIL(1,zzerr80,&zzMissSet,&zzMissText,&zzBadTok,&zzBadText,&zzErrk); goto fail;}
      }
      zzEXIT(zztasp2);
      }
    }
  }
  else {
    if ( (LA(1)==V_LBRACE) ) {
#line 1821 "./verilog.g"
      v_concatenation();
#line 1821 "./verilog.g"
      zzaRet.prim = zzaArg(zztasp1,1 ).prim;
    }
    else {zzFAIL(1,zzerr81,&zzMissSet,&zzMissText,&zzBadTok,&zzBadText,&zzErrk); goto fail;}
  }
  zzEXIT(zztasp1);
  return;
fail:
  zzEXIT(zztasp1);
  zzsyn(zzMissText, zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk, zzBadText);
  zzresynch(setwd25, 0x8);
  }
}

void
#ifdef __USE_PROTOS
v_expression(void)
#else
v_expression()
#endif
{
#line 1824 "./verilog.g"
  zzRULE;
  zzBLOCK(zztasp1);
  zzMake0;
  {
#line 1824 "./verilog.g"
  push_exp_now();
#line 1824 "./verilog.g"
  v_expression2();
#line 1824 "./verilog.g"
  
  if(!zzerrors)
  {
    push_oper(i_oper_make(V_EOF,0));
    push_primary(NULL);
    pop_exp_now(); 
    zzaRet.prim = shred_expression(); 
  }
  else
  {
    zzaRet.prim = NULL;
  }
  zzEXIT(zztasp1);
  return;
fail:
  zzEXIT(zztasp1);
  zzsyn(zzMissText, zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk, zzBadText);
  zzresynch(setwd25, 0x10);
  }
}

void
#ifdef __USE_PROTOS
v_expression2(void)
#else
v_expression2()
#endif
{
#line 1839 "./verilog.g"
  zzRULE;
  zzBLOCK(zztasp1);
  zzMake0;
  {
#line 1839 "./verilog.g"
  {
    zzBLOCK(zztasp2);
    zzMake0;
    {
    if ( (setwd25[LA(1)]&0x20) ) {
#line 1839 "./verilog.g"
      v_primary();
#line 1839 "./verilog.g"
      push_primary(zzaArg(zztasp2,1 ).prim);
    }
    else {
      if ( (setwd25[LA(1)]&0x40) ) {
#line 1840 "./verilog.g"
        v_unary_operator();
#line 1840 "./verilog.g"
        v_primary();
#line 1841 "./verilog.g"
        push_primary(NULL); push_oper(zzaArg(zztasp2,1 ).oper); push_primary(zzaArg(zztasp2,2 ).prim);
      }
      else {
        if ( (LA(1)==V_STRING)
 ) {
#line 1842 "./verilog.g"
          zzmatch(V_STRING);
#line 1842 "./verilog.g"
          push_primary(i_primary_make(PRIM_NUMBER, i_number_basemake(NV_STRING, zzaArg(zztasp2,1 ).text)));
 zzCONSUME;

        }
        else {zzFAIL(1,zzerr82,&zzMissSet,&zzMissText,&zzBadTok,&zzBadText,&zzErrk); goto fail;}
      }
    }
    zzEXIT(zztasp2);
    }
  }
#line 1843 "./verilog.g"
  {
    zzBLOCK(zztasp2);
    zzMake0;
    {
    if ( (setwd25[LA(1)]&0x80) ) {
#line 1843 "./verilog.g"
      v_binary_operator();
#line 1843 "./verilog.g"
      push_oper(zzaArg(zztasp2,1 ).oper);
#line 1843 "./verilog.g"
      v_expression2();
    }
    else {
      if ( (LA(1)==V_QUEST) ) {
#line 1844 "./verilog.g"
        zzmatch(V_QUEST); zzCONSUME;
#line 1844 "./verilog.g"
        v_expression();
#line 1844 "./verilog.g"
        zzmatch(V_COLON); zzCONSUME;
#line 1844 "./verilog.g"
        v_expression();
#line 1845 "./verilog.g"
        push_oper(i_oper_make(V_QUEST,1));
        push_primary(i_bin_expr_make(zzaArg(zztasp2,2 ).prim,i_oper_make(V_COLON, 1),zzaArg(zztasp2,4 ).prim));
      }
      else {
        if ( (setwd26[LA(1)]&0x1) ) {
        }
        else {zzFAIL(1,zzerr83,&zzMissSet,&zzMissText,&zzBadTok,&zzBadText,&zzErrk); goto fail;}
      }
    }
    zzEXIT(zztasp2);
    }
  }
  zzEXIT(zztasp1);
  return;
fail:
  zzEXIT(zztasp1);
  zzsyn(zzMissText, zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk, zzBadText);
  zzresynch(setwd26, 0x2);
  }
}

void
#ifdef __USE_PROTOS
v_mintypmax_expression(void)
#else
v_mintypmax_expression()
#endif
{
#line 1850 "./verilog.g"
  zzRULE;
  zzBLOCK(zztasp1);
  zzMake0;
  {
#line 1850 "./verilog.g"
  v_expression();
#line 1851 "./verilog.g"
  {
    zzBLOCK(zztasp2);
    zzMake0;
    {
    if ( (LA(1)==V_COLON) ) {
#line 1851 "./verilog.g"
      zzmatch(V_COLON); zzCONSUME;
#line 1851 "./verilog.g"
      v_expression();
#line 1851 "./verilog.g"
      zzmatch(V_COLON); zzCONSUME;
#line 1851 "./verilog.g"
      v_expression();
#line 1852 "./verilog.g"
      zzaRet.prim = i_primary_mintypmax_make(zzaArg(zztasp1,1).prim, zzaArg(zztasp2,2).prim, zzaArg(zztasp2,4).prim);
    }
    else {
      if ( (setwd26[LA(1)]&0x4)
 ) {
#line 1853 "./verilog.g"
        zzaRet.prim = zzaArg(zztasp1,1).prim;
      }
      else {zzFAIL(1,zzerr84,&zzMissSet,&zzMissText,&zzBadTok,&zzBadText,&zzErrk); goto fail;}
    }
    zzEXIT(zztasp2);
    }
  }
  zzEXIT(zztasp1);
  return;
fail:
  zzEXIT(zztasp1);
  zzsyn(zzMissText, zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk, zzBadText);
  zzresynch(setwd26, 0x8);
  }
}

void
#ifdef __USE_PROTOS
v_unary_operator(void)
#else
v_unary_operator()
#endif
{
#line 1856 "./verilog.g"
  zzRULE;
  zzBLOCK(zztasp1);
  zzMake0;
  {
  if ( (LA(1)==V_PLUS) ) {
#line 1856 "./verilog.g"
    zzmatch(V_PLUS);
#line 1856 "./verilog.g"
    zzaRet.oper = i_oper_make(V_PLUS, 11);
 zzCONSUME;

  }
  else {
    if ( (LA(1)==V_MINUS) ) {
#line 1857 "./verilog.g"
      zzmatch(V_MINUS);
#line 1857 "./verilog.g"
      zzaRet.oper = i_oper_make(V_MINUS,11);
 zzCONSUME;

    }
    else {
      if ( (LA(1)==V_BANG) ) {
#line 1858 "./verilog.g"
        zzmatch(V_BANG);
#line 1858 "./verilog.g"
        zzaRet.oper = i_oper_make(V_BANG, 11);
 zzCONSUME;

      }
      else {
        if ( (LA(1)==V_TILDE) ) {
#line 1859 "./verilog.g"
          zzmatch(V_TILDE);
#line 1859 "./verilog.g"
          zzaRet.oper = i_oper_make(V_TILDE,11);
 zzCONSUME;

        }
        else {
          if ( (LA(1)==V_AND)
 ) {
#line 1860 "./verilog.g"
            zzmatch(V_AND);
#line 1860 "./verilog.g"
            zzaRet.oper = i_oper_make(V_AND,  11);
 zzCONSUME;

          }
          else {
            if ( (LA(1)==V_NAND) ) {
#line 1861 "./verilog.g"
              zzmatch(V_NAND);
#line 1861 "./verilog.g"
              zzaRet.oper = i_oper_make(V_NAND, 11);
 zzCONSUME;

            }
            else {
              if ( (LA(1)==V_OR) ) {
#line 1862 "./verilog.g"
                zzmatch(V_OR);
#line 1862 "./verilog.g"
                zzaRet.oper = i_oper_make(V_OR,   11);
 zzCONSUME;

              }
              else {
                if ( (LA(1)==V_NOR) ) {
#line 1863 "./verilog.g"
                  zzmatch(V_NOR);
#line 1863 "./verilog.g"
                  zzaRet.oper = i_oper_make(V_NOR,  11);
 zzCONSUME;

                }
                else {
                  if ( (LA(1)==V_XOR) ) {
#line 1864 "./verilog.g"
                    zzmatch(V_XOR);
#line 1864 "./verilog.g"
                    zzaRet.oper = i_oper_make(V_XOR,  11);
 zzCONSUME;

                  }
                  else {
                    if ( (LA(1)==V_XNOR)
 ) {
#line 1865 "./verilog.g"
                      zzmatch(V_XNOR);
#line 1865 "./verilog.g"
                      zzaRet.oper = i_oper_make(V_XNOR, 11);
 zzCONSUME;

                    }
                    else {
                      if ( (LA(1)==V_XNOR2) ) {
#line 1866 "./verilog.g"
                        zzmatch(V_XNOR2);
#line 1866 "./verilog.g"
                        zzaRet.oper = i_oper_make(V_XNOR, 11);
 zzCONSUME;

                      }
                      else {zzFAIL(1,zzerr85,&zzMissSet,&zzMissText,&zzBadTok,&zzBadText,&zzErrk); goto fail;}
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
  }
  zzEXIT(zztasp1);
  return;
fail:
  zzEXIT(zztasp1);
  zzsyn(zzMissText, zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk, zzBadText);
  zzresynch(setwd26, 0x10);
  }
}

void
#ifdef __USE_PROTOS
v_binary_operator(void)
#else
v_binary_operator()
#endif
{
#line 1869 "./verilog.g"
  zzRULE;
  zzBLOCK(zztasp1);
  zzMake0;
  {
  if ( (LA(1)==V_STAR) ) {
#line 1869 "./verilog.g"
    zzmatch(V_STAR);
#line 1869 "./verilog.g"
    zzaRet.oper = i_oper_make(V_STAR, 10);
 zzCONSUME;

  }
  else {
    if ( (LA(1)==V_SLASH) ) {
#line 1870 "./verilog.g"
      zzmatch(V_SLASH);
#line 1870 "./verilog.g"
      zzaRet.oper = i_oper_make(V_SLASH,10);
 zzCONSUME;

    }
    else {
      if ( (LA(1)==V_MOD) ) {
#line 1871 "./verilog.g"
        zzmatch(V_MOD);
#line 1871 "./verilog.g"
        zzaRet.oper = i_oper_make(V_MOD,  10);
 zzCONSUME;

      }
      else {
        if ( (LA(1)==V_PLUS)
 ) {
#line 1872 "./verilog.g"
          zzmatch(V_PLUS);
#line 1872 "./verilog.g"
          zzaRet.oper = i_oper_make(V_PLUS,  9);
 zzCONSUME;

        }
        else {
          if ( (LA(1)==V_MINUS) ) {
#line 1873 "./verilog.g"
            zzmatch(V_MINUS);
#line 1873 "./verilog.g"
            zzaRet.oper = i_oper_make(V_MINUS, 9);
 zzCONSUME;

          }
          else {
            if ( (LA(1)==V_SHL) ) {
#line 1874 "./verilog.g"
              zzmatch(V_SHL);
#line 1874 "./verilog.g"
              zzaRet.oper = i_oper_make(V_SHL,   8);
 zzCONSUME;

            }
            else {
              if ( (LA(1)==V_SHR) ) {
#line 1875 "./verilog.g"
                zzmatch(V_SHR);
#line 1875 "./verilog.g"
                zzaRet.oper = i_oper_make(V_SHR,   8);
 zzCONSUME;

              }
              else {
                if ( (LA(1)==V_LT) ) {
#line 1876 "./verilog.g"
                  zzmatch(V_LT);
#line 1876 "./verilog.g"
                  zzaRet.oper = i_oper_make(V_LT,    7);
 zzCONSUME;

                }
                else {
                  if ( (LA(1)==V_LEQ)
 ) {
#line 1877 "./verilog.g"
                    zzmatch(V_LEQ);
#line 1877 "./verilog.g"
                    zzaRet.oper = i_oper_make(V_LEQ,   7);
 zzCONSUME;

                  }
                  else {
                    if ( (LA(1)==V_GT) ) {
#line 1878 "./verilog.g"
                      zzmatch(V_GT);
#line 1878 "./verilog.g"
                      zzaRet.oper = i_oper_make(V_GT,    7);
 zzCONSUME;

                    }
                    else {
                      if ( (LA(1)==V_GEQ) ) {
#line 1879 "./verilog.g"
                        zzmatch(V_GEQ);
#line 1879 "./verilog.g"
                        zzaRet.oper = i_oper_make(V_GEQ,   7);
 zzCONSUME;

                      }
                      else {
                        if ( (LA(1)==V_EQ2) ) {
#line 1880 "./verilog.g"
                          zzmatch(V_EQ2);
#line 1880 "./verilog.g"
                          zzaRet.oper = i_oper_make(V_EQ2,   6);
 zzCONSUME;

                        }
                        else {
                          if ( (LA(1)==V_NEQ) ) {
#line 1881 "./verilog.g"
                            zzmatch(V_NEQ);
#line 1881 "./verilog.g"
                            zzaRet.oper = i_oper_make(V_NEQ,   6);
 zzCONSUME;

                          }
                          else {
                            if ( (LA(1)==V_EQ3)
 ) {
#line 1882 "./verilog.g"
                              zzmatch(V_EQ3);
#line 1882 "./verilog.g"
                              zzaRet.oper = i_oper_make(V_EQ3,   6);
 zzCONSUME;

                            }
                            else {
                              if ( (LA(1)==V_NEQ2) ) {
#line 1883 "./verilog.g"
                                zzmatch(V_NEQ2);
#line 1883 "./verilog.g"
                                zzaRet.oper = i_oper_make(V_NEQ2,  6);
 zzCONSUME;

                              }
                              else {
                                if ( (LA(1)==V_AND) ) {
#line 1884 "./verilog.g"
                                  zzmatch(V_AND);
#line 1884 "./verilog.g"
                                  zzaRet.oper = i_oper_make(V_AND,   5);
 zzCONSUME;

                                }
                                else {
                                  if ( (LA(1)==V_NAND) ) {
#line 1885 "./verilog.g"
                                    zzmatch(V_NAND);
#line 1885 "./verilog.g"
                                    zzaRet.oper = i_oper_make(V_NAND,  5);
 zzCONSUME;

                                  }
                                  else {
                                    if ( (LA(1)==V_XOR) ) {
#line 1886 "./verilog.g"
                                      zzmatch(V_XOR);
#line 1886 "./verilog.g"
                                      zzaRet.oper = i_oper_make(V_XOR,   5);
 zzCONSUME;

                                    }
                                    else {
                                      if ( (LA(1)==V_XNOR)
 ) {
#line 1887 "./verilog.g"
                                        zzmatch(V_XNOR);
#line 1887 "./verilog.g"
                                        zzaRet.oper = i_oper_make(V_XNOR,  5);
 zzCONSUME;

                                      }
                                      else {
                                        if ( (LA(1)==V_XNOR2) ) {
#line 1888 "./verilog.g"
                                          zzmatch(V_XNOR2);
#line 1888 "./verilog.g"
                                          zzaRet.oper = i_oper_make(V_XNOR,  5);
 zzCONSUME;

                                        }
                                        else {
                                          if ( (LA(1)==V_OR) ) {
#line 1889 "./verilog.g"
                                            zzmatch(V_OR);
#line 1889 "./verilog.g"
                                            zzaRet.oper = i_oper_make(V_OR,    4);
 zzCONSUME;

                                          }
                                          else {
                                            if ( (LA(1)==V_NOR) ) {
#line 1890 "./verilog.g"
                                              zzmatch(V_NOR);
#line 1890 "./verilog.g"
                                              zzaRet.oper = i_oper_make(V_NOR,   4);
 zzCONSUME;

                                            }
                                            else {
                                              if ( (LA(1)==V_AND2) ) {
#line 1891 "./verilog.g"
                                                zzmatch(V_AND2);
#line 1891 "./verilog.g"
                                                zzaRet.oper = i_oper_make(V_AND2,  3);
 zzCONSUME;

                                              }
                                              else {
                                                if ( (LA(1)==V_OR2)
 ) {
#line 1892 "./verilog.g"
                                                  zzmatch(V_OR2);
#line 1892 "./verilog.g"
                                                  zzaRet.oper = i_oper_make(V_OR2,   2);
 zzCONSUME;

                                                }
                                                else {zzFAIL(1,zzerr86,&zzMissSet,&zzMissText,&zzBadTok,&zzBadText,&zzErrk); goto fail;}
                                              }
                                            }
                                          }
                                        }
                                      }
                                    }
                                  }
                                }
                              }
                            }
                          }
                        }
                      }
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
  }
  zzEXIT(zztasp1);
  return;
fail:
  zzEXIT(zztasp1);
  zzsyn(zzMissText, zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk, zzBadText);
  zzresynch(setwd26, 0x20);
  }
}

void
#ifdef __USE_PROTOS
v_primary(void)
#else
v_primary()
#endif
{
#line 1895 "./verilog.g"
  zzRULE;
  zzBLOCK(zztasp1);
  zzMake0;
  {
  if ( (setwd26[LA(1)]&0x40) ) {
#line 1895 "./verilog.g"
    v_number();
#line 1895 "./verilog.g"
    zzaRet.prim = i_primary_make(PRIM_NUMBER,zzaArg(zztasp1,1 ).num);
  }
  else {
    if ( (setwd26[LA(1)]&0x80) && (setwd27[LA(2)]&0x1) ) {
#line 1896 "./verilog.g"
      v_identifier();
#line 1897 "./verilog.g"
      {
        zzBLOCK(zztasp2);
        zzMake0;
        {
        if ( (LA(1)==V_LBRACK) ) {
#line 1897 "./verilog.g"
          zzmatch(V_LBRACK); zzCONSUME;
#line 1897 "./verilog.g"
          v_expression();
#line 1898 "./verilog.g"
          {
            zzBLOCK(zztasp3);
            zzMake0;
            {
            if ( (LA(1)==V_RBRACK) ) {
#line 1898 "./verilog.g"
              zzmatch(V_RBRACK);
#line 1899 "./verilog.g"
              zzaRet.prim= i_primary_symbit_make(zzaArg(zztasp1,1).symbol,zzaArg(zztasp2,2).prim);
 zzCONSUME;

            }
            else {
              if ( (LA(1)==V_COLON)
 ) {
#line 1900 "./verilog.g"
                zzmatch(V_COLON); zzCONSUME;
#line 1900 "./verilog.g"
                v_expression();
#line 1900 "./verilog.g"
                zzmatch(V_RBRACK);
#line 1901 "./verilog.g"
                zzaRet.prim= i_primary_symrange_make(zzaArg(zztasp1,1).symbol,zzaArg(zztasp2,2).prim,zzaArg(zztasp3,2).prim);
 zzCONSUME;

              }
              else {zzFAIL(1,zzerr87,&zzMissSet,&zzMissText,&zzBadTok,&zzBadText,&zzErrk); goto fail;}
            }
            zzEXIT(zztasp3);
            }
          }
        }
        else {
          if ( (setwd27[LA(1)]&0x2) ) {
#line 1902 "./verilog.g"
            zzaRet.prim= i_primary_make(PRIM_SYMBOL,zzaArg(zztasp1,1).symbol);
          }
          else {zzFAIL(1,zzerr88,&zzMissSet,&zzMissText,&zzBadTok,&zzBadText,&zzErrk); goto fail;}
        }
        zzEXIT(zztasp2);
        }
      }
    }
    else {
      if ( (LA(1)==V_LBRACE) ) {
#line 1903 "./verilog.g"
        v_multiple_concatenation();
#line 1903 "./verilog.g"
        zzaRet.prim = zzaArg(zztasp1,1 ).prim;
      }
      else {
        if ( (setwd27[LA(1)]&0x4) && (LA(2)==V_LP) ) {
#line 1904 "./verilog.g"
          v_function_call();
#line 1904 "./verilog.g"
          zzaRet.prim = zzaArg(zztasp1,1 ).prim;
        }
        else {
          if ( (LA(1)==V_LP) ) {
#line 1905 "./verilog.g"
            zzmatch(V_LP); zzCONSUME;
#line 1905 "./verilog.g"
            v_mintypmax_expression();
#line 1905 "./verilog.g"
            zzmatch(V_RP);
#line 1905 "./verilog.g"
            zzaRet.prim = zzaArg(zztasp1,2 ).prim;
 zzCONSUME;

          }
          else {zzFAIL(2,zzerr89,zzerr90,&zzMissSet,&zzMissText,&zzBadTok,&zzBadText,&zzErrk); goto fail;}
        }
      }
    }
  }
  zzEXIT(zztasp1);
  return;
fail:
  zzEXIT(zztasp1);
  zzsyn(zzMissText, zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk, zzBadText);
  zzresynch(setwd27, 0x8);
  }
}

void
#ifdef __USE_PROTOS
v_number(void)
#else
v_number()
#endif
{
#line 1909 "./verilog.g"
  zzRULE;
  zzBLOCK(zztasp1);
  zzMake0;
  {
  if ( (LA(1)==V_DECIMAL_NUMBER)
 ) {
#line 1909 "./verilog.g"
    zzmatch(V_DECIMAL_NUMBER);
#line 1909 "./verilog.g"
    zzaRet.num = i_number_make(zzaArg(zztasp1,1).ival);
 zzCONSUME;

  }
  else {
    if ( (LA(1)==V_HBASE) ) {
#line 1910 "./verilog.g"
      zzmatch(V_HBASE);
#line 1910 "./verilog.g"
      zzaRet.num = i_number_basemake(NV_HBASE, zzaArg(zztasp1,1 ).text);
 zzCONSUME;

    }
    else {
      if ( (LA(1)==V_DBASE) ) {
#line 1911 "./verilog.g"
        zzmatch(V_DBASE);
#line 1911 "./verilog.g"
        zzaRet.num = i_number_basemake(NV_DBASE, zzaArg(zztasp1,1 ).text);
 zzCONSUME;

      }
      else {
        if ( (LA(1)==V_BBASE) ) {
#line 1912 "./verilog.g"
          zzmatch(V_BBASE);
#line 1912 "./verilog.g"
          zzaRet.num = i_number_basemake(NV_BBASE, zzaArg(zztasp1,1 ).text);
 zzCONSUME;

        }
        else {
          if ( (LA(1)==V_OBASE) ) {
#line 1913 "./verilog.g"
            zzmatch(V_OBASE);
#line 1913 "./verilog.g"
            zzaRet.num = i_number_basemake(NV_OBASE, zzaArg(zztasp1,1 ).text);
 zzCONSUME;

          }
          else {
            if ( (LA(1)==V_FLOAT1)
 ) {
#line 1914 "./verilog.g"
              zzmatch(V_FLOAT1);
#line 1914 "./verilog.g"
              zzaRet.num = i_number_fmake(zzaArg(zztasp1,1 ).rval);
 zzCONSUME;

            }
            else {
              if ( (LA(1)==V_FLOAT2) ) {
#line 1915 "./verilog.g"
                zzmatch(V_FLOAT2);
#line 1915 "./verilog.g"
                zzaRet.num = i_number_fmake(zzaArg(zztasp1,1 ).rval);
 zzCONSUME;

              }
              else {zzFAIL(1,zzerr91,&zzMissSet,&zzMissText,&zzBadTok,&zzBadText,&zzErrk); goto fail;}
            }
          }
        }
      }
    }
  }
  zzEXIT(zztasp1);
  return;
fail:
  zzEXIT(zztasp1);
  zzsyn(zzMissText, zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk, zzBadText);
  zzresynch(setwd27, 0x10);
  }
}

void
#ifdef __USE_PROTOS
v_concatenation(void)
#else
v_concatenation()
#endif
{
#line 1918 "./verilog.g"
  zzRULE;
  zzBLOCK(zztasp1);
  zzMake0;
  {
#line 1919 "./verilog.g"
  zzmatch(V_LBRACE); zzCONSUME;
#line 1919 "./verilog.g"
  v_explist();
#line 1919 "./verilog.g"
  zzmatch(V_RBRACE);
#line 1920 "./verilog.g"
  zzaRet.prim = i_primary_concat_make(NULL,zzaArg(zztasp1,2 ).explist);
 zzCONSUME;

  zzEXIT(zztasp1);
  return;
fail:
  zzEXIT(zztasp1);
  zzsyn(zzMissText, zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk, zzBadText);
  zzresynch(setwd27, 0x20);
  }
}

void
#ifdef __USE_PROTOS
v_multiple_concatenation(void)
#else
v_multiple_concatenation()
#endif
{
#line 1923 "./verilog.g"
  zzRULE;
  zzBLOCK(zztasp1);
  zzMake0;
  {
#line 1924 "./verilog.g"
  struct i_explist *lroot=NULL, *lcurrent=NULL;
#line 1925 "./verilog.g"
  zzmatch(V_LBRACE); zzCONSUME;
#line 1925 "./verilog.g"
  v_expression();
#line 1926 "./verilog.g"
  lroot=lcurrent=(struct i_explist *)calloc(1,sizeof(struct i_explist));
  lcurrent->item=zzaArg(zztasp1,2 ).prim;
#line 1935 "./verilog.g"
  {
    zzBLOCK(zztasp2);
    zzMake0;
    {
    if ( (setwd27[LA(1)]&0x40) ) {
#line 1935 "./verilog.g"
      {
        zzBLOCK(zztasp3);
        zzMake0;
        {
        while ( (LA(1)==V_COMMA) ) {
#line 1930 "./verilog.g"
          zzmatch(V_COMMA); zzCONSUME;
#line 1930 "./verilog.g"
          v_expression();
#line 1931 "./verilog.g"
          lcurrent->next=(struct i_explist *)calloc(1,sizeof(struct i_explist));
          lcurrent=lcurrent->next;
          lcurrent->item=zzaArg(zztasp3,2 ).prim;
          zzLOOP(zztasp3);
        }
        zzEXIT(zztasp3);
        }
      }
#line 1936 "./verilog.g"
      zzaRet.prim = i_primary_concat_make(NULL,i_explist_make(lroot));
    }
    else {
      if ( (LA(1)==V_LBRACE) ) {
#line 1938 "./verilog.g"
        zzmatch(V_LBRACE); zzCONSUME;
#line 1938 "./verilog.g"
        v_explist();
#line 1938 "./verilog.g"
        zzmatch(V_RBRACE);
#line 1939 "./verilog.g"
        zzaRet.prim = i_primary_concat_make(lroot->item,zzaArg(zztasp2,2 ).explist); 
        free(lroot);
 zzCONSUME;

      }
      else {zzFAIL(1,zzerr92,&zzMissSet,&zzMissText,&zzBadTok,&zzBadText,&zzErrk); goto fail;}
    }
    zzEXIT(zztasp2);
    }
  }
#line 1943 "./verilog.g"
  zzmatch(V_RBRACE); zzCONSUME;
  zzEXIT(zztasp1);
  return;
fail:
  zzEXIT(zztasp1);
  zzsyn(zzMissText, zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk, zzBadText);
  zzresynch(setwd27, 0x80);
  }
}

void
#ifdef __USE_PROTOS
v_function_call(void)
#else
v_function_call()
#endif
{
#line 1946 "./verilog.g"
  zzRULE;
  zzBLOCK(zztasp1);
  zzMake0;
  {
#line 1946 "./verilog.g"
  v_name_of_function();
#line 1946 "./verilog.g"
  zzmatch(V_LP); zzCONSUME;
#line 1946 "./verilog.g"
  v_explist();
#line 1946 "./verilog.g"
  zzmatch(V_RP);
#line 1947 "./verilog.g"
  zzaRet.prim=i_primary_funcall_make(zzaArg(zztasp1,1 ).symbol,zzaArg(zztasp1,3 ).explist);
 zzCONSUME;

  zzEXIT(zztasp1);
  return;
fail:
  zzEXIT(zztasp1);
  zzsyn(zzMissText, zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk, zzBadText);
  zzresynch(setwd28, 0x1);
  }
}

void
#ifdef __USE_PROTOS
v_name_of_function(void)
#else
v_name_of_function()
#endif
{
#line 1950 "./verilog.g"
  zzRULE;
  zzBLOCK(zztasp1);
  zzMake0;
  {
#line 1950 "./verilog.g"
  v_identifier_nodot();
#line 1950 "./verilog.g"
  zzaRet.symbol = zzaArg(zztasp1,1 ).symbol;
  zzEXIT(zztasp1);
  return;
fail:
  zzEXIT(zztasp1);
  zzsyn(zzMissText, zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk, zzBadText);
  zzresynch(setwd28, 0x2);
  }
}

void
#ifdef __USE_PROTOS
v_explist(void)
#else
v_explist()
#endif
{
#line 1953 "./verilog.g"
  zzRULE;
  zzBLOCK(zztasp1);
  zzMake0;
  {
#line 1953 "./verilog.g"
  struct i_explist *lroot=NULL, *lcurrent=NULL;
#line 1954 "./verilog.g"
  v_fn_expression();
#line 1955 "./verilog.g"
  lroot=lcurrent=(struct i_explist *)calloc(1,sizeof(struct i_explist));
  lcurrent->item=zzaArg(zztasp1,1 ).prim;
#line 1963 "./verilog.g"
  {
    zzBLOCK(zztasp2);
    zzMake0;
    {
    while ( (LA(1)==V_COMMA)
 ) {
#line 1958 "./verilog.g"
      zzmatch(V_COMMA); zzCONSUME;
#line 1958 "./verilog.g"
      v_fn_expression();
#line 1959 "./verilog.g"
      lcurrent->next=(struct i_explist *)calloc(1,sizeof(struct i_explist));
      lcurrent=lcurrent->next;
      lcurrent->item=zzaArg(zztasp2,2 ).prim;
      zzLOOP(zztasp2);
    }
    zzEXIT(zztasp2);
    }
  }
#line 1964 "./verilog.g"
  zzaRet.explist=i_explist_make(lroot);
  zzEXIT(zztasp1);
  return;
fail:
  zzEXIT(zztasp1);
  zzsyn(zzMissText, zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk, zzBadText);
  zzresynch(setwd28, 0x4);
  }
}

void
#ifdef __USE_PROTOS
v_fn_expression(void)
#else
v_fn_expression()
#endif
{
#line 1967 "./verilog.g"
  zzRULE;
  zzBLOCK(zztasp1);
  zzMake0;
  {
  if ( (setwd28[LA(1)]&0x8) ) {
#line 1968 "./verilog.g"
    v_expression();
#line 1968 "./verilog.g"
    zzaRet.prim = zzaArg(zztasp1,1 ).prim;
  }
  else {
    if ( (setwd28[LA(1)]&0x10) ) {
#line 1969 "./verilog.g"
      zzaRet.prim = NULL;
    }
    else {zzFAIL(1,zzerr93,&zzMissSet,&zzMissText,&zzBadTok,&zzBadText,&zzErrk); goto fail;}
  }
  zzEXIT(zztasp1);
  return;
fail:
  zzEXIT(zztasp1);
  zzsyn(zzMissText, zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk, zzBadText);
  zzresynch(setwd28, 0x20);
  }
}

void
#ifdef __USE_PROTOS
v_identifier(void)
#else
v_identifier()
#endif
{
#line 1976 "./verilog.g"
  zzRULE;
  zzBLOCK(zztasp1);
  zzMake0;
  {
  if ( (setwd28[LA(1)]&0x40) ) {
#line 1976 "./verilog.g"
    v_identifier_nodot();
#line 1976 "./verilog.g"
    zzaRet.symbol = zzaArg(zztasp1,1 ).symbol;
  }
  else {
    if ( (LA(1)==V_IDENDOT) ) {
#line 1977 "./verilog.g"
      zzmatch(V_IDENDOT);
#line 1977 "./verilog.g"
      zzaRet.symbol = zzaArg(zztasp1,1 ).symbol;
 zzCONSUME;

    }
    else {zzFAIL(1,zzerr94,&zzMissSet,&zzMissText,&zzBadTok,&zzBadText,&zzErrk); goto fail;}
  }
  zzEXIT(zztasp1);
  return;
fail:
  zzEXIT(zztasp1);
  zzsyn(zzMissText, zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk, zzBadText);
  zzresynch(setwd28, 0x80);
  }
}

void
#ifdef __USE_PROTOS
v_identifier_nodot(void)
#else
v_identifier_nodot()
#endif
{
#line 1980 "./verilog.g"
  zzRULE;
  zzBLOCK(zztasp1);
  zzMake0;
  {
  if ( (LA(1)==V_IDENTIFIER)
 ) {
#line 1981 "./verilog.g"
    zzmatch(V_IDENTIFIER);
#line 1981 "./verilog.g"
    zzaRet.symbol = zzaArg(zztasp1,1 ).symbol;
 zzCONSUME;

  }
  else {
    if ( (LA(1)==V_IDENTIFIER2) ) {
#line 1982 "./verilog.g"
      zzmatch(V_IDENTIFIER2);
#line 1982 "./verilog.g"
      zzaRet.symbol = zzaArg(zztasp1,1 ).symbol;
 zzCONSUME;

    }
    else {
      if ( (LA(1)==V_FUNCTION_NAME) ) {
#line 1983 "./verilog.g"
        zzmatch(V_FUNCTION_NAME);
#line 1983 "./verilog.g"
        zzaRet.symbol = zzaArg(zztasp1,1 ).symbol;
 zzCONSUME;

      }
      else {zzFAIL(1,zzerr95,&zzMissSet,&zzMissText,&zzBadTok,&zzBadText,&zzErrk); goto fail;}
    }
  }
  zzEXIT(zztasp1);
  return;
fail:
  zzEXIT(zztasp1);
  zzsyn(zzMissText, zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk, zzBadText);
  zzresynch(setwd29, 0x1);
  }
}

void
#ifdef __USE_PROTOS
v_delay(void)
#else
v_delay()
#endif
{
#line 1986 "./verilog.g"
  zzRULE;
  zzBLOCK(zztasp1);
  zzMake0;
  {
#line 1986 "./verilog.g"
  zzmatch(V_POUND); zzCONSUME;
#line 1986 "./verilog.g"
  {
    zzBLOCK(zztasp2);
    zzMake0;
    {
    if ( (setwd29[LA(1)]&0x2) ) {
#line 1986 "./verilog.g"
      v_number();
    }
    else {
      if ( (setwd29[LA(1)]&0x4) ) {
#line 1987 "./verilog.g"
        v_identifier();
      }
      else {
        if ( (LA(1)==V_LP)
 ) {
#line 1988 "./verilog.g"
          zzmatch(V_LP); zzCONSUME;
#line 1988 "./verilog.g"
          v_mintypmax_expression();
#line 1989 "./verilog.g"
          {
            zzBLOCK(zztasp3);
            zzMake0;
            {
            while ( (LA(1)==V_COMMA) ) {
#line 1989 "./verilog.g"
              zzmatch(V_COMMA); zzCONSUME;
#line 1989 "./verilog.g"
              v_mintypmax_expression();
              zzLOOP(zztasp3);
            }
            zzEXIT(zztasp3);
            }
          }
#line 1989 "./verilog.g"
          zzmatch(V_RP); zzCONSUME;
        }
        else {zzFAIL(1,zzerr96,&zzMissSet,&zzMissText,&zzBadTok,&zzBadText,&zzErrk); goto fail;}
      }
    }
    zzEXIT(zztasp2);
    }
  }
  zzEXIT(zztasp1);
  return;
fail:
  zzEXIT(zztasp1);
  zzsyn(zzMissText, zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk, zzBadText);
  zzresynch(setwd29, 0x8);
  }
}

void
#ifdef __USE_PROTOS
v_delay_control(void)
#else
v_delay_control()
#endif
{
#line 1992 "./verilog.g"
  zzRULE;
  zzBLOCK(zztasp1);
  zzMake0;
  {
#line 1992 "./verilog.g"
  zzmatch(V_POUND); zzCONSUME;
#line 1992 "./verilog.g"
  {
    zzBLOCK(zztasp2);
    zzMake0;
    {
    if ( (setwd29[LA(1)]&0x10) ) {
#line 1992 "./verilog.g"
      v_number();
    }
    else {
      if ( (setwd29[LA(1)]&0x20) ) {
#line 1993 "./verilog.g"
        v_identifier();
      }
      else {
        if ( (LA(1)==V_LP) ) {
#line 1994 "./verilog.g"
          zzmatch(V_LP); zzCONSUME;
#line 1994 "./verilog.g"
          v_mintypmax_expression();
#line 1994 "./verilog.g"
          zzmatch(V_RP); zzCONSUME;
        }
        else {zzFAIL(1,zzerr97,&zzMissSet,&zzMissText,&zzBadTok,&zzBadText,&zzErrk); goto fail;}
      }
    }
    zzEXIT(zztasp2);
    }
  }
  zzEXIT(zztasp1);
  return;
fail:
  zzEXIT(zztasp1);
  zzsyn(zzMissText, zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk, zzBadText);
  zzresynch(setwd29, 0x40);
  }
}

void
#ifdef __USE_PROTOS
v_event_control(void)
#else
v_event_control()
#endif
{
#line 1997 "./verilog.g"
  zzRULE;
  zzBLOCK(zztasp1);
  zzMake0;
  {
#line 1997 "./verilog.g"
  zzmatch(V_AT); zzCONSUME;
#line 1997 "./verilog.g"
  {
    zzBLOCK(zztasp2);
    zzMake0;
    {
    if ( (setwd29[LA(1)]&0x80)
 ) {
#line 1997 "./verilog.g"
      v_identifier();
    }
    else {
      if ( (LA(1)==V_LP) ) {
#line 1998 "./verilog.g"
        zzmatch(V_LP); zzCONSUME;
#line 1998 "./verilog.g"
        v_event_expression();
#line 1998 "./verilog.g"
        zzmatch(V_RP); zzCONSUME;
      }
      else {zzFAIL(1,zzerr98,&zzMissSet,&zzMissText,&zzBadTok,&zzBadText,&zzErrk); goto fail;}
    }
    zzEXIT(zztasp2);
    }
  }
  zzEXIT(zztasp1);
  return;
fail:
  zzEXIT(zztasp1);
  zzsyn(zzMissText, zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk, zzBadText);
  zzresynch(setwd30, 0x1);
  }
}

void
#ifdef __USE_PROTOS
v_event_expression(void)
#else
v_event_expression()
#endif
{
#line 2001 "./verilog.g"
  zzRULE;
  zzBLOCK(zztasp1);
  zzMake0;
  {
#line 2002 "./verilog.g"
  v_event_expression2();
#line 2002 "./verilog.g"
  {
    zzBLOCK(zztasp2);
    zzMake0;
    {
    while ( (LA(1)==V_ORLIT) ) {
#line 2002 "./verilog.g"
      zzmatch(V_ORLIT); zzCONSUME;
#line 2002 "./verilog.g"
      v_event_expression2();
      zzLOOP(zztasp2);
    }
    zzEXIT(zztasp2);
    }
  }
  zzEXIT(zztasp1);
  return;
fail:
  zzEXIT(zztasp1);
  zzsyn(zzMissText, zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk, zzBadText);
  zzresynch(setwd30, 0x2);
  }
}

void
#ifdef __USE_PROTOS
v_event_expression2(void)
#else
v_event_expression2()
#endif
{
#line 2005 "./verilog.g"
  zzRULE;
  zzBLOCK(zztasp1);
  zzMake0;
  {
#line 2005 "./verilog.g"
  {
    zzBLOCK(zztasp2);
    zzMake0;
    {
    if ( (setwd30[LA(1)]&0x4) ) {
#line 2005 "./verilog.g"
      v_expression();
    }
    else {
      if ( (LA(1)==V_POSEDGE) ) {
#line 2006 "./verilog.g"
        zzmatch(V_POSEDGE); zzCONSUME;
#line 2006 "./verilog.g"
        v_expression();
      }
      else {
        if ( (LA(1)==V_NEGEDGE)
 ) {
#line 2007 "./verilog.g"
          zzmatch(V_NEGEDGE); zzCONSUME;
#line 2007 "./verilog.g"
          v_expression();
        }
        else {zzFAIL(1,zzerr99,&zzMissSet,&zzMissText,&zzBadTok,&zzBadText,&zzErrk); goto fail;}
      }
    }
    zzEXIT(zztasp2);
    }
  }
  zzEXIT(zztasp1);
  return;
fail:
  zzEXIT(zztasp1);
  zzsyn(zzMissText, zzBadTok, (ANTLRChar *)"", zzMissSet, zzMissTok, zzErrk, zzBadText);
  zzresynch(setwd30, 0x8);
  }
}
