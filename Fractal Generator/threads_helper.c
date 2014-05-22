//
//  threads_helper.c
//  Fractal Generator
//
//  Created by Juan Miguel Rubio on 22/05/14.
//  Copyright (c) 2014 Juan Miguel Rubio. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "threads_helper.h"

int get_max_cpus ()
{
    long nprocs = -1;
    long nprocs_max = -1;
    
#ifdef _WIN32
#ifndef _SC_NPROCESSORS_ONLN
    SYSTEM_INFO info;
    GetSystemInfo(&info);
#define sysconf(a) info.dwNumberOfProcessors
#define _SC_NPROCESSORS_ONLN
#endif
#endif
#ifdef _SC_NPROCESSORS_ONLN
    nprocs = sysconf(_SC_NPROCESSORS_ONLN);
    if (nprocs < 1)
    {
        fprintf(stderr, "Could not determine number of CPUs online:\n%s\n",
                strerror (errno));
        exit (EXIT_FAILURE);
    }
    nprocs_max = sysconf(_SC_NPROCESSORS_CONF);
    if (nprocs_max < 1)
    {
        fprintf(stderr, "Could not determine number of CPUs configured:\n%s\n",
                strerror (errno));
        exit (EXIT_FAILURE);
    }
    printf ("%ld of %ld processors online\n", nprocs, nprocs_max);
#else
    fprintf(stderr, "Could not determine number of CPUs\n");
    exit (EXIT_FAILURE);
#endif
    
    return (int) nprocs;
};