rbx = rdi;

/*
 * input is a string containing 6 characters [a, b, c, d, e, f]. 
 * */
if(string_length(input) != 6 ) bomb();
else {
    eax = 0;
    while(rax != 6) { // 40108b
        ecx = &(rbx + rax);
        rdx = %cl; // last byte of each char

        edx &= 0xf;
        edx = &(rdx + 0x4024b0); 
        
        /* offset = last four bits of each char 
         * 
         * base = 0x4024b0 =>
         * 
         * use these offsets to choose (0x40245e:  "flyers") from
         * "maduiersnfotvbylSo you think you can stop the bomb with ctrl-c, do you?" 
         *
         * So:
         * 
         * offset       char   
         *   9           f        a & 0xf = 9
         *   F           l        b & 0xf = F
         *   E           y        c & 0xf = E
         *   5           e        d & 0xf = 5
         *   6           r        e & 0xf = 6
         *   7           s        f & 0xf = 7
         * 
         * */
        


        &(rsp + 10 + rax) = dl;
        rax += 1;
    } 
    


    &(rsp + 16) = 0; // '\0'
    if(strings_not_equal(rsp + 10, 0x40245e)) bomb(); // 0x40245e:  "flyers"
}
