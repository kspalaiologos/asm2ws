
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "asm_common.h"

/* label ordering */

struct _label_t {
    char * name;
    vector(struct node_t *) references;
    uint8_t declared;
};

struct label_state_t {
    vector(struct _label_t) labels;
};

struct _label_t * getlabel(struct label_state_t * ctx, char * name) {
    vector_foreach(struct _label_t, it, ctx->labels)
        if(!strcmp(it->name, name))
            return it;
    return NULL;
}

void push_label(struct label_state_t * ctx, char * name, struct node_t * id) {
    struct _label_t * ref = NULL;
    if((ref = getlabel(ctx, name + 1)) != NULL) {
        if(ref->declared == 1) {
            fprintf(stderr, "wsi: label redeclared: '%s'\n", name);
            exit(1);
        }

        ref->declared = 1;
        vector_push_back(ref->references, id);
        free(name);
        return;
    }

    struct _label_t instance;
    instance.declared = 1;
    instance.references = NULL;
    vector_push_back(instance.references, id);
    instance.name = name + 1;

    vector_push_back(ctx->labels, instance);
}

void pop_label(struct label_state_t * ctx, char * name, struct node_t * id) {
    struct _label_t * ref = NULL;
    if((ref = getlabel(ctx, name + 1)) != NULL) {
        vector_push_back(ref->references, id);
        free(name);
        return;
    }

    struct _label_t instance;
    instance.declared = 0;
    instance.references = NULL;
    vector_push_back(instance.references, id);
    instance.name = name + 1;

    vector_push_back(ctx->labels, instance);
}

int comparator(const void * a, const void * b) {
    const struct _label_t * la = a, * lb = b;
    return vector_size(la->references) - vector_size(lb->references);
}

void dump_labels(struct label_state_t * ctx) {
    vector_foreach(struct _label_t, it, ctx->labels) {
        fprintf(stderr, "%s - refs: %lu\n", it->name, vector_size(it->references));
    }
}

void finalize_labels(struct label_state_t * ctx) {
    uint32_t n = 1;

    if(ctx->labels == NULL)
        return;

    qsort(ctx->labels, vector_size(ctx->labels), sizeof(struct _label_t), comparator);
    vector_foreach(struct _label_t, it, ctx->labels) {
        if(!it->declared) {
            fprintf(stderr, "wsi: unresolved reference: '%s'\n", it->name);
            exit(1);
        }

        if(vector_size(it->references) == 1) {
            it->references[0]->data1.type = IMM_LABEL;
        } else {
            vector_foreach(struct node_t *, it2, it->references)
                (*it2)->data1.value = n;
            ++n;
        }
        
        free(it->name - 1);
        vector_free(it->references);
    }
    vector_free(ctx->labels);
}

#define S fputc(' ', output)
#define T fputc('\t', output)
#define N fputc('\n', output)

#define rep(bound) for(x = 0; x < bound; x++)

void numeral(FILE * output, int32_t x) {
    uint32_t u = x;
    if(x < 0) { T; u = -u; } else { S; }
    uint8_t b[32];
    int8_t i = 0, j;
    while(u) { b[i++] = u & 1; u >>= 1; }
    for(j = i - 1; j >= 0; j--)
        if(b[j]) T; else S;
    N;
}

void unumeral(FILE * output, int32_t x) {
    uint8_t b[32];
    int8_t i = 0, j;
    while(x) { b[i++] = x & 1; x >>= 1; }
    for(j = i - 1; j >= 0; j--)
        if(b[j]) T; else S;
    N;
}

void asm_gen(FILE * output, vector(struct node_t) data, int optlevel) {
    if(data) {
        int32_t x;

        // Pass 1: Labels to numbers.
        struct label_state_t s = { NULL };
        vector_foreach(struct node_t, it, data) {
            if(it->type == LBL) {
                it->data1.type = IMM_VALUE;
                push_label(&s, it->data1.label, it);
                continue;
            }

            if(it->data1.type == IMM_LABEL) {
                it->data1.type = IMM_VALUE;
                pop_label(&s, it->data1.label, it);
            }

            if(it->data2.type == IMM_LABEL) {
                it->data2.type = IMM_VALUE;
                pop_label(&s, it->data2.label, it);
            }

            if(it->data3.type == IMM_LABEL) {
                it->data3.type = IMM_VALUE;
                pop_label(&s, it->data3.label, it);
            }
        }

        finalize_labels(&s);

        // Pass 2: Optimizing the code.
        asm_optimize(&data, optlevel);

        // Pass 3: Generating code.
        vector_foreach(struct node_t, it, data) {
            switch(it->type) {
                case GETC:
                    if(it->data1.type != IMM_NONE) {
                        S;S;numeral(output, it->data1.value); // GETC N => PUSH N
                    }

                    T;N;T;S; // GETC
                    break;
                case PUTC:
                    if(it->data2.type != IMM_NONE) {
                        // REP PUTC X times
                        rep(it->data1.value) {
                            T;N;S;S;
                        }
                        
                        break;
                    }

                    if(it->data1.type != IMM_NONE) {
                        S;S;numeral(output, it->data1.value); // PUTC N => PUSH N
                    }
                    
                    T;N;S;S; // PUTC
                    break;
                case GETN:
                    if(it->data1.type != IMM_NONE) {
                        S;S;numeral(output, it->data1.value); // GETN N => PUSH N
                    }
                    
                    T;N;T;T; // GETN
                    break;
                case PUTN:
                    if(it->data2.type != IMM_NONE) {
                        // REP PUTN X times
                        rep(it->data1.value) {
                            T;N;S;T;
                        }
                        
                        break;
                    }

                    if(it->data1.type != IMM_NONE) {
                        S;S;numeral(output, it->data1.value); // PUTN N => PUSH N
                    }
                    
                    T;N;S;T; // PUTN
                    break;
                case PSH:
                    if(it->data1.type != IMM_NONE) {
                        S;S;numeral(output, it->data1.value); // PUSH N => PUSH N
                    } else {
                        S;S;numeral(output, 0); // PUSH => PUSH 0
                    }

                    break;
                case DUP:
                    if(it->data1.type != IMM_NONE) {
                        // REP DUP N
                        rep(it->data1.value) {
                            S;N;S;
                        }
                    } else {
                        // DUP
                        S;N;S;
                    }
                    
                    break;
                case DROP:
                    if(it->data1.type != IMM_NONE) {
                        // REP DROP N
                        rep(it->data1.value) {
                            S;N;N;
                        }
                    } else {
                        // DROP
                        S;N;N;
                    }

                    break;
                case ADD:
                    // TODO: REP ADD N, M - add N, M times
                    if(it->data2.type != IMM_NONE) {
                        // REP ADD N
                        // `ADD' N times.
                        rep(it->data1.value) {
                            T;S;S;S;
                        }
                    } else if(it->data1.type != IMM_NONE) {
                        // PSH N / ADD
                        S;S;numeral(output, it->data1.value);
                        T;S;S;S;
                    } else {
                        // ADD
                        T;S;S;S;
                    }

                    break;
                case SUB:
                    // TODO: REP SUB N, M - subtract N, M times
                    if(it->data2.type != IMM_NONE) {
                        // REP SUB N
                        // `SUB' N times.
                        rep(it->data1.value) {
                            T;S;S;T;
                        }
                    } else if(it->data1.type != IMM_NONE) {
                        // PSH N / SUB
                        S;S;numeral(output, it->data1.value);
                        T;S;S;T;
                    } else {
                        // SUB
                        T;S;S;T;
                    }
                    
                    break;
                case MUL:
                    // TODO: REP MUL N, M - multiply N, M times
                    if(it->data2.type != IMM_NONE) {
                        // REP MUL N
                        // `MUL' N times.
                        rep(it->data1.value) {
                            T;S;S;N;
                        }
                    } else if(it->data1.type != IMM_NONE) {
                        // PSH N / MUL
                        S;S;numeral(output, it->data1.value);
                        T;S;S;N;
                    } else {
                        // MUL
                        T;S;S;N;
                    }

                    break;
                case DIV:
                    // TODO: REP DIV N, M - divide N, M times
                    if(it->data2.type != IMM_NONE) {
                        // REP DIV N
                        // `DIV' N times.
                        rep(it->data1.value) {
                            T;S;T;S;
                        }
                    } else if(it->data1.type != IMM_NONE) {
                        // PSH N / DIV
                        if(it->data1.value == 0)
                            fprintf(stderr, "warn: %u:%u: division by zero.", it->line, it->column);
                        S;S;numeral(output, it->data1.value);
                        T;S;T;S;
                    } else {
                        // DIV
                        T;S;T;S;
                    }

                    break;
                case MOD:
                    // TODO: REP MOD N, M - divide N, M times
                    if(it->data2.type != IMM_NONE) {
                        // REP MOD N
                        // `MOD' N times.
                        rep(it->data1.value) {
                            T;S;T;T;
                        }
                    } else if(it->data1.type != IMM_NONE) {
                        // PSH N / MOD
                        if(it->data1.value == 0)
                            fprintf(stderr, "warn: %u:%u: division by zero.", it->line, it->column);
                        S;S;numeral(output, it->data1.value);
                        T;S;T;T;
                    } else {
                        // MOD
                        T;S;T;T;
                    }

                    break;
                case STO:
                    if(it->data2.type != IMM_NONE) {
                        // STO THERE, THAT
                        S;S;numeral(output, it->data1.value);
                        S;S;numeral(output, it->data2.value);
                    } else if(it->data1.type != IMM_NONE) {
                        // PSH N / STO
                        S;S;numeral(output, it->data1.value);
                    }

                    // else, STO
                    T;T;S;

                    break;
                case RCL:
                    if(it->data1.type != IMM_NONE) {
                        // PSH N / RCL
                        S;S;numeral(output, it->data1.value);
                    }

                    // else, RCL
                    T;T;T;

                    break;
                case CALL:
                    N;S;T;unumeral(output, it->data1.value);
                    break;
                case JMP:
                    N;S;N;unumeral(output, it->data1.value);
                    break;
                case BZ:
                    N;T;S;unumeral(output, it->data1.value);
                    break;
                case BLTZ:
                    N;T;T;unumeral(output, it->data1.value);
                    break;
                case RET:
                    N;T;N;
                    break;
                case XCHG:
                    S;N;T;
                    break;
                case STOP:
                    N;N;N;
                    break;
                case LBL:
                    if(it->data1.type == IMM_LABEL)
                        break;
                    N;S;S;unumeral(output, it->data1.value);
                    break;
                case COPY:
                    S;T;S;numeral(output, it->data1.value);
                    break;
                case SLIDE:
                    S;T;N;numeral(output, it->data1.value);
                    break;
            }
        }
    }

    vector_free(data);
    return;
}
