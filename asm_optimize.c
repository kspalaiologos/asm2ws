
#include "asm_common.h"

#define INSERT_HERE(i, x) vector_insert(*data, n - vector_begin(*data) + (i), (x))
#define ERASE_HERE(i) vector_erase(*data, n - vector_begin(*data) + (i))

#define HAS(x) ((uintptr_t) (n - vector_begin(*data) - (x)) > vector_size(*data))

void asm_optimize(vector(struct node_t) * data) {
    for(uint32_t idx = 0; idx < vector_size(*data); idx++) {
        struct node_t * n = (*data) + idx;

        switch(n->type) {
            case MUL:
                if(n->data1.type == IMM_VALUE && n->data1.value == 2) {
                    // MUL 2 => ADD / ADD     ~~ unconditionally saves 2 bytes.
                    n->data1.type = IMM_NONE;
                    n->data1.value = 0;
                    n->type = ADD;

                    INSERT_HERE(0, *n);
                }

                break;
            case XCHG:
                if(HAS(1) && n[1].type == XCHG) {
                    // cancel out XCHG / XCHG
                    ERASE_HERE(0);
                    ERASE_HERE(0);
                }

                break;
        }
    }
}
