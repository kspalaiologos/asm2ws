
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "whitespace.h"
#include "common.h"

static void error(char * message) {
    fprintf(stderr, "wsi: %s\n", message);
    exit(1);
}

static void warn(char * message) {
    fprintf(stderr, "wsi: warn: %s\n", message);
    exit(1);
}

int main(int argc, char * argv[]) {
    int i, dis = 0, count = 0, aot = 0, jit = 0, masm = 0;
    for(i = 1; i < argc; i++) {
        char * arg = argv[i];
        if(*arg == '-') {
            if(!strcmp(arg, "--help") || arg[1] == 'h') {
                fprintf(stderr,
                    "wsi: a whitespace omnitool.\n"
                    "copyright (c) by Kamila Szewczyk.\n"
                    "licensed under the terms of the GPL license.\n\n"
                    "usage:\n"
                    " wsi [...] program.ws"
                    " -h/--help: display this screen\n"
                    " -d/--disassemble: disassemble the whitespace program.\n"
                    " -c/--cycles: count operations made by the program.\n"
                    " -a/--aot: build a C source file.\n"
                    #ifdef JIT
                    " -j/--jit: enable the JIT compiler.\n"
                    #endif
                    " -m/--masm: run the macro assembler.\n"
                    "default operation: run whitespace code.\n"
                );
                return 1;
            } else if(!strcmp(arg, "--disassemble") || arg[1] == 'd') {
                dis = 1;
            } else if(!strcmp(arg, "--cycles") || arg[1] == 'c') {
                count = 1;
            } else if(!strcmp(arg, "--masm") || arg[1] == 'm') {
                masm = 1;
            } else if(!strcmp(arg, "--aot") || arg[1] == 'a') {
                aot = 1;
#ifdef JIT
            } else if(!strcmp(arg, "--jit") || arg[1] == 'j') {
                jit = 1;
#endif
            } else {
                fprintf(stderr, "wsi: unknown switch: '%s'\n", arg);
                return 1;
            }
        } else {
            if(dis && count) {
                fprintf(stderr, "wsi: disassembling and cycle counting is mutually exclusive.\n");
                return 1;
            }
            
            if(aot && (dis || count)) {
                fprintf(stderr, "wsi: disassembling or cycle counting is mutually exclusive with ahead-of-time compilation.\n");
                return 1;
            }

#ifdef JIT
            if(jit && (aot || dis || count)) {
                fprintf(stderr, "wsi: --jit is exclusive with every other f-flag.");
                return 1;
            }
#endif

            if(masm && (jit || aot || dis || count)) {
                fprintf(stderr, "wsi: --masm is exclusive with every other f-flag.");
                return 1;
            }

            FILE * input = fopen(arg, "rb");
            if(!input) {
                perror("wsi: fopen");
                return 1;
            }
            
            if(masm) {
                asm_file(input);
                fclose(input);
                return 0;
            }

            struct parse_result_t data = parse(input, error, warn);
            fclose(input);

            if(dis) {
                disasm(stdout, data.program);
                vector_free(data.program);
                vector_free(data.labels);
                return 0;
            }

            if(aot) {
                char * buf = compile(data);
                printf("%s", buf);
                vector_free(data.program);
                vector_free(data.labels);
                free(buf);
                return 0;
            }

#ifdef JIT
            if(jit) {
                run_jit(data, error);
                vector_free(data.program);
                vector_free(data.labels);
                return 0;
            }
#endif

            struct state s = { NULL, NULL, NULL };
            int32_t cycles = run(data, &s, error);
            
            if(count) {
                fprintf(stderr, "finished, took %d cycles.\n", cycles);
            }
            
            vector_free(data.program);
            vector_free(data.labels);
            vector_free(s.callstack);
            vector_free(s.heap);
            vector_free(s.stack);
            return 0;
        }
    }

    error("invalid invocation. try -h.");
}
