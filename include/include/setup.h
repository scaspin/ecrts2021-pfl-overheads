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

#ifndef __SETUP_H__
#define __SETUP_H__

// TODO: Set the total number of physical cores on the machine
#define TOTAL_CORES 8

// TODO: Set the number of sockets to be used in experiments (tested up to 2)
#define NUM_SOCKETS 2

// TODO: Set the processor speed in MHz
#define PROC_SPEED 2297.579

// TODO: Set the CPU map based on the system architecture (see the readme)
static int CPU_MAP[TOTAL_CORES] = {
    // Socket 0
    //0,2,4,6,8,10,12,14,16,18,20,22,24,26,28,30,32,34,
    // Socket 1
    //1,3,5,7,9,11,13,15,17,19,21,23,25,27,29,31,33,35
//};
0,2,4,6,1,3,5,7};

// ludwig socket 0
//0,4,8,12,16,20
// ludwig socket 1
//1,5,9,13,17,21
// ludwig socket 2
//2,6,10,14,18,22
// ludwig socket 3
//3,7,11,15,19,23

// TODO: Set the total number of trials to run in each experiment
#define TRIALS 10000

// Derived parameters are set here
#define BUFSIZE          (TRIALS / 100)
#define CORES_PER_SOCKET (TOTAL_CORES / NUM_SOCKETS)
#define NUM_SERVERS      NUM_SOCKETS
#define QUEUE_LENGTH     TOTAL_CORES
#define SCHEDSIZE        TOTAL_CORES

#endif // __SETUP_H__
