
#include <string.h>
#include <stdint.h>
#include "common.h"

static int8_t next(FILE * input) {
    char c;
    while((c = getc(input)) != EOF)
        if(c == ' ' || c == '\n' || c == '\t')
            return c;
    return 0;
}

static int32_t getnum(FILE * input) {
    uint8_t sign = next(input), c;
    int32_t n = 0, q;
    if(sign != '\t' && sign != ' ')
        return 0;
    q = sign == '\t' ? -1 : 1;
    while((c = next(input)) != '\n') {
        n <<= 1;
        if(c == '\t') n++;
        if(!c) break;
    }
    return n * q;
}

static vector(char) getlab(FILE * input) {
    vector(char) label = NULL;
    uint8_t c;
    while((c = next(input)) != '\n')
        vector_push_back(label, c == '\t' ? 'T' : 'S');
    vector_push_back(label, 0);
    return label;
}

static struct instruction_t parse_heap(FILE * input, void (*fatal)(char * s)) {
    switch(next(input)) {
        case ' ': return (struct instruction_t) { STO, 0 };
        case '\t': return (struct instruction_t) { RCL, 0 };
        default: fatal("<lf>|e, IMP heap"); return (struct instruction_t) { ERR, 0 };
    }
}

static struct instruction_t parse_arith(FILE * input, void (*fatal)(char * s)) {
    switch(next(input)) {
        case ' ': switch(next(input)) {
            case ' ': return (struct instruction_t) { ADD, 0 };
            case '\t': return (struct instruction_t) { SUB, 0 };
            case '\n': return (struct instruction_t) { MUL, 0 };
            default: fatal("e, IMP arith"); return (struct instruction_t) { ERR, 0 };
        }
        case '\t': switch(next(input)) {
            case ' ': return (struct instruction_t) { DIV, 0 };
            case '\t': return (struct instruction_t) { MOD, 0 };
            default: fatal("<lf>|e, IMP arith"); return (struct instruction_t) { ERR, 0 };
        }
        default: fatal("<lf>|e, IMP arith"); return (struct instruction_t) { ERR, 0 };
    }
}

static struct instruction_t parse_io(FILE * input, void (*fatal)(char * s)) {
    switch(next(input)) {
        case ' ': switch(next(input)) {
            case ' ': return (struct instruction_t) { PUTC, 0 };
            case '\t': return (struct instruction_t) { PUTN, 0 };
            default: fatal("<lf>|e, IMP io"); return (struct instruction_t) { ERR, 0 };
        }
        case '\t': switch(next(input)) {
            case ' ': return (struct instruction_t) { GETC, 0 };
            case '\t': return (struct instruction_t) { GETN, 0 };
            default: fatal("<lf>|e, IMP io"); return (struct instruction_t) { ERR, 0 };
        }
        default: fatal("<lf>|e, IMP io"); return (struct instruction_t) { ERR, 0 };
    }
}

static struct instruction_t parse_stack(FILE * input, void (*fatal)(char * s)) {
    switch(next(input)) {
        case ' ': return (struct instruction_t) { PSH, getnum(input) };
        case '\n': switch(next(input)) {
            case ' ': return (struct instruction_t) { DUP, 0 };
            case '\t': return (struct instruction_t) { XCHG, 0 };
            case '\n': return (struct instruction_t) { DROP, 0 };
            default: fatal("e, IMP stk"); return (struct instruction_t) { ERR, 0 };
        }
        default: fatal("<tab>|e, IMP stk"); return (struct instruction_t) { ERR, 0 };
    }
}

static struct instruction_t parse_flow(FILE * input, void (*fatal)(char * s)) {
    switch(next(input)) {
        case ' ': switch(next(input)) {
            case ' ':return (struct instruction_t) { LBL, .label = getlab(input) };
            case '\t': return (struct instruction_t) { CALL, .label = getlab(input) };
            case '\n': return (struct instruction_t) { JMP, .label = getlab(input) };
            default: fatal("e, IMP flow"); return (struct instruction_t) { ERR, 0 };
        }
        case '\t': switch(next(input)) {
            case ' ': return (struct instruction_t) { BZ, .label = getlab(input) };
            case '\t': return (struct instruction_t) { BLTZ, .label = getlab(input) };
            case '\n': return (struct instruction_t) { RET, 0 };
            default: fatal("e, IMP flow"); return (struct instruction_t) { ERR, 0 };
        }
        case '\n':
            if(next(input) == '\n')
                return (struct instruction_t) { END, 0 };
            else
                { fatal("bad end, IMP flow"); return (struct instruction_t) { ERR, 0 }; }
        default: fatal("e, IMP flow"); return (struct instruction_t) { ERR, 0 };
    }
}

struct _label_t {
    int32_t id;
    char * name;
    struct instruction_t parent;
};

static struct _label_t * getlabel(vector(struct _label_t) vec, char * label_text) {
    vector_foreach(struct _label_t, it, vec)
        if(!strcmp(label_text, it->name))
            return it;
    return NULL;
}

static vector(struct label_t) fixup_labels(vector(struct instruction_t) program, void(*warn)(char * s)) {
    vector(struct _label_t) labels = NULL;
    vector(struct label_t) labs = NULL;
    int32_t labid = 1;

    vector_foreach(struct instruction_t, it, program)
        if(it->type == LBL) {
            struct _label_t * l;
            if((l = getlabel(labels, it->label)) != NULL) {
                warn("duplicated label.");
                vector_free(it->label);
                it->data = l->id;
                continue;
            }
            struct _label_t lab = {.id = labid, .parent = *it, .name = it->label};
            struct label_t public_label = {.id = labid, .parent = it};
            it->data = labid++;
            vector_push_back(labels, lab);
            vector_push_back(labs, public_label);
        }

    vector_foreach(struct instruction_t, it, program)
        switch(it->type) {
            case CALL: case JMP: case BZ: case BLTZ: {
                struct _label_t * l;
                if((l = getlabel(labels, it->label)) != NULL) {
                    vector_free(it->label);
                    it->data = l->id;
                } else {
                    vector_free(it->label);
                    warn("dead label.");
                    it->data = 0;
                }
            }
        }

    vector_foreach(struct _label_t, it, labels)
        vector_free(it->name);

    vector_free(labels);

    return labs;
}

struct parse_result_t parse(FILE * input, void (*fatal)(char * s), void (*warn)(char * s)) {
    vector(struct instruction_t) q = NULL;
    struct instruction_t cur;

    while(!feof(input)) {
        switch(next(input)) {
            case '\t':
                switch(next(input)) {
                    case '\t':
                        vector_push_back(q, cur = parse_heap(input, fatal));
                        break;
                    case ' ':
                        vector_push_back(q, cur = parse_arith(input, fatal));
                        break;
                    case '\n':
                        vector_push_back(q, cur = parse_io(input, fatal));
                        break;
                    default:
                        fatal("? <tab>, E, IMP N/A"); return (struct parse_result_t) { NULL, NULL };
                }
                break;
            case ' ':
                vector_push_back(q, cur = parse_stack(input, fatal));
                break;
            case '\n':
                vector_push_back(q, cur = parse_flow(input, fatal));
                break;
        }

        if(cur.type == ERR)
            return (struct parse_result_t) { NULL, NULL };
    }

    return (struct parse_result_t) { q, fixup_labels(q, warn) };
}
