
#include "asm_common.h"

#define INSERT_HERE(i, x) vector_insert(*data, n - vector_begin(*data) + (i), (x))
#define ERASE_HERE(i) vector_erase(*data, n - vector_begin(*data) + (i))

#define HAS(x) ((uintptr_t) (n - vector_begin(*data) - (x)) > vector_size(*data))

// optlevel = 1: optimize for size
// optlevel = 2: optimize for speed
void asm_optimize(vector(struct node_t) * data, int optlevel) {
    for(uint32_t idx = 0; idx < vector_size(*data); idx++) {
        struct node_t * n = (*data) + idx;

        switch(n->type) {
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
