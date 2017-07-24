r13 = rsp
rsi = rsp

read_six_numbers // start at rsp

r14 = rsp
r12d = 0

rbp = r13 // 0x401114

eax = &r13
eax = eax - 1

if(eax > 5) bomb
else { // 0x401128
    r12d = r12d + 1
    if (r12d != 6) {
        ebx = r12d
        eax = ebx /*0x401135*/
        eax = &(rsp + 4 * eax)
        
        if (eax == &rbp) bomb
        else { // 0x401145
          ebx = ebx + 1
          if (ebx > 5) {
             r13 = r13 + 4
             //jmp to 0x401114
          } else  // jmp to 0x401135
        }
    } else { // 0x401153
      rsi = rsp + 0x18
      rax = r14
      ecx = 0x7
      edx = ecx /* 0x401160 */
      edx = edx - &rax
      &rax = edx
      rax = rax + 4
      if(rax == rsi) {
        esi = 0
        // jmp to 0x401197
      } else // jump to 0x401160
    }
}

/* 0x401197 */
ecx = &(rsp + rsi)
ecx = exc + 1

if(ecx > 1) {
  eax = 0x1
  edx = 0x6032d0

  /* 0x401176 */
  rdx = &(rdx + 8)
  eax = eax + 1 
  if (ecx != eax) // jmp to 0x401176
  else // jmp to 0x401188

} else { // 0x401183
  edx = 0x6032d0
  &(rsp + 2 * rsi + 0x20) = rdx // 0x401188
  rsi = rsi + 4
  if (rsi != 0x18) {
      // jmp to 0x401197
  } else // jmp to 0x4011ab
}

/* 0x4011ab */

rbx = &(rsp + 0x20)
rax = rsp + 0x28
rsi = rsp + 0x50
rcx = rbx
rdx = &rax // 0x4011bd
&(rcx + 8) = rdx
rax = rax + 8
if (rax == rsi) // jmp to 0x4011d2
else {
  rcx = rdx
  // jmp to 0x4011bd
}

/* 0x4011d2 */
(rdx + 0x8) = 0
ebp = 0x5
rax = &(rbx + 0x8) // 0x4011df
eax = &rax

if(&rbx < eax) // bomb
else { 
  rbx = &(rbx + 0x8)
  ebp = ebp - 0x1
  if (ebp != 0) // jmp to 0x4011df
  else // over
}
