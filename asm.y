
%define api.pure full
%define api.token.raw
%define parse.error verbose
%locations
%param { yyscan_t scanner }

%code top {
    #include <stdio.h>
    #include <string.h>
    #include "asm_common.h"

    static struct immediate_t imm_none() { return (struct immediate_t) { .value = 0, IMM_NONE }; }
    static struct immediate_t imm_val(int32_t x) { return (struct immediate_t) { .value = x, IMM_VALUE }; }
    static struct immediate_t imm_lbl(char * n) { return (struct immediate_t) { .label = n, IMM_LABEL }; }
    
    #define node(type, param) (struct node_t) { type, param, imm_val(0), imm_val(0), yyloc.first_line, yyloc.first_column }
    #define node2(type, param1, param2) (struct node_t) { type, param1, param2, imm_val(0), yyloc.first_line, yyloc.first_column }
}

%code requires {
    typedef void * yyscan_t;
}

%union {
    char * string;
    struct node_t ins;
    vector(struct node_t) insn;
    struct immediate_t imm;
}

%code {
    int yylex(YYSTYPE * yylvalp, YYLTYPE * yyllocp, yyscan_t scanner);
    void yyerror(YYLTYPE * yyllocp, yyscan_t unused, const char * msg);
    
    void asm_gen(vector(struct node_t));
}

%token END 0 "end of file"

%type<insn> ToplevelScope
%type<ins> Construct
%type<imm> NumericalConstant
%token I_GETC I_GETN I_PUTC I_PUTN
%token I_PSH I_DUP I_XCHG I_DROP
%token I_ADD I_SUB I_MUL I_DIV I_MOD
%token I_STO I_RCL
%token I_CALL I_JMP I_JZ I_JLTZ I_RET I_END
%token I_REP COMMA LF

%token<string> CHAR STRING NUMBER G_LBL G_REF

%start Start
%%
Start
: MaybeLF ToplevelScope { asm_gen($2); }
;

MaybeLF
: %empty
| LF
;

ToplevelScope
: ToplevelScope Construct LF { vector_push_back($1, $2); $$ = $1; }
| %empty { $$ = NULL; }
;

Construct
: I_GETC NumericalConstant { $$ = node(GETC, $2); }
| I_PUTC NumericalConstant { $$ = node(PUTC, $2); }
| I_GETN NumericalConstant { $$ = node(GETN, $2); }
| I_PUTN NumericalConstant { $$ = node(PUTN, $2); }
| I_PSH NumericalConstant { $$ = node(PSH, $2); }
| I_REP I_DUP NumericalConstant { $$ = node(DUP, $3); }
| I_REP I_DROP NumericalConstant { $$ = node(DROP, $3); }
| I_REP I_ADD NumericalConstant { $$ = node(ADD, $3); }
| I_REP I_SUB NumericalConstant { $$ = node(SUB, $3); }
| I_REP I_MUL NumericalConstant { $$ = node(MUL, $3); }
| I_REP I_DIV NumericalConstant { $$ = node(DIV, $3); }
| I_REP I_MOD NumericalConstant { $$ = node(MOD, $3); }
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
| I_CALL NumericalConstant { $$ = node(CALL, $2); }
| I_JMP NumericalConstant { $$ = node(JMP, $2); }
| I_JZ NumericalConstant { $$ = node(BZ, $2); }
| I_JLTZ NumericalConstant { $$ = node(BLTZ, $2); }
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
| I_END { $$ = node(END, imm_none()); }
| NumericalConstant { $$ = node(PSH, $1); }
;

NumericalConstant
: CHAR {
    $$ = imm_val($1[1] == '\\' ? $1[2] : $1[1]);
    free($1);
}
| NUMBER {
    int32_t sign = 1, ret, base = 10;
    
    if(*$1 == '-') {
        sign = -1;
        $1++;
    }

    switch($1[strlen($1) - 1]) {
        case 'h': case 'H': base = 16; break;
        case 'b': case 'B': base =  2; break;
    }

    ret = strtol($1, NULL, base);

    if(sign == -1)
        $1--;
    
    $$ = imm_val(ret * sign);
    free($1);
}
| G_LBL {
    $$ = imm_lbl(++$1);
}
| G_REF {
    $$ = imm_lbl(++$1);
}
;

%%

void yyerror(YYLTYPE * yyllocp, yyscan_t unused, const char * msg) {
    (void) unused;
    fprintf(stderr, "wsi: %d:%d: %s\n", yyllocp->first_line, yyllocp->first_column, msg);
    exit(1);
}
