
#include "common.h"

#ifdef JIT

#include <libtcc.h>
void run_jit(struct parse_result_t program, void(*fatal)(char *)) {
    TCCState * s;
    char * code;
    s = tcc_new();

    if(!s)
        fatal("ICE");
    
    tcc_set_output_type(s, TCC_OUTPUT_MEMORY);

    code = compile(program);
    
    tcc_define_symbol(s, "DURING_JIT", "");

    if(tcc_compile_string(s, code) == -1)
        fatal("ICE: check -a output.");
    
    free(code);

    if(tcc_relocate(s, TCC_RELOCATE_AUTO) < 0)
        fatal("ICE");
    
    ((void (*)(void)) tcc_get_symbol(s, "main"))();

    tcc_delete(s);
}

#endif
