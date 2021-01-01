
#include "asm_common.h"

void asm_gen(vector(struct node_t) data) {
    if(data) {
        vector_foreach(struct node_t, it, data) {
            printf("%d: %d\n", it->type, it->data1);
        }
    }

    vector_free(data);
    return;
}
