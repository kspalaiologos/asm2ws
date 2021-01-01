/* A Bison parser, made by GNU Bison 3.7.1.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2020 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

#ifndef YY_YY_ASM_TAB_H_INCLUDED
# define YY_YY_ASM_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif
/* "%code requires" blocks.  */
#line 21 "asm.y"

    typedef void * yyscan_t;

#line 53 "asm.tab.h"

/* Token kinds.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    YYEMPTY = -2,
    END = 0,                       /* "end of file"  */
    YYerror = 1,                   /* error  */
    YYUNDEF = 2,                   /* "invalid token"  */
    I_GETC = 3,                    /* I_GETC  */
    I_GETN = 4,                    /* I_GETN  */
    I_PUTC = 5,                    /* I_PUTC  */
    I_PUTN = 6,                    /* I_PUTN  */
    I_PSH = 7,                     /* I_PSH  */
    I_DUP = 8,                     /* I_DUP  */
    I_XCHG = 9,                    /* I_XCHG  */
    I_DROP = 10,                   /* I_DROP  */
    I_ADD = 11,                    /* I_ADD  */
    I_SUB = 12,                    /* I_SUB  */
    I_MUL = 13,                    /* I_MUL  */
    I_DIV = 14,                    /* I_DIV  */
    I_MOD = 15,                    /* I_MOD  */
    I_STO = 16,                    /* I_STO  */
    I_RCL = 17,                    /* I_RCL  */
    I_CALL = 18,                   /* I_CALL  */
    I_JMP = 19,                    /* I_JMP  */
    I_JZ = 20,                     /* I_JZ  */
    I_JLTZ = 21,                   /* I_JLTZ  */
    I_RET = 22,                    /* I_RET  */
    I_END = 23,                    /* I_END  */
    I_REP = 24,                    /* I_REP  */
    COMMA = 25,                    /* COMMA  */
    LF = 26,                       /* LF  */
    CHAR = 27,                     /* CHAR  */
    STRING = 28,                   /* STRING  */
    NUMBER = 29,                   /* NUMBER  */
    G_LBL = 30,                    /* G_LBL  */
    G_REF = 31                     /* G_REF  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 25 "asm.y"

    char * string;
    struct node_t ins;
    vector(struct node_t) insn;
    struct immediate_t imm;

#line 108 "asm.tab.h"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif

/* Location type.  */
#if ! defined YYLTYPE && ! defined YYLTYPE_IS_DECLARED
typedef struct YYLTYPE YYLTYPE;
struct YYLTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
};
# define YYLTYPE_IS_DECLARED 1
# define YYLTYPE_IS_TRIVIAL 1
#endif



int yyparse (yyscan_t scanner);

#endif /* !YY_YY_ASM_TAB_H_INCLUDED  */
