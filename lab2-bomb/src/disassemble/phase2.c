
/*
 * read six numbers from input,
 * and store them in an arr
 * */
void read_six_numbers(input, arr) {
    int result =
        sscanf(input,         // rdi 
           0x4025c3,      // rsi // "%d %d %d %d %d %d"
           arr,           // rdx
           arr + 0x4,     // rcx
           arr + 0x8,     // r8
           arr + 0xc,     // r9
           arr + 0x10,    // rsp
           arr + 0x14);   // rsp + 8  
}

void phase_2() {
    read_six_numbers(input, rsp); // a,b,c,d,e,f
   
    /**
     * a = &(rsp)
     * b = &(rsp + 0x4)
     * c = &(rsp + 0x8)
     * d = &(rsp + 0xc)
     * e = &(rsp + 0x10)
     * f = &(rsp + 0x14)
     * */

    if(a != 1) bomb(); // a == 1
    else {
        rbx = rsp + 4;
        rbp = rsp + 18;
        
        while(true) {
            // b = 2, c = 4, d = 8, e = 16, f = 32
            if (&rbx != (&(rbx - 4) * 2)) bomb();
            else {
                rbx += 4;
                if(rbp == rbx) break;
            }
        }
    }
}

