//
//  virtualClock.h
//  VirtualClock
//
//  Created by 李勁璋 on 2013/12/25.
//  Copyright (c) 2013年 李勁璋. All rights reserved.
//

#ifndef VirtualClock_virtualClock_h
#define VirtualClock_virtualClock_h
#include <sys/time.h>
#include <time.h>
#include "virservo.h"
//#include "tmv.h"

#include "tmv.h"

//extern struct virClock *virClock;

struct virClock {
    struct timespec virTime;
   // tmv_t virTime;
    tmv_t virClockPeriod;
    Boolean isInternalTimer;
	struct servo *servo;
	enum servo_state servo_state;
};

struct virClock *virClock_create();

void virClock_run(struct virClock * virClock,TimeInternal period, Boolean isInternalTimer,int max_adj,int syncInterval);

void virClock_destroy(struct virClock * virClock);

void printVirTime(struct virClock * virClock);

void monitor(struct virClock * virClock);

void virClock_Update(struct virClock * virClock);

void getVirTime(struct virClock *virClock, TimeInternal *time);

void getVirTimeOffsetFromSystem(struct virClock *virClock, TimeInternal *time);

void systemtimeToVir(struct virClock *virClock, TimeInternal *time);



#endif
