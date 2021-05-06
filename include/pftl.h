/* 
 * Code taken from Bjorn Brandenburg's dissertation found here: https://www.cs.unc.edu/~bbb/diss/ with modifications to allow for overhead measurements
 */

#ifndef PFT_H
#define PFT_H

#include <linux/types.h>
#include "mem.h"
#include "cycles.h"

typedef __u32 u32;
static __inline__ u32 xadd32(u32 i, volatile u32* mem)
{
	u32 inc = i;
	__asm__ __volatile__(
		"lock; xaddl %0, %1"
		:"+r" (i), "+m" (*mem)
		: : "memory");
	return i + inc;
}


// Phase-Fair (ticket) Lock
#define RINC  0x100 // reader increment
#define WBITS 0x3   // writer bits in rin
#define PRES  0x2   // writer present bit
#define PHID  0x1   // writer phase ID bit

typedef struct pft_lock_struct {
    
/*    volatile u32 win, wout;
    unsigned int _buf1[15];

    volatile u32 rin, rout;
    unsigned int _buf2[15];
*/

/*  volatile u32 win;
    u32 _b1;
    unsigned int _buf1[15];

    volatile u32 wout;
    u32 _b2;
    unsigned int _buf2[15];

    
    volatile u32 rin;
    u32 _b3;
    unsigned int _buf3[15];

    volatile u32 rout;
    u32 _b4;
    unsigned int _buf4[15];
*/

   volatile u32 win, rin;
   volatile u32 wout, rout;
   unsigned int _butf1[14];

} __attribute ((aligned (16) )) pft_lock_t;

/*
 *  Phase-Fair (ticket) Lock: initialize.
 */
static inline void pft_lock_init(pft_lock_t* lock)
{
    lock->rin = 0;
    lock->rout = 0;

    lock->win = 0;
    lock->wout = 0;
}

/*
 *  Phase-Fair (ticket) Lock: read lock.
 */
static inline uint64_t *pft_read_lock(pft_lock_t *lock)
{
    uint64_t *ret;
    uint64_t start, end;
	
    ret = (uint64_t *)malloc(sizeof(uint64_t) * 2);
    ret[0] = ret[1] = 0;

    start = get_cycles();

    unsigned int w;

    // Increment the rin count and reads the writer bits
    //w = __sync_fetch_and_add(&lock->rin, RINC) & WBITS;

    u32 blocked = xadd32(4, &lock->rin) & 0x3;


    // Spin (wait) if there is a writer present (w != 0), until
    // either PRES and/or PHID flips

    end = get_cycles();
    ret[0] = end - start;
    start = get_cycles();

    /*
    while ((w != 0) && (w == (lock->rin & WBITS)))
    {
        cpu_relax();
    }*/

    while (blocked && ((lock->rin & 0x3) == blocked))
	cpu_relax();

    end = get_cycles();
    ret[1] += end - start;

    return ret;
}

/*
 *  Phase-Fair (ticket) Lock: read unlock.
 */
static inline uint64_t *pft_read_unlock(pft_lock_t *lock)
{
    uint64_t *ret;
    uint64_t start, end;

    ret = (uint64_t *)malloc(sizeof(uint64_t) * 2);
    ret[0] = ret[1] = 0;

    start = get_cycles();
    // Increment rout to mark the read-lock returned
    //__sync_fetch_and_add(&lock->rout, RINC);

    xadd32(4, &lock->rout);

    end = get_cycles();
    ret[0] += end - start;

    return ret;
}

/*
 *  Phase-Fair (ticket) Lock: write lock.
 */
static inline uint64_t *pft_write_lock(pft_lock_t *lock)
{

    uint64_t *ret;
    uint64_t start, end;

    ret = (uint64_t *)malloc(sizeof(uint64_t) * 2);
    ret[0] = ret[1] = 0;

    start = get_cycles();

    /*
    unsigned int w, rticket, wticket;

    // Wait until it is my turn to write-lock the resource
    wticket = __sync_fetch_and_add(&lock->win, 1);
    */
	
    u32 ticket;
    ticket = xadd32(1, &lock->win) - 1;

    if (ticket != lock->wout){

    }

    end = get_cycles();
    ret[0] += end - start;

    start = get_cycles();
    /*
    while (wticket != lock->wout)
    {
        cpu_relax();
    }*/

    while (ticket != lock->wout)  {
	cpu_relax();
    }

    end = get_cycles();
    ret[1] += end - start;
    mb(); 

    start = get_cycles();

    // Set the write-bits of rin to indicate this writer is here
    /*w = PRES | (wticket & PHID);
    rticket = __sync_fetch_and_add(&lock->rin, w);


    // include first pull as overhead, not blocking to mirror fast reads impl
    int reads_present = 0;
    if (rticket != lock->rout)
    {
	reads_present = 1;
    }*/

    ticket = xadd32(0x2 | (ticket & 0x1), &lock->rin) & (~(u32)3);
    int reads = 0;
    if (ticket == lock->rout)
	reads = 1;

    end = get_cycles();
    ret[0] += end - start;
    start = get_cycles();

    /*
    while (rticket != lock->rout)
    {
        cpu_relax();
    }*/

    while (ticket != lock->rout)
	cpu_relax();

    end = get_cycles();
    ret[1] += (end - start);

    return ret;
}

/*
 *  Phase-Fair (ticket) Lock: write unlock.
 */
static inline uint64_t *pft_write_unlock(pft_lock_t *lock)
{
    uint64_t *ret;
    uint64_t start, end;

    ret = (uint64_t *)malloc(sizeof(uint64_t) * 2);
    ret[0] = ret[1] = 0;

    start = get_cycles();

    /*
    unsigned int andoperand;

    // Clear the least-significant byte of rin
    andoperand = -256;
    __sync_fetch_and_and(&lock->rin, andoperand);

    // Increment wout to indicate this write has released the lock
    lock->wout++; // only one writer should ever be here
*/
    u32 ticket = lock->wout;
    xadd32((u32) -(0x2 | (ticket & 0x1)), &lock->rin);
    lock->wout++;

    end = get_cycles();
    ret[0] = end - start;

    return ret;
}

#endif // PFT_H

