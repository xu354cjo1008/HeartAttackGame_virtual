//
//  main.c
//  VirtualClock
//
//  Created by 李勁璋 on 2013/12/25.
//  Copyright (c) 2013年 李勁璋. All rights reserved.
//

#include <stdio.h>
#include "virtualClock.h"
#ifdef __MACH__
#include <mach/clock.h>
#include <mach/mach.h>
#endif
#define MAX_ADJ 500000
struct virClock *virClock;
const int onesPeriod = 100;
int monitorPeriod = onesPeriod;
int main(int argc, const char * argv[])
{
    printf("hello world\n");
    // insert code here...
    TimeInternal ts;
    ts.seconds = 0;
    ts.nanoseconds = 10000000;
    virClock = virClock_create(ts, TRUE, MAX_ADJ);

    while (TRUE) {
        monitor();
    }
    virClock_destroy(virClock);
    
    return 0;
}

