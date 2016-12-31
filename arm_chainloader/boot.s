/*
At this point, the kernel is already ready in memory;
all that is left is setting up some registers and jumping.
However, it's kind of ugly to do it in inline asm.. so here
*/

  /*
   r0: 0
   r1: machine ID (3138/3189 for Pi 1/2)
   r2: DTB address
   r3: kernel address
   */

.globl boot_linux
boot_linux:
    /* jump to kernel */
    // mov pc, r3
    //bx r3
    ldr r3, =0x2000000
    push {lr}
    blx r3
    pop {lr}
    
    bx lr
