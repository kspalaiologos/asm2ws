
#ifndef _COMMON_H
#define _COMMON_H

#include "config.h"

#include <stdio.h>
#include "whitespace.h"

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
