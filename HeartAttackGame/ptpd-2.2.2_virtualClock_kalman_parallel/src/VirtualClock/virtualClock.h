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
#include "kalmanFilter.h"
#include "tmv.h"

//extern struct virClock *virClock;

#define preEstFreqLength 15
#define varianceEstLength 30

struct virClock {
    struct timespec virTime;
   // tmv_t virTime;
    tmv_t virClockPeriod;
    tmv_t nominalVirClockPeriod;
    tmv_t oringialVirClockPeriod;
    Boolean isInternalTimer;
    struct kalmanFilter *kalmanFilter;
	struct servo *servo;
	enum servo_state servo_state;
    int adjCount;
    int64_t adjPeriod;
    double tickadj;
    double lastadj;
    TimeInternal filteredOffset;
    double filteredFreqError;
    
    double preEstFreq[preEstFreqLength];
    
    double sysOffsetBuf[varianceEstLength];
    double timeVariance;
    int timeVarEstState;
};

struct virClock *virClock_create();

void virClock_run(struct virClock * virClock,TimeInternal period, Boolean isInternalTimer,int max_adj,int syncInterval);

void virClock_destroy(struct virClock * virClock);

void printVirTime(struct virClock * virClock);

void monitor(struct virClock * virClock);

void virClock_Update(struct virClock * virClock);

void adjVirTime(struct virClock * virClock,double freq);

void stepVirTime(struct virClock * virClock, tmv_t ts);


void getVirTime(struct virClock *virClock, TimeInternal *time);

void getVirTimeOffsetFromSystem(struct virClock *virClock, TimeInternal *time);

void systemtimeToVir(struct virClock *virClock, TimeInternal *time);
void virtimeToSystem(struct virClock *virClock, TimeInternal *time);
void virOffsetToSystem(struct virClock *virClock, TimeInternal *offset, TimeInternal *sysOffset);



enum servo_state updateVirClock(struct virClock * virClock, TimeInternal offset_t,TimeInternal ingress_ts, TimeInternal offset_sys);

#endif
