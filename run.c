
#include "whitespace.h"
#include "common.h"

#define GUARD(x, y) if(vector_size(x) < y) fatal("stack underflow");
#define AT(x, y) vector_end(x)[-y]
#define BILOAD \
    rhs = AT(state->stack, 1); \
    lhs = AT(state->stack, 2); \
    vector_pop_back(state->stack); \
    vector_pop_back(state->stack)
#define DONE cycles++; break

int32_t run(struct parse_result_t program, struct state * state, void(*fatal)(char *)) {
    int32_t lhs, rhs, tmp, cycles = 0;
    vector_foreach(struct instruction_t, ins, program.program) {
        switch(ins->type) {
            case PSH:
                vector_push_back(state->stack, ins->data);
                DONE;
            case DUP:
                GUARD(state->stack, 1);
                vector_push_back(state->stack, AT(state->stack, 1));
                DONE;
            case DROP:
                vector_pop_back(state->stack);
                DONE;
            case XCHG:
                GUARD(state->stack, 2);
                tmp = AT(state->stack, 2);
                AT(state->stack, 2) = AT(state->stack, 1);
                AT(state->stack, 1) = tmp;
                DONE;
            case GETC:
                GUARD(state->stack, 1);
                rhs = AT(state->stack, 1);
                vector_pop_back(state->stack);
                if(rhs < 0)
                    fatal("storing to address < 0");
                while(vector_size(state->heap) <= (unsigned) rhs)
                    vector_push_back(state->heap, 0);
                state->heap[rhs] = getchar();
                DONE;
            case PUTC:
                GUARD(state->stack, 1);
                putchar(AT(state->stack, 1));
                vector_pop_back(state->stack);
                DONE;
            case GETN:
                GUARD(state->stack, 1);
                scanf("%d", &tmp);
                rhs = AT(state->stack, 1);
                vector_pop_back(state->stack);
                if(rhs < 0)
                    fatal("storing to address < 0");
                while(vector_size(state->heap) <= (unsigned) rhs)
                    vector_push_back(state->heap, 0);
                state->heap[rhs] = tmp;
                DONE;
            case PUTN:
                GUARD(state->stack, 1);
                printf("%d", AT(state->stack, 1));
                vector_pop_back(state->stack);
                DONE;
            case ADD:
                GUARD(state->stack, 2);
                BILOAD;
                vector_push_back(state->stack, lhs + rhs);
                DONE;
            case SUB:
                GUARD(state->stack, 2);
                BILOAD;
                vector_push_back(state->stack, lhs - rhs);
                DONE;
            case MUL:
                GUARD(state->stack, 2);
                BILOAD;
                vector_push_back(state->stack, lhs * rhs);
                DONE;
            case DIV:
                GUARD(state->stack, 2);
                BILOAD;
                if(rhs == 0)
                    fatal("division by zero.");
                vector_push_back(state->stack, lhs / rhs);
                DONE;
            case MOD:
                GUARD(state->stack, 2);
                BILOAD;
                if(rhs == 0)
                    fatal("division by zero.");
                vector_push_back(state->stack, lhs % rhs);
                DONE;
            case END:
                return cycles;
            case STO:
                GUARD(state->stack, 2);
                BILOAD;
                if(lhs < 0)
                    fatal("storing to address < 0");
                while(vector_size(state->heap) <= (unsigned) lhs)
                    vector_push_back(state->heap, 0);
                state->heap[lhs] = rhs;
                DONE;
            case RCL:
                GUARD(state->stack, 1);
                lhs = AT(state->stack, 1); // addr
                vector_pop_back(state->stack);
                if(lhs < 0)
                    fatal("recalling from address < 0");
                if(vector_size(state->heap) <= (unsigned) lhs)
                    vector_push_back(state->stack, 0);
                else
                    vector_push_back(state->stack, state->heap[lhs]);
                DONE;
            case JMP:
                ins = program.labels[ins->data - 1].parent;
                DONE;
            case BZ:
                GUARD(state->stack, 1);
                lhs = AT(state->stack, 1); // addr
                vector_pop_back(state->stack);
                if(lhs == 0)
                    ins = program.labels[ins->data - 1].parent;
                DONE;
            case BLTZ:
                GUARD(state->stack, 1);
                lhs = AT(state->stack, 1); // addr
                vector_pop_back(state->stack);
                if(lhs < 0)
                    ins = program.labels[ins->data - 1].parent;
                DONE;
            case CALL:
                vector_push_back(state->callstack, ins);
                ins = program.labels[ins->data - 1].parent;
                DONE;
            case RET: {
                GUARD(state->callstack, 1);
                struct instruction_t * ret = AT(state->callstack, 1); // addr
                vector_pop_back(state->callstack);
                ins = ret;
                DONE;
            }
        }
    }

    return cycles;
}
