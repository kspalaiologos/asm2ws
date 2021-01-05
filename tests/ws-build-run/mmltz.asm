
    GETN 0
    GETN 1
    RCL 0
    RCL 1
    MOD
    SUB 5
    PUTC ':'
    BLTZ %ONE
    PUTC '('
    END
@ONE
    PUTC ')'
