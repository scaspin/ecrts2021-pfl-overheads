////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2018 The University of North Carolina at Chapel Hill.
// All Rights Reserved.
//
// Permission to use, copy, modify and distribute this software and its
// documentation for educational, research and non-profit purposes, without
// fee, and without a written agreement is hereby granted, provided that the
// above copyright notice and the following three paragraphs appear in all
// copies.
//
// IN NO EVENT SHALL THE UNIVERSITY OF NORTH CAROLINA AT CHAPEL HILL BE
// LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR
// CONSEQUENTIAL DAMAGES, INCLUDING LOST PROFITS, ARISING OUT OF THE
// USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF THE UNIVERSITY
// OF NORTH CAROLINA HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH
// DAMAGES.
//
// THE UNIVERSITY OF NORTH CAROLINA SPECIFICALLY DISCLAIM ANY
// WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE
// PROVIDED HEREUNDER IS ON AN "AS IS" BASIS, AND THE UNIVERSITY OF
// NORTH CAROLINA HAS NO OBLIGATIONS TO PROVIDE MAINTENANCE, SUPPORT,
// UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
//
// The authors may be contacted via:
//
// US Mail: Real-Time Systems Group at UNC
// Department of Computer Science
// Sitterson Hall
// University of N. Carolina
// Chapel Hill, NC 27599-3175
//
// EMail: nemitz@cs.unc.edu; tamert@cs.unc.edu; anderson@cs.unc.edu
//
////////////////////////////////////////////////////////////////////////////////

#ifndef RFLOCK_H
#define RFLOCK_H

#include "mem.h"

// Read Fast Phase-Fair (ticket) Lock
#define WINC 0x100
#define WBITS 0x3
#define PRES 0x2
#define PHID 0x1
#define PRESENT 0x3
#define COMPLETED 0x4

#define CORES_MAX 40
int CORES;

typedef struct rflock_struct {
    volatile unsigned read_status[CORES_MAX * 16];
    
    volatile unsigned win;
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
    for (int i=0; i < CORES; i++)
    {
	lock->read_status[i*16]= COMPLETED;
    }

    lock->win = 0;
    lock->wout = 0;
}

/*
 *  Fast read Phase-Fair (ticket) Lock: read lock.
 */
static inline void rflock_read_lock(rflock_t *lock, int core)
{
    unsigned int w;
    lock->read_status[core*16] = PRESENT;
    w = lock->win & WBITS;
    lock->read_status[core*16] = w & PHID;

    while (((w & PRES) != 0) && (w == (lock->win & WBITS)))
    {
        cpu_relax();
    }
}

/*
 *  Phase-Fair (ticket) Lock: read unlock.
 */
static inline void rflock_read_unlock(rflock_t *lock, int core)
{
    lock->read_status[core*16] = COMPLETED;
}

/*
 *  Phase-Fair (ticket) Lock: write lock.
 */
static inline void rflock_write_lock(rflock_t *lock)
{
    unsigned int w, wticket, read_waiting;

    // Wait until it is my turn to write-lock the resource
    wticket = __sync_fetch_and_add(&lock->win, WINC) & ~WBITS;
    while (wticket != lock->wout)
    {
        cpu_relax();
    }

    __sync_fetch_and_xor(&lock->win, WBITS);
    read_waiting = lock->win & PHID;

    for (int i = 0; i<CORES ; i++)
    {
	while ((lock->read_status[i*16] != read_waiting) && (lock->read_status[i*16] != COMPLETED))
	{
		cpu_relax();
	}
    }
}

/*
 *  Phase-Fair (ticket) Lock: write unlock.
 */
static inline void rflock_write_unlock(rflock_t *lock)
{
    __sync_fetch_and_and(&lock->win, 0xFFFFFF01);
    lock->wout = lock->wout + WINC; // only one writer should ever be here
}

#endif 

