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

#ifndef TICKET_H 
#define TICKET_H

#include "mem.h"

// Ticket (spin) lock

typedef struct
{
    volatile unsigned long serving;
    volatile unsigned long next;
} ticket_lock_t;

/*
 *  Ticket (spin) lock: initialize.
 */
static inline void ticket_init(ticket_lock_t* lock)
{
    lock->serving = 0;
    lock->next = 0;
}

/*
 *  Ticket (spin) lock: lock the ticket lock by taking a ticket
 *  and waiting until that number is being served.
 */
static inline void ticket_lock(ticket_lock_t* lock)
{
    int ticket = __sync_fetch_and_add(&(lock->next), 1);
    mb();
    while(lock->serving != ticket)
    {
        cpu_relax();
    }
    mb();
}

/*
 *  Ticket (spin) lock: unlock the ticket by incrementing
 *  the ticket number that is being served.
 */
static inline void ticket_unlock(ticket_lock_t* lock)
{
    mb();
    lock->serving += 1;
    mb();
}

#endif // TICKET_H

