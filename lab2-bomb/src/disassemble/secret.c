/* Address    Value
 * 0x6030f0   0x24
 * 0x6030f8   0x603110
 * 0x603110   0x8
 * 0x603120   0x603150
 * 0x603150   0x16
 *
 * rdi : 0x6030f0
 * esi : num - 1
 * */
void fun7(rdi, esi) { // must return 2
    if(rdi == NULL) {
        return -1;
    } else {
        edx = &rdi;
        if(edx > esi) {
            return 2 * fun7(&(rdi + 8), esi); // No.1 => esi <= 0x24
        } else {
            if (edx == esi) {
                return 0; // No.3  esi == 0x16 
            } else {
                return 1 + 2 * fun7(&(rdi + 0x10), esi); // No.2 => esi > 0x8
            }
        }
    }
}

void secret() {
    rdi = read_line();
    rax = strtol(rdi, NULL, 10);
    
    rbx = rax; 
    eax = rax - 1;

    if(eax > 0x3eb) bomb();
    else {
        eax = fun7(0x6030f0, ebx);
        if(eax != 0x2) bomb();
    }
}
