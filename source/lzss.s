@ Code by mtheall

.arch armv5te

.arm
.align 2
.global lzssDecompress
.hidden lzssDecompress
.type lzssDecompress, %function
lzssDecompress:
  push   {r4-r6}
  ldr    r3, =(0x01010101)

.Lloop:
  cmp    r2, #0              @ if(size <= 0)
  pople  {r4-r6}             @   pop stack
  bxle   lr                  @   return

  rors   r3, r3, #1          @ r3 = (r3<<31) | (r3>>1)
  ldrcsb r12, [r0], #1       @ if(r3 & (1<<31)) flags = *in++
  tst    r12, r3             @ if(flags & r3 == 0)
  beq    .Lcopy_uncompressed @   goto copy_uncompressed

  ldrb   r4, [r0], #1        @ r4 = *in++
  lsr    r6, r4, #4          @ len = r4>>4
  add    r6, r6, #3          @ len += 3
  and    r5, r4, #0x0F       @ disp = r4 & 0x0F      note: disp is in r5
  ldrb   r4, [r0], #1        @ r4 = *in++
  orr    r4, r4, r5, lsl #8  @ disp = r4 | (disp<<8) note: disp changes to r4
  add    r4, r4, #1          @ disp++
  sub    r2, r2, r6          @ size -= len
  tst    r4, #1              @ if(r4 & 1 == 0) // aligned displacement
  beq    .Lcopy_aligned      @   goto copy_aligned

.Lcopy_compressed:
  ldrb   r5, [r1, -r4]       @ r5 = *(out - disp)
  subs   r6, r6, #1          @ --len
  strb   r5, [r1], #1        @ *out++ = r5
  bne    .Lcopy_compressed   @ goto copy_compressed
  b      .Lloop              @ if(len == 0) goto loop

.Lcopy_aligned:
  tst    r1, #0x1            @ if(r1 & 0x1 == 0) // src/dst is aligned
  beq    .Lcopy_hwords       @   goto copy_hwords
  ldrb   r5, [r1, -r4]       @ r5 = *(out - disp) // read a byte to align
  sub    r6, r6, #1          @ len--
  strb   r5, [r1], #1        @ *out++ = r5
.Lcopy_hwords:
  subs   r6, r6, #2          @ len -= 2
  ldrgeh r5, [r1, -r4]       @ if(len >= 0) r5 = *(short*)(out - disp)
  strgeh r5, [r1], #2        @ if(len >= 0) *(short*)out++ = r5
  bgt    .Lcopy_hwords       @ if(len > 0) goto copy_hwords
  beq    .Lloop              @ if(len == 0) goto loop
@ extra byte
  ldrb   r5, [r1, -r4]       @ r5 = *(out - disp)
  strb   r5, [r1], #1        @ *out++ = r5
  b      .Lloop              @ goto loop

.Lcopy_uncompressed:
  ldrb   r4, [r0], #1        @ r4 = *in++
  sub    r2, r2, #1          @ size--
  strb   r4, [r1], #1        @ *out++ = r4
  b      .Lloop              @ goto loop
