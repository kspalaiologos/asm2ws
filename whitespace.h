
#ifndef _WHITESPACE_H
#define _WHITESPACE_H

#include <stdint.h>
#include "vector.h"

enum {
    GETC, PUTC, GETN, PUTN,
    PSH, DUP, XCHG, DROP,
    ADD, SUB, MUL, DIV, MOD, STO, RCL,
    LBL, CALL, JMP, BZ, BLTZ, RET, STOP,
    ERR,

    // v0.3
    COPY, SLIDE
};

struct instruction_t {
    uint8_t type;

    union {
        int32_t data;
        char * label;
    };
};

struct label_t {
    int32_t id;
    struct instruction_t * parent;
};

struct parse_result_t {
    vector(struct instruction_t) program;
    vector(struct label_t) labels;
};

struct state {
    vector(int32_t) stack;
    vector(int32_t) heap;
    vector(struct instruction_t *) callstack;
};

#endif
