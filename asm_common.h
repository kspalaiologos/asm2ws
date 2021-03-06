
#ifndef _ASM_COMMON_H
#define _ASM_COMMON_H

#include <stdio.h>
#include "common.h"

enum { IMM_LABEL, IMM_VALUE, IMM_NONE };

struct immediate_t {
    union {
        int32_t value;
        char * label;
    };

    uint8_t type;
};

struct node_t {
    uint8_t type;
    struct immediate_t data1, data2, data3;
    uint32_t line, column;
};

void asm_gen(FILE *, vector(struct node_t), int optlevel);
void asm_optimize(vector(struct node_t) * data, int optlevel);

#endif
