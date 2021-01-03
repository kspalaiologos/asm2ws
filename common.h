
#ifndef _COMMON_H
#define _COMMON_H

#include "config.h"

#include <stdio.h>
#include "whitespace.h"

// after parse calls error(), two behaviors are acceptable:
// a) terminating the program, so that the control flow never comes back to parse
// b) allowing the control flow to go to parse, but accepting the fact that the parse result
//    contains invalid data, which shouldn't be passed into other functions.
struct parse_result_t parse(FILE * input, void (*error)(char * s), void (*warn)(char * s));
void disasm(FILE * output, vector(struct instruction_t) program);
void disasm_single(FILE * output, struct instruction_t * it);
int32_t run(struct parse_result_t program, struct state * state, void(*fatal)(char *));
char * compile(struct parse_result_t program);
void asm_file(FILE * f);

#ifdef JIT

#ifndef HAVE_LIBTCC_H
#error JIT compiler used without libtcc.h present.
#endif

void run_jit(struct parse_result_t program, void(*fatal)(char *));

#endif

#endif
