eax = sscanf(input, 0x4025cf, rsp + 0x8, rsp + 0xc); // 0x4025cf : "%d %d"
if (eax <= 1) bomb();
else {
    if( &(rsp + 0x8) > 0x7) {
        bomb();
    } else {
        eax = &(rsp + 0x8);
        
        /* jmp to &(0x402470 + 8 * %rax)
         * 
         * Jump table:
         *
         * Address       Value        &(rsp + 0x8)    => eax == &(rsp + 0xc)
         * 0x402470      0x400f7c          0           0xcf
         * 0x402478      0x400fb9          1           0x137
         * 0x402480      0x400f83          2           0x2c3
         * 0x402488      0x400f8a          3           0x100
         * 0x402490      0x400f91          4           0x185
         * 0x402498      0x400f98          5           0xce
         * 0x4024a0      0x400f9f          6           0x2aa
         * 0x4024a8      0x400fa6          7           0x147
         */

        // then jump to 0x400fbe

        if (eax != &(rsp + 0xc)) bomb()
    }
}

