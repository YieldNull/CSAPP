void func4(edi, esi, edx) {
    eax = edx - esi;
    ecx = eax;

    ecx >>>= 0x1f; // sign bit

    eax += ecx;
    eax /= 2;
    ecx = rax + rsi;
   
    /*
     * ecx = ((edx - esi) / 2) + rsi = (edx + esi) / 2 
     *  
     * edx    esi     ecx   edi (to stop)
     * 14      0       7    7
     * 6       0       3    3
     * 2       0       1    1
     * 1       0       0    0
     */
 
    if (ecx > edi) {
        edx = rcx - 1;
        return 2 * func4(edi, esi, edx);
    } else {
        if (ecx < edi) { // unreachable
            esi = rcx + 1;
            return 1 + 2 * func4(edi, esi, edx);
        } else {
            return 0; // final 
        }
    }
}

void phase_4(input) { // input two number a,b
    n = sscanf(input, 0x4025cf, rsp + 0x8, rsp + 0xc);
    if (n != 2) bomb();
    else {
        if(&(rsp + 0x8) > 0xe) bomb();  // a < 14
        else {
            eax = func4(&(rsp + 0x8), 0x0, 0xe); // must return 0

            if(eax != 0) bomb();
            else if (&(rsp + 0xc) != 0x0) bomb(); // b must be 0
        }
    }
}
