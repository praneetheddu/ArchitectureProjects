#pragma once
#include <math.h>
#include "paging.h"

//int offset_bits;
/*  --------------------------------- PROBLEM 1 --------------------------------------
    Split the virtual address into VPN and offset.
    These macros will be used whenever you get a memory address from the CPU and need
    to translate it.

    HINT: Examine the global defines in pagesim.h, which will be
    necessary in implementing these functions.

    HINT: While these values will be powers of two, allowing you to use bit-wise
    arithmetic, consider using modulus division and integer division for
    an elegant solution.
    -----------------------------------------------------------------------------------
 */

/* Get the virtual page number from a virtual address. */
static inline vpn_t vaddr_vpn(vaddr_t addr) {
               /* FIXME */
    //vaddr_t temp_addr;
    addr = addr >> OFFSET_LEN;
    return addr;
}

/* Get the offset into the page from a virtual address. */
static inline uint16_t vaddr_offset(vaddr_t addr) {
        addr = addr % (vpn_t) pow(2, OFFSET_LEN);
    //addr = addr & 0x3FFF;
    //return addr;
    //addr <<= (VADDR_LEN - OFFSET_LEN);
    //addr >>= (VADDR_LEN - OFFSET_LEN);
    return addr;                    /* FIXME */
}