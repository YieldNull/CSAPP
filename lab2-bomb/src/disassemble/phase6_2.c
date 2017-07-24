r13 = rsp;
rsi = rsp;

read_six_numbers(input,rsp); // array [a,b,c,d,e,f] start at rsp
r14 = rsp;

r12d = 0;
while(true){
    rbp = r13;
    eax = &r13 - 1;

    if(eax > 5) bomb(); // each number is not more than 6
    else {
        r12d++;
        if(r12d < 6) {
            ebx = r12d;
            do {
                if (&(rsp + 4 * ebx) == &rbp) // each number differs from others
                    bomb();
                else 
                    ebx++;
            } while(ebx <= 5)
            r13 = r13 + 4;
        } else{
            break;
        }
    }    
}

rsi = rsp + 0x18;
rax = r14;
ecx = 7;
do {
    &rax = 7 - &rax; // map with (7 - _)
    rax += 4;
} while (rax < rsi)

rsi = 0;
do () { // At 0x401197
    ecx = &(rsp + rsi);
    if(ecx > 1) {
        eax = 1;
        edx = 0x6032d0;
        do {
            rdx = &(rdx + 8); // At 0x401176
            eax++;         
        } while (eax < ecx)       
    } else { // At 0x401183
        edx = 0x6032d0;
    }
    
    /* address of the value of the linked list
     * +8 is the address of next node
     *
     * rsp + 0x20 + 0x00 = 0x6032d0 + ((7 - a) - 1) * 16
     * rsp + 0x20 + 0x08 = 0x6032d0 + ((7 - b) - 1) * 16
     * rsp + 0x20 + 0x10 = 0x6032d0 + ((7 - c) - 1) * 16
     * rsp + 0x20 + 0x18 = 0x6032d0 + ((7 - d) - 1) * 16
     * rsp + 0x20 + 0x20 = 0x6032d0 + ((7 - e) - 1) * 16
     * rsp + 0x20 + 0x28 = 0x6032d0 + ((7 - f) - 1) * 16
     * */
    &(rsp + 2 * rsi + 0x20) = rdx; // At 0x401188
    rsi += 4;

} while (rsi < 0x18)

/* The original linked list
 *
 * Address      Value          Next
 * 0x6032d0     0x10000014c    0x6032e0
 * 0x6032e0     0x2000000a8    0x6032f0
 * 0x6032f0     0x30000039c    0x603300
 * 0x603300     0x4000002b3    0x603310
 * 0x603310     0x5000001dd    0x603320
 * 0x603320     0x6000001bb    0x000000
 */

rbx = &(rsp + 0x20);
rax = rsp + 0x28;
rsi = rsp + 0x50; // limit
rcx = rbx;

// relink the linked list from (rsp + 0x20) to (rsp + 0x48) in sequence
while(true) {
    rdx = &rax;
    &(rcx + 8) = rdx;
    rax += 8;

    if(rax < rsi) rcx = rdx;
    else break;
}
(rdx + 0x8) = 0; // set Nil to the end of list


ebp = 5;
do {
    rax = &(rbx + 0x8);
    eax = &rax; // next value, truncate the higher 32 bits
    
    // &rbx is the current value 
    if(&rbx < eax) bomb(); // so the relinked list is descending sorted 
    else {
        rbx = &(rbx + 0x8);
        ebp -= 1;       
    }   
} while(ebp > 0)

/* let's sort the original list:
 *
 * Value     Index          Calculation 
 * 0x39c      2      7 - a - 1 = 2  => a = 4
 * 0x2b3      3      7 - b - 1 = 3  => b = 3
 * 0x1dd      4      7 - c - 1 = 4  => c = 2
 * 0x1bb      5      7 - d - 1 = 5  => d = 1
 * 0x14c      0      7 - e - 1 = 0  => e = 6
 * 0x0a8      1      7 - f - 1 = 1  => f = 5
 * */
