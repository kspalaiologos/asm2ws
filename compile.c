
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>

#include "vector.h"
#include "common.h"

static void append_code(char ** buf, char * format, ...) {
    va_list args, args2;
    va_start(args, format);
    va_copy(args2, args);
    uint32_t buflen = *buf ? strlen(*buf) : 0;
    uint32_t length = 1 + buflen + vsnprintf(NULL, 0, format, args);
    va_end(args);
    if(*buf)
        *buf = realloc(*buf, length);
    else
        *buf = malloc(length);
    assert(*buf);
    vsnprintf(*buf + buflen, length - buflen, format, args2);
    va_end(args2);
}

#define emit(x) append_code(&code, x)
#define emitf(x,...) append_code(&code, x, __VA_ARGS__)

char * compile(struct parse_result_t program) {
    unsigned callid = vector_size(program.labels);
    char * code = NULL;
    emit(
        "#include \"vector.h\"\n"
        "#ifndef DURING_JIT\n"
        "\t#include <stdlib.h>\n"
        "\t#include <stdio.h>\n"
        "\t#include <stdint.h>\n"
        "#else\n"
        "\tint putchar(int);\n"
        "\tint getchar(void);\n"
        "\tint printf(char * fmt, ...);\n"
        "\tint scanf(char * fmt, ...);\n"
        "\ttypedef int int32_t;\n"
        "\t#define NULL ((void *) 0)\n"
        "#endif\n"
        "\n"
        "static vector(int32_t) stack;\n"
        "static vector(int32_t) heap;\n"
        "static vector(int32_t) callstack;\n"
        "static int32_t lhs, rhs, tmp, ip;\n"
        "\n"
        "#define AT(x, y) vector_end(x)[-y]\n"
        "#define BILOAD \\\n"
        "\trhs = AT(stack, 1); \\\n"
        "\tlhs = AT(stack, 2); \\\n"
        "\tvector_pop_back(stack); \\\n"
        "\tvector_pop_back(stack)\n"
        "\n"
        "int main(void) {\n"
        "\tbranch: switch(ip) {\n"
        "case 0:\n"
    );

    vector_foreach(struct instruction_t, ins, program.program) {
        switch(ins->type) {
            case GETC:
                emit(
                    "\trhs = AT(stack, 1);\n"
                    "\tvector_pop_back(stack);\n"
                    "\twhile(vector_size(heap) <= (unsigned) rhs)\n"
                    "\t\tvector_push_back(heap, 0);\n"
                    "\theap[rhs] = getchar();\n"
                );
                break;
            case PUTC:
                emit(
                    "\tputchar(AT(stack, 1));\n"
                    "\tvector_pop_back(stack);\n"
                );
                break;
            case GETN:
                emit(
                    "\trhs = AT(stack, 1);\n"
                    "\tvector_pop_back(stack);\n"
                    "\twhile(vector_size(heap) <= (unsigned) rhs)\n"
                    "\t\tvector_push_back(heap, 0);\n"
                    "\tscanf(\"%%d\", &tmp);\n"
                    "\theap[rhs] = tmp;\n"
                );
                break;
            case PUTN:
                emit(
                    "\tprintf(\"%%d\", AT(stack, 1));\n"
                    "\tvector_pop_back(stack);\n"
                );
                break;
            case PSH:
                emitf("\tvector_push_back(stack, %d);\n", ins->data);
                break;
            case DUP:
                emit("\tvector_push_back(stack, AT(stack, 1));\n");
                break;
            case XCHG:
                emit(
                    "\ttmp = AT(stack, 2);\n"
                    "\tAT(stack, 2) = AT(stack, 1);\n"
                    "\tAT(stack, 1) = tmp;\n"
                );
                break;
            case DROP:
                emit("\tvector_pop_back(stack);\n");
                break;
            case ADD:
                emit("\tBILOAD; vector_push_back(stack, lhs + rhs);\n");
                break;
            case SUB:
                emit("\tBILOAD; vector_push_back(stack, lhs - rhs);\n");
                break;
            case DIV:
                emit("\tBILOAD; vector_push_back(stack, lhs / rhs);\n");
                break;
            case MUL:
                emit("\tBILOAD; vector_push_back(stack, lhs * rhs);\n");
                break;
            case MOD:
                emit("\tBILOAD; vector_push_back(stack, lhs %% rhs);\n");
                break;
            case STOP:
                emit("\tgoto end;\n");
                break;
            case STO:
                emit(
                    "\tBILOAD;\n"
                    "\twhile(vector_size(heap) <= (unsigned) lhs)\n"
                    "\t\tvector_push_back(heap, 0);\n"
                    "\theap[lhs] = rhs;\n"
                );
                break;
            case RCL:
                emit(
                    "\tlhs = AT(stack, 1);\n"
                    "\tvector_pop_back(stack);\n"
                    "\tif(vector_size(heap) <= (unsigned) lhs)\n"
                    "\t\tvector_push_back(stack, 0);\n"
                    "\telse\n"
                    "\t\tvector_push_back(stack, heap[lhs]);\n"
                );
                break;
            case LBL:
                emitf("case %d:\n", ins->data);
                break;
            case JMP:
                emitf(
                    "\tip = %d;\n"
                    "\tgoto branch;\n"
                , ins->data);
                break;
            case BZ:
                emitf(
                    "\tlhs = AT(stack, 1);\n"
                    "\tvector_pop_back(stack);\n"
                    "\tif(lhs == 0) {\n"
                    "\t\tip = %d;\n"
                    "\t\tgoto branch;\n"
                    "\t}\n"
                , ins->data);
                break;
            case BLTZ:
                emitf(
                    "\tlhs = AT(stack, 1);\n"
                    "\tvector_pop_back(stack);\n"
                    "\tif(lhs < 0) {\n"
                    "\t\tip = %d;\n"
                    "\t\tgoto branch;\n"
                    "\t}\n"
                , ins->data);
                break;
            case CALL:
                emitf(
                    "\tvector_push_back(callstack, %d);\n"
                    "\tip = %d;\n"
                    "\tgoto branch;\n"
                    "case %d:\n"
                , callid + 1, ins->data, callid + 1);
                callid++;
                break;
            case RET:
                emit(
                    "\tip = AT(callstack, 1);\n"
                    "\tvector_pop_back(callstack);\n"
                    "\tgoto branch;\n"
                );
                break;
        }
    }

    emit(
        "\t}\n"
        "\tend:\n"
        "\tvector_free(stack);\n"
        "\tvector_free(heap);\n"
        "\tvector_free(callstack);\n"
        "}\n"
    );

    return code;
}
