#ifndef VLEX_DEFINES_H
#define VLEX_DEFINES_H

#define V_IGNORE (1)
#define V_ID (2)
#define V_PORT (3)
#define V_MODULE (4)
#define V_ENDMODULE (5)
#define V_STRING (6)
#define V_WS (7)
#define V_MACRO (8)
#define V_CMT (9)
#define V_NUMBER (10)
#define V_FUNC (11)
#define V_PREPROC (12)
#define V_PREPROC_WS (13)
#define V_KW (14)

extern int my_yylineno;
extern char *v_preproc_name;
int yylex(void);
extern char *yytext;
extern int yyleng;

const char *is_builtin_define (register const char *str, register unsigned int len);

#endif

/*
 * $Id$
 * $Log$
 * Revision 1.2  2007/04/20 02:08:10  gtkwave
 * initial release
 *
 */

