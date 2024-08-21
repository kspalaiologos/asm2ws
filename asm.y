
%define api.pure full
%define api.token.raw
%define parse.error verbose
%locations
%parse-param { int optlevel }
%param { yyscan_t scanner }

%code top {
    #include <stdio.h>
    #include <string.h>
    #include <errno.h>
    #include "asm_common.h"

    static struct immediate_t imm_none() { return (struct immediate_t) { .value = 0, IMM_NONE }; }
    static struct immediate_t imm_val(int32_t x) { return (struct immediate_t) { .value = x, IMM_VALUE }; }
    static struct immediate_t imm_lbl(char * n) { return (struct immediate_t) { .label = n, IMM_LABEL }; }
    
    #define node(type, param) (struct node_t) { type, param, imm_none(), imm_none(), yyloc.first_line, yyloc.first_column }
    #define node2(type, param1, param2) (struct node_t) { type, param1, param2, imm_none(), yyloc.first_line, yyloc.first_column }

    vector(struct node_t) insn = NULL;
}

%code requires {
    typedef void * yyscan_t;
}

%union {
    char * string;
    struct node_t ins;
    struct immediate_t imm;
}

%code {
    int yylex(YYSTYPE * yylvalp, YYLTYPE * yyllocp, yyscan_t scanner);
    void yyerror(YYLTYPE * yyllocp, int unused_1, yyscan_t unused, const char * msg);
}

%token END 0 "end of file"

%type<ins> Construct
%type<imm> NumericalConstant Label
%token I_GETC I_GETN I_PUTC I_PUTN
%token I_PSH I_DUP I_XCHG I_DROP I_COPY I_SLIDE
%token I_ADD I_SUB I_MUL I_DIV I_MOD
%token I_STO I_RCL
%token I_CALL I_JMP I_JZ I_JLTZ I_RET I_END
%token I_REP COMMA SLASH LF

%token<string> CHAR STRING NUMBER G_LBL G_REF

%start Start
%%
Start
: Lines Line { asm_gen(stdout, insn, optlevel); }
;

Lines
: Lines Line LF
| %empty
;

Line
: Instructions LastInstruction
| %empty
;

Instructions
: Instructions G_LBL OptionalSLASH { vector_push_back(insn, node(LBL, imm_lbl($2))); }
| Instructions Construct SLASH { vector_push_back(insn, $2); }
| %empty
;

LastInstruction
: G_LBL { vector_push_back(insn, node(LBL, imm_lbl($1))); }
| Construct { vector_push_back(insn, $1); }
;

OptionalSLASH
: SLASH
| %empty
;

Construct
: I_GETC NumericalConstant { $$ = node(GETC, $2); }
| I_PUTC NumericalConstant { $$ = node(PUTC, $2); }
| I_GETN NumericalConstant { $$ = node(GETN, $2); }
| I_PUTN NumericalConstant { $$ = node(PUTN, $2); }
| I_PSH NumericalConstant { $$ = node(PSH, $2); }
| I_REP I_DUP NumericalConstant { $$ = node(DUP, $3); }
| I_REP I_DROP NumericalConstant { $$ = node(DROP, $3); }
| I_REP I_ADD NumericalConstant { $$ = node2(ADD, $3, imm_val(1)); }
| I_REP I_PUTC NumericalConstant { $$ = node2(PUTC, $3, imm_val(1)); }
| I_REP I_PUTN NumericalConstant { $$ = node2(PUTN, $3, imm_val(1)); }
| I_REP I_SUB NumericalConstant { $$ = node2(SUB, $3, imm_val(1)); }
| I_REP I_MUL NumericalConstant { $$ = node2(MUL, $3, imm_val(1)); }
| I_REP I_DIV NumericalConstant { $$ = node2(DIV, $3, imm_val(1)); }
| I_REP I_MOD NumericalConstant { $$ = node2(MOD, $3, imm_val(1)); }
| I_ADD NumericalConstant { $$ = node(ADD, $2); }
| I_SUB NumericalConstant { $$ = node(SUB, $2); }
| I_MUL NumericalConstant { $$ = node(MUL, $2); }
| I_DIV NumericalConstant { $$ = node(DIV, $2); }
| I_MOD NumericalConstant { $$ = node(MOD, $2); }
| I_STO NumericalConstant COMMA NumericalConstant { $$ = node2(STO, $2, $4); }
| I_STO NumericalConstant { $$ = node(STO, $2); }
| I_STO { $$ = node(STO, imm_none()); }
| I_RCL NumericalConstant { $$ = node(RCL, $2); }
| I_RCL { $$ = node(RCL, imm_none()); }
| I_COPY NumericalConstant { $$ = node(COPY, $2); }
| I_SLIDE NumericalConstant { $$ = node(SLIDE, $2); }
| I_CALL Label { $$ = node(CALL, $2); }
| I_JMP Label { $$ = node(JMP, $2); }
| I_JZ Label { $$ = node(BZ, $2); }
| I_JLTZ Label { $$ = node(BLTZ, $2); }
| I_RET { $$ = node(RET, imm_none()); }
| I_GETC { $$ = node(GETC, imm_none()); }
| I_PUTC { $$ = node(PUTC, imm_none()); }
| I_GETN { $$ = node(GETN, imm_none()); }
| I_PUTN { $$ = node(PUTN, imm_none()); }
| I_PSH { $$ = node(PSH, imm_none()); }
| I_DUP { $$ = node(DUP, imm_none()); }
| I_XCHG { $$ = node(XCHG, imm_none()); }
| I_DROP { $$ = node(DROP, imm_none()); }
| I_ADD { $$ = node(ADD, imm_none()); }
| I_SUB { $$ = node(SUB, imm_none()); }
| I_MUL { $$ = node(MUL, imm_none()); }
| I_DIV { $$ = node(DIV, imm_none()); }
| I_MOD { $$ = node(MOD, imm_none()); }
| I_END { $$ = node(STOP, imm_none()); }
| NumericalConstant { $$ = node(PSH, $1); }
;

NumericalConstant
: CHAR {
    char c = $1[1];
    if(c == '\\') {
        c = $1[2];
        switch(c) {
            case 'a': c = '\a'; break;
            case 'b': c = '\b'; break;
            case 'f': c = '\f'; break;
            case 'n': c = '\n'; break;
            case 'r': c = '\r'; break;
            case 't': c = '\t'; break;
            case 'v': c = '\v'; break;
        }
    }
    $$ = imm_val(c);
    free($1);
}
| NUMBER {
    long ret;
    int base = 10;

    switch($1[strlen($1) - 1]) {
        case 'h': case 'H': base = 16; break;
        case 'o': case 'O': base =  8; break;
        case 'b': case 'B': base =  2; break;
    }

    errno = 0;
    ret = strtol($1, NULL, base);
    if(errno) {
        fprintf(stderr, "wsi: parsing integer %s: %s\n", $1, strerror(errno));
        exit(1);
    }
    if(ret < INT32_MIN || ret > INT32_MAX) {
        fprintf(stderr, "wsi: integer %s out of range\n", $1);
        exit(1);
    }
    
    $$ = imm_val((int32_t) ret);
    free($1);
}
;

Label
: G_REF { $$ = imm_lbl($1); }
;

%%

void yyerror(YYLTYPE * yyllocp, int unused_1, yyscan_t unused, const char * msg) {
    (void) unused;
    (void) unused_1;
    fprintf(stderr, "wsi: %d:%d: %s\n", yyllocp->first_line, yyllocp->first_column, msg);
    exit(1);
}
