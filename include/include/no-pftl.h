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

#ifndef PFT_H
#define PFT_H

#include "mem.h"

// Phase-Fair (ticket) Lock
#define RINC  0x100 // reader increment
#define WBITS 0x3   // writer bits in rin
#define PRES  0x2   // writer present bit
#define PHID  0x1   // writer phase ID bit

typedef struct pft_lock_struct {
    volatile unsigned int rin;
    unsigned int _buf1[15];

    volatile unsigned int rout;
    unsigned int _buf2[15];

    volatile unsigned int win;
    unsigned int _buf3[15];

    volatile unsigned int wout;
    unsigned int _buf4[15];
} __attribute ((aligned (16) )) pft_lock_t;

/*
 *  Phase-Fair (ticket) Lock: initialize.
 */
static inline void pft_lock_init(pft_lock_t *lock)
{
    lock->rin = 0;
    lock->rout = 0;

    lock->win = 0;
    lock->wout = 0;
}

/*
 *  Phase-Fair (ticket) Lock: read lock.
 */
static inline void pft_read_lock(pft_lock_t *lock)
{
    unsigned int w;

    // Increment the rin count and reads the writer bits
    w = __sync_fetch_and_add(&lock->rin, RINC) & WBITS;

    // Spin (wait) if there is a writer present (w != 0), until
    // either PRES and/or PHID flips
    while ((w != 0) && (w == (lock->rin & WBITS)))
    {
        cpu_relax();
    }
}

/*
 *  Phase-Fair (ticket) Lock: read unlock.
 */
static inline void pft_read_unlock(pft_lock_t *lock)
{
    // Increment rout to mark the read-lock returned
    __sync_fetch_and_add(&lock->rout, RINC);
}

/*
 *  Phase-Fair (ticket) Lock: write lock.
 */
static inline void pft_write_lock(pft_lock_t *lock)
{
    unsigned int w, rticket, wticket;

    // Wait until it is my turn to write-lock the resource
    wticket = __sync_fetch_and_add(&lock->win, 1);
    while (wticket != lock->wout)
    {
        cpu_relax();
    }

    // Set the write-bits of rin to indicate this writer is here
    w = PRES | (wticket & PHID);
    rticket = __sync_fetch_and_add(&lock->rin, w);

    // Wait until all current readers have finished (i.e rout
    // catches up)
    while (rticket != lock->rout)
    {
        cpu_relax();
    }
}

/*
 *  Phase-Fair (ticket) Lock: write unlock.
 */
static inline void pft_write_unlock(pft_lock_t *lock)
{
    unsigned int andoperand;

    // Clear the least-significant byte of rin
    andoperand = -256;
    __sync_fetch_and_and(&lock->rin, andoperand);

    // Increment wout to indicate this write has released the lock
    lock->wout++; // only one writer should ever be here
}

#endif // PFT_H

