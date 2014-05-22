//
//  threads_helper.h
//  Fractal Generator
//
//  Created by Juan Miguel Rubio on 22/05/14.
//  Copyright (c) 2014 Juan Miguel Rubio. All rights reserved.
//

#ifndef Fractal_Generator_threads_helper_h
    #define Fractal_Generator_threads_helper_h
#endif

#ifdef _WIN32
    #include <windows.h>
#else
    #include <unistd.h>
#endif

int get_max_cpus ();