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

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <assert.h>
#include <sched.h>
#include <inttypes.h>
#ifdef __APPLE__
#include <sys/uio.h>
#else
#include <sys/io.h>
#endif
#include <sys/mman.h>
#include <stdbool.h>
#include <limits.h>
#include <errno.h>


#include "nesting.h"
#include "ticket.h"
#include "mem.h"
#include "setup.h"

#include "pftl.h"
#include "pfl.h"

#define GHZ PROC_SPEED

int nr_cpus;
int nr_requested;
int nr_resources;
int cs_length;
int write_prob;

/*
    Algorithms:
      1: pftl
      2: fast read pftl
*/
int alg;
pft_lock_t pftl;
rflock_t rflock;

ticket_lock_t csvlock;
FILE *csvfile;

int shared_objects[sizeof(resource_mask_t)*8];

static volatile int counter = 0;

struct thread_context {
    int id;
    int* objects;
};

//to solve pthread issue
int ret_val;
size_t stack_size_t;
pthread_attr_t attrs;


#ifndef __APPLE__
int migrate_thread_to_cpu(int target_cpu)
{
    cpu_set_t *cpu_set;
    size_t sz;
    int ret;

    cpu_set = CPU_ALLOC(nr_cpus);
    sz = CPU_ALLOC_SIZE(nr_cpus);
    CPU_ZERO_S(sz, cpu_set);
    CPU_SET_S(target_cpu, sz, cpu_set);

    ret = sched_setaffinity(0, sz, cpu_set);

    CPU_FREE(cpu_set);

    return ret;
}
#endif

float random_cs_length(int lmax)
{
    if (lmax==0)
	    return 0; 
    return rand() % lmax;
}

bool is_write(int perc_writes)
{
    if (rand() % 100 < perc_writes) {
        return true;
    } else {
        return false;
    }
}

void* rt_thread_rw(void* args)
{
    pft_lock_t *pftl_self;
    //pftc_lock_t *pftc_self;
    rflock_t *flock_self;	
	
    int i,j,k,l,r;
    struct thread_context* ctx = (struct thread_context*) args;
    resource_mask_t tmp;
   
    uint64_t max_rl = 0, max_ru = 0;
    uint64_t sum_rl = 0, sum_ru = 0;

    uint64_t max_wl = 0, max_wu = 0;
    uint64_t sum_wl = 0, sum_wu = 0;

    
    uint64_t read_lock[BUFSIZE], read_unlock[BUFSIZE];
    uint64_t write_lock[BUFSIZE], write_unlock[BUFSIZE];
    uint64_t write_block[BUFSIZE], read_block[BUFSIZE];

    uint64_t end_time;
    //resource_mask_t requests[TRIALS];
    //float cs_lengths[TRIALS];
    //bool write[TRIALS];
    uint64_t *ret_tmp = NULL;

#ifndef __APPLE__
    migrate_thread_to_cpu(CPU_MAP[ctx->id]);
#endif

    // Determine resources at init time so as not to alter the runtime demand
    // for the lock
    for (i = 0; i < TRIALS; i++)
    {
	//requests[i] = random_resources(nr_requested, nr_resources);
        //cs_lengths[i] = random_cs_length(cs_length);
	//write[i] = is_write(write_prob);
    }

    for (l = 0; l < TRIALS/BUFSIZE; l++)
    {
        __sync_fetch_and_add(&counter, 1);
        while (counter != nr_cpus*(l+1))
        {
            cpu_relax();
        }

	//fence instruction
        mb();
    
        for (i = 0; i < BUFSIZE; i++)
        {
            k = l*BUFSIZE + i;
	    r = rand()% 100 < write_prob;

            switch (alg)
            {
            case 1: // PFTL
		
		if (r) {
			ret_tmp = pft_write_lock(&pftl);
			write_lock[i] = ret_tmp[0];
			write_block[i] = ret_tmp[1];
		} else {
			ret_tmp = pft_read_lock(&pftl);
			read_lock[i] = ret_tmp[0];
			read_block[i] = ret_tmp[1];
		}
                break;

            case 2: // fast reads
		if (r) {
                        ret_tmp = rflock_write_lock(&rflock);
			write_lock[i] = ret_tmp[0];
                        write_block[i] = ret_tmp[1];
                } else {
                        ret_tmp = rflock_read_lock(&rflock, CPU_MAP[ctx->id]);
			read_lock[i] = ret_tmp[0];
                        read_block[i] = ret_tmp[1];
                }
                break;
            }

            //lock[i] = ret_tmp[0];
            //block[i] = ret_tmp[1];

            free(ret_tmp);
            ret_tmp = NULL;

            /*******code for critical section*******/
            /*mb(); //fence instruction
	    if (cs_length != 0) 
	    {
	    	end_time = get_cycles() + (int)(GHZ * cs_length);
            	while (get_cycles() < end_time)
            	{
                	cpu_relax();
            	}
	    }*/
	    /***************************************/
            __sync_synchronize();

            switch (alg)
            {
            case 1: // PFTL
            	if (r) {
                        ret_tmp = pft_write_unlock(&pftl);
                        write_unlock[i] = ret_tmp[0];

                } else {
                        ret_tmp = pft_read_unlock(&pftl);
			read_unlock[i] = ret_tmp[0];
                }
                break;
	    case 2: // FAST READS
            	if (r) {
                        ret_tmp = rflock_write_unlock(&rflock);
			write_unlock[i] = ret_tmp[0];
                } else {
                        ret_tmp = rflock_read_unlock(&rflock, CPU_MAP[ctx->id]);
			read_unlock[i] = ret_tmp[0];
                }
                break;
	    }

            //unlock[i] = ret_tmp[0];

            free(ret_tmp);
            ret_tmp = NULL;
        }

	mb();

        ticket_lock(&csvlock);

        for (i = 0; i < BUFSIZE; i++)
        {
            k = l*BUFSIZE + i;
            
	    fprintf(csvfile, "%d, %d, %d, %d, %d, %d, %" PRIu64 ", %" PRIu64 ", %" PRIu64 ", %" PRIu64 ", %" PRIu64 ", %" PRIu64 "\n",
                    ctx->id, nr_cpus, nr_resources, nr_requested, write_prob, cs_length,
                    read_lock[i], read_unlock[i], read_block[i], write_lock[i], write_unlock[i], write_block[i]);

	    /* don't need this unless we want stats... do this later *
            if(read_lock[i] > max_l)
                max_l = lock[i];

            if(unlock[i] > max_u)
                max_u = unlock[i];
           
            sum_l += lock[i];
            sum_u += unlock[i];
	    */
        }

        ticket_unlock(&csvlock);
    }

	//fprintf(stderr,"Thread %d: max lock %f, max unlock %f\navg lock %f, avg unlock %f\n",
        //    ctx->id, max_l/GHZ, max_u/GHZ, sum_l/(TRIALS*GHZ), sum_u/(TRIALS*GHZ));

    return NULL;
}

int main(int argc, char** argv)
{
    if (argc != 8)
    {
        printf("argc: %d\n",argc);
        fprintf(stderr, "Usage: run cpus nr_requested nr_resources cs_length write_prob alg csvfile\n");
        return 1;
    }

    int i;
    pthread_t *tasks;
    struct thread_context* ctx;

    //to solve pthread issue
    pthread_attr_init(&attrs);
    pthread_attr_setstacksize(&attrs, 2 * PTHREAD_STACK_MIN);
    
    nr_cpus = atoi(argv[1]);
    nr_requested = atoi(argv[2]);
    nr_resources = atoi(argv[3]);
    cs_length = atoi(argv[4]);
    write_prob = atoi(argv[5]);
    alg = atoi(argv[6]);
    csvfile = fopen(argv[7], "w");

    tasks = calloc(nr_cpus, sizeof(pthread_t));
    ctx = malloc(nr_cpus * sizeof(struct thread_context));

    fprintf(csvfile, "thread, numcores, numresources, numrequested, percwrites, cslength, readlock, readunlock, readblock, writelock, writeunlock, writeblock\n");

    ticket_init(&csvlock);

    pft_lock_init(&pftl);
    rflock_init(&rflock, nr_cpus);

#ifdef DISABLE_INTERRUPTS
    iopl(3);
#endif

    for(i = 0; i < nr_cpus; i++)
    {
        ctx[i].id = i;
        ctx[i].objects = shared_objects;
    }

    for(i = 0; i < nr_cpus; i++)
    {
        mb();
        //pthread_create(&tasks[i], NULL, rt_thread, (void *) (&ctx[i]));
        ret_val = pthread_create(&tasks[i], &attrs, rt_thread_rw, (void *) (&ctx[i]));
  	if(ret_val != 0)
      		printf("error: %s\n", strerror(errno));
  	ret_val = pthread_attr_getstacksize(&attrs, &stack_size_t);
  	//printf("stack_size: %ld\n", stack_size_t);
	
	mb();
    }

    for(i = 0; i < nr_cpus; i++)
    {
        pthread_join(tasks[i], NULL);
    }

    return 0;
}

