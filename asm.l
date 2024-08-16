
%{

#include "asm_common.h"
#include "asm.tab.h"

#include <stdio.h>

#define YY_USER_ACTION                            \
    yylloc->first_line = yylloc->last_line;       \
    yylloc->first_column = yylloc->last_column;   \
    if (yylloc->last_line == yylineno)            \
        yylloc->last_column += yyleng;            \
    else                                          \
        yylloc->last_line = yylineno,             \
        yylloc->last_column = yytext              \
            + yyleng - strrchr(yytext, '\n');

%}

%option noinput nounput noyywrap 8bit nodefault
%option yylineno
%option reentrant bison-bridge bison-locations

%%

(?i:getc)                   { return I_GETC; }
(?i:getn)                   { return I_GETN; }
(?i:putc)                   { return I_PUTC; }
(?i:putn)                   { return I_PUTN; }

(?i:psh)                    { return I_PSH; }
(?i:push)                   { return I_PSH; }
(?i:dup)                    { return I_DUP; }
(?i:xchg)                   { return I_XCHG; }
(?i:swp)                    { return I_XCHG; }
(?i:swap)                   { return I_XCHG; }
(?i:drop)                   { return I_DROP; }
(?i:dsc)                    { return I_DROP; }
(?i:copy)                   { return I_COPY; }
(?i:take)                   { return I_COPY; }
(?i:pull)                   { return I_COPY; }
(?i:slide)                  { return I_SLIDE; }

(?i:add)                    { return I_ADD; }
(?i:sub)                    { return I_SUB; }
(?i:mul)                    { return I_MUL; }
(?i:div)                    { return I_DIV; }
(?i:mod)                    { return I_MOD; }

(?i:sto)                    { return I_STO; }
(?i:rcl)                    { return I_RCL; }

(?i:rep)                    { return I_REP; }

@[A-Za-z_][A-Za-z0-9_]*     { yylval->string = strdup(yytext); return G_LBL; }
%[A-Za-z_][A-Za-z0-9_]*     { yylval->string = strdup(yytext); return G_REF; }

(?i:call)                   { return I_CALL; }
(?i:gosub)                  { return I_CALL; }
(?i:jsr)                    { return I_CALL; }
(?i:j)                      { return I_JMP; }
(?i:b)                      { return I_JMP; }
(?i:jmp)                    { return I_JMP; }
(?i:jz)                     { return I_JZ; }
(?i:bz)                     { return I_JZ; }
(?i:jltz)                   { return I_JLTZ; }
(?i:bltz)                   { return I_JLTZ; }
(?i:ret)                    { return I_RET; }
(?i:end)                    { return I_END; }

","                         { return COMMA; }
";"(.*)\n                   { ; }
\'(\\.|[^\\\'\n])\'         { yylval->string = strdup(yytext); return CHAR; }
\"(\\.|[^\\\"\n])*\"        { yylval->string = strdup(yytext); return STRING; }
-?[01]+[bB]                 { yylval->string = strdup(yytext); return NUMBER; }
-?[0-9][0-9A-Fa-f]*[hH]     { yylval->string = strdup(yytext); return NUMBER; }
-?[0-9]+                    { yylval->string = strdup(yytext); return NUMBER; }
\n+                         { return LF; }
"/"+                        { return LF; }
[\t\r\f ]                   { ; }
.                           { fprintf(stderr, "wsi: unrecognized token at line %d: `%c'", yylineno, *yytext); }

%%

void asm_file(FILE * f, int optlevel) {
    yyscan_t yyscanner;
    
    yylex_init(&yyscanner);
    yyset_in(f, yyscanner);
    yyparse(optlevel, yyscanner);
    yylex_destroy(yyscanner);
}
