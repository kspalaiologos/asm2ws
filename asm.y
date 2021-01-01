
%define api.pure full
%define api.token.raw
%define parse.error verbose
%locations
%param { yyscan_t scanner }

%code top {
    #include <stdio.h>
    #include <string.h>
    #include "asm_common.h"
    
    #define node(type, param) (struct node_t) { type, param, yyloc.first_line, yyloc.first_column }
}

%code requires {
    typedef void * yyscan_t;
}

%union {
    char * string;
    struct node_t ins;
    vector(struct node_t) insn;
    int32_t num;
}

%code {
    int yylex(YYSTYPE * yylvalp, YYLTYPE * yyllocp, yyscan_t scanner);
    void yyerror(YYLTYPE * yyllocp, yyscan_t unused, const char * msg);
    
    void asm_gen(vector(struct node_t));
}

%token END 0 "end of file"

%type<insn> ToplevelScope
%type<ins> Construct
%type<num> NumericalConstant
%token I_GETC I_GETN I_PUTC I_PUTN
%token I_PSH I_DUP I_XCHG I_DROP
%token I_ADD I_SUB I_MUL I_DIV I_MOD
%token I_STO I_RCL
%token<string> G_LBL G_REF
%token I_CALL I_JMP I_JZ I_JLTZ I_RET I_END
%token LF

%token<string> CHAR STRING NUMBER

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
| I_GETC { $$ = node(GETC, 0); }
| I_PUTC { $$ = node(PUTC, 0); }
| I_GETN { $$ = node(GETN, 0); }
| I_PUTN { $$ = node(PUTN, 0); }
| I_PSH { $$ = node(PSH, 0); }
;

NumericalConstant
: CHAR {
    $$ = $1[1] == '\\' ? $1[2] : $1[1];
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
    
    $$ = ret * sign;
    free($1);
}
;

%%

void yyerror(YYLTYPE * yyllocp, yyscan_t unused, const char * msg) {
    (void) unused;
    fprintf(stderr, "wsi: %d:%d: %s\n", yyllocp->first_line, yyllocp->first_column, msg);
    exit(1);
}
