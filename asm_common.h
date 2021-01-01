
#ifndef _ASM_COMMON_H
#define _ASM_COMMON_H

#include "common.h"

struct node_t {
    uint8_t type;
    int32_t data;
    uint32_t line, column;
};

void asm_gen(vector(struct node_t));

#endif
