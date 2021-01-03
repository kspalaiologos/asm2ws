
#include "asm_common.h"
void asm_optimize(vector(struct node_t) * data) {
    vector_foreach(struct node_t, n, *data) {
        switch(n->type) {
            case MUL:
                if(n->data1.type == IMM_VALUE && n->data1.value == 2) {
                    n->data1.type = IMM_NONE;
                    n->data1.value = 0;
                    n->type = ADD;

                    vector_insert(*data, n - vector_begin(*data), *n);
                }

                break;
        }
    }
}
