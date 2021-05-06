#ifndef RFLOCK_H
#define RFLOCK_H

#include "mem.h"
#include "cycles.h"

// Read Fast Phase-Fair (ticket) Lock
#define WINC 0x100
#define WBITS 0x3
#define PRES 0x2
#define PHID 0x1
#define PRESENT 0x3
#define COMPLETED 0x4

#define CORES_MAX 40
int CORES;

// on zildjian cache line is 64 bytes, unsigned int is 4 bytes
typedef struct rflock_struct {
    volatile unsigned int read_status[16*CORES_MAX];
    
    volatile unsigned int win;
    unsigned int _buf1[15];
    
    volatile unsigned int wout;
    unsigned int _buf2[15];
} __attribute ((aligned (16) )) rflock_t;


/*
 *  Fast read Phase-Fair (ticket) Lock: initialize.
 */
static inline void rflock_init(rflock_t *lock, int cores)
{
    CORES = cores;
    for (int i=0; i < CORES ; i++)
    {
	lock->read_status[i*16]=COMPLETED;
    }

    lock->win = 0;
    lock->wout = 0;
    
}

static inline void printlockstate(rflock_t *lock)
{
	printf("lock state: win: %x, wout: %x\n", lock->win, lock->wout);
}

/*
 *  Fast read Phase-Fair (ticket) Lock: read lock.
 */
static inline uint64_t *rflock_read_lock(rflock_t *lock, int core)
{
    uint64_t *ret;
    uint64_t start, end;

    ret = (uint64_t *)malloc(sizeof(uint64_t) * 2);
    if (ret == NULL)
	    printf("malloc on 89 failed");
    
    ret[0] = ret[1] = 0;
    start = get_cycles();

    unsigned int w;
    lock->read_status[16*core] = PRESENT;
    
    mb();
    
    w = lock->win & WBITS;
    lock->read_status[16*core] = w & PHID;

    end = get_cycles();
    ret[0] = end - start;

    start = get_cycles();
    while (((w & PRES) != 0) && (w == (lock->win & WBITS)))
    {
        cpu_relax();
    }
    end = get_cycles();
    ret[1] = end - start;

    return ret;
}

/*
 *  Phase-Fair (ticket) Lock: read unlock.
 */
static inline uint64_t *rflock_read_unlock(rflock_t *lock, int core)
{
    uint64_t *ret;
    uint64_t start, end;

    ret = (uint64_t *)malloc(sizeof(uint64_t) * 2);
    if (ret == NULL)
            printf("malloc on 125 failed");
    ret[0] = ret[1] = 0;

    start = get_cycles();

    lock->read_status[16*core] = COMPLETED;

    end = get_cycles();
    ret[0] = end - start;

    return ret;
}

/*
 *  Phase-Fair (ticket) Lock: write lock.
 */
static inline uint64_t *rflock_write_lock(rflock_t *lock)
{
    uint64_t *ret;
    uint64_t start, end;

    ret = (uint64_t *)malloc(sizeof(uint64_t) * 2);
    if (ret == NULL)
            printf("malloc on 149  failed");

    ret[0] = ret[1] = 0;
    start = get_cycles();

    unsigned int w, wticket, read_waiting;

    // Wait until it is my turn to write-lock the resource
    wticket = __sync_fetch_and_add(&lock->win, WINC) & ~WBITS;
    
    end = get_cycles();
    ret[0] += end - start;

    start = get_cycles();
    while (wticket != lock->wout)
    {
        cpu_relax();
    }
    end = get_cycles();
    ret[1] += end - start; //blocking due to other writes

    start = get_cycles();
    __sync_fetch_and_xor(&lock->win, 0x3);
    read_waiting = lock->win & PHID;

    //initial check to pull read status into cache -- counts as overheads, not blocking
    int present_reads = 0;
    for (int i = 0; i<CORES ; i++)
    {
	//no reads waiting
	if ((lock->read_status[i*16] != read_waiting) && (lock->read_status[i*16] != COMPLETED))
	{
		present_reads = 1;
	}
    }

    end = get_cycles();
    ret[0] += end - start; //overheads for pulling values and some atomics
    if (present_reads == 0) {
       return ret;
    }
    
    start = get_cycles();
    for (int i = 0; i<CORES ; i++)
    {
	while ((lock->read_status[i*16] != read_waiting) && (lock->read_status[i*16] != COMPLETED))
	{
        	cpu_relax();
	}
    }

    end = get_cycles();
    ret[1] += end - start;

    return ret;
}

/*
 *  Phase-Fair (ticket) Lock: write unlock.
 */
static inline uint64_t *rflock_write_unlock(rflock_t *lock)
{
    uint64_t *ret;
    uint64_t start, end;

    ret = (uint64_t *)malloc(sizeof(uint64_t) * 2);
    if (ret == NULL)
            printf("malloc on 200 failed");

    ret[0] = ret[1] = 0;
    start = get_cycles();

    __sync_fetch_and_and(&lock->win, 0xFFFFFF01);
    lock->wout = lock->wout + WINC; // only one writer should ever be here

    end = get_cycles();
    ret[0] = end - start;

    return ret;
}

#endif 

