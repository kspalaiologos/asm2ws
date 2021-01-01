
#include <stdio.h>
#include "common.h"

void disasm_single(FILE * output, struct instruction_t * it) {
    switch(it->type) {
        case GETC: fprintf(output, "\tGETC\n"); break;
        case GETN: fprintf(output, "\tGETN\n"); break;
        case PUTN: fprintf(output, "\tPUTN\n"); break;
        case PUTC: fprintf(output, "\tPUTC\n"); break;
        case PSH: fprintf(output, "\tPSH %d\n", it->data); break;
        case DUP: fprintf(output, "\tDUP\n"); break;
        case XCHG: fprintf(output, "\tXCHG\n"); break;
        case DROP: fprintf(output, "\tDROP\n"); break;
        case ADD: fprintf(output, "\tADD\n"); break;
        case SUB: fprintf(output, "\tSUB\n"); break;
        case MUL: fprintf(output, "\tMUL\n"); break;
        case DIV: fprintf(output, "\tDIV\n"); break;
        case MOD: fprintf(output, "\tMOD\n"); break;
        case STO: fprintf(output, "\tSTO\n"); break;
        case RCL: fprintf(output, "\tRCL\n"); break;
        case LBL: fprintf(output, "@L%d\n", it->data); break;
        case CALL: fprintf(output, "\tCALL %%L%d\n", it->data); break;
        case JMP: fprintf(output, "\tJMP %%L%d\n", it->data); break;
        case BZ: fprintf(output, "\tJZ %%L%d\n", it->data); break;
        case BLTZ: fprintf(output, "\tJLTZ %%L%d\n", it->data); break;
        case RET: fprintf(output, "\tRET\n"); break;
        case STOP: fprintf(output, "\tEND\n"); break;
        case ERR: fprintf(output, "\tERROR\n"); break;
        default: fprintf(output, "\t??\n"); break;
    }
}

void disasm(FILE * output, vector(struct instruction_t) program) {
    vector_foreach(struct instruction_t, it, program)
        disasm_single(output, it);
}
