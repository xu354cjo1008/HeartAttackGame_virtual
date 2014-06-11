//
//  virtualClock.c
//  VirtualClock
//
//  Created by 李勁璋 on 2013/12/25.
//  Copyright (c) 2013年 李勁璋. All rights reserved.
//


#include <stdio.h>
#include <stdlib.h>
#include "virtualClock.h"
#include "signal.h"
#include "contain.h"
#ifdef __MACH__
#include <mach/clock.h>
#include <mach/mach.h>
#endif


#define TIMER_INTERVAL 10000
volatile unsigned int virelapsed;

static struct virClock *virClock;

void
vircatch_alarm(int sig)
{
	virelapsed++;
    if (!virClock)
        return;
    timespec_add_ns(&virClock->virTime, tmv_to_nanoseconds(virClock->virClockPeriod));
}

void
initVirTimer(Boolean isInternalTimer)
{
	//elapsed = 0;
    if (isInternalTimer == TRUE) {
        struct itimerval itimer;
        signal(SIGALRM, SIG_IGN);
        itimer.it_value.tv_sec = itimer.it_interval.tv_sec = 0;
        itimer.it_value.tv_usec = itimer.it_interval.tv_usec = TIMER_INTERVAL;
        signal(SIGALRM, vircatch_alarm);
        int res = 0;
        res = setitimer(ITIMER_REAL, &itimer, NULL);
        if (res) {
            printf("Set timer failed!!\n");
        }
    } else {
        //signal(SIGALRM, vircatch_alarm);
        printf("Use outside timer!!\n");
    }
}

struct virClock *virClock_create()
{
    virClock = calloc(1, sizeof(*virClock));
	if (!virClock)
		return NULL;
    
	return virClock;
}

void virClock_run(struct virClock *virClock, TimeInternal period, Boolean isInternalTimer,int max_adj,int syncInterval)
{
    clock_serv_t cclock;
    mach_timespec_t mts;
    host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &cclock);
    clock_get_time(cclock, &mts);
    mach_port_deallocate(mach_task_self(), cclock);
    
    struct timespec ts;
    ts.tv_sec = mts.tv_sec;
    ts.tv_nsec = mts.tv_nsec;
    
    virClock->servo = servo_create(CLOCK_SERVO_PI, 0, max_adj, 1);
    if (!virClock->servo) {
		printf("Failed to create clock servo");
	}
    servo_sync_interval(virClock->servo, syncInterval < 0 ? 1.0 / (1 << -syncInterval) : 1 << syncInterval);

	virClock->servo_state = SERVO_UNLOCKED;
    
    virClock->virTime = ts;
    virClock->virClockPeriod = timeinternal_to_tmv(period);
    virClock->isInternalTimer = isInternalTimer;
    
    initVirTimer(virClock->isInternalTimer);
}

void virClock_destroy(struct virClock * virClock)
{
    struct itimerval itimer;
    if (virClock->isInternalTimer == TRUE) {
        itimer.it_value.tv_sec = itimer.it_interval.tv_sec = 0;
        itimer.it_value.tv_usec = itimer.it_interval.tv_usec = 0;
        setitimer(ITIMER_REAL, &itimer, NULL);
    }
    signal(SIGPIPE, SIG_DFL);
    servo_destroy(virClock->servo);
	free(virClock);
}

void printVirTime(struct virClock * virClock)
{
#define MAXTIMESTR 32
    char time_str[MAXTIMESTR];
    time_t time_s;
    time_s = virClock->virTime.tv_sec;
    strftime(time_str, MAXTIMESTR, "%Y-%m-%d %X", localtime(&time_s));
    printf("%s.%06d\n",time_str,(int)virClock->virTime.tv_nsec/1000);
}

void monitor(struct virClock * virClock)
{
#define MAXTIMESTR 32
    extern int monitorPeriod;
    extern int onesPeriod;
    if (virelapsed >= monitorPeriod) {
        printVirTime(virClock);
        monitorPeriod = virelapsed + onesPeriod;
    }
}


void virClock_Update(struct virClock * virClock)
{
    if (!virClock)
        return;
    timespec_add_ns(&virClock->virTime, tmv_to_nanoseconds(virClock->virClockPeriod));
    
#define MAXTIMESTR 32
    char time_str[MAXTIMESTR];
    time_t time_s;
    time_s = virClock->virTime.tv_sec;
    strftime(time_str, MAXTIMESTR, "%Y-%m-%d %X", localtime(&time_s));
    printf("%s.%06d\n",time_str,(int)virClock->virTime.tv_nsec/1000);
}

void adjVirTime(struct virClock * virClock,double freq)
{
    double adjPeriod = freq*0.000000001*tmv_dbl(virClock->virClockPeriod);
    virClock->virClockPeriod = virClock->virClockPeriod + (int64_t)adjPeriod;
   // printf("%f\n",freq);
    printf("%11lld\n",(int64_t)adjPeriod);
}

void setVirTime(struct virClock * virClock,struct timespec ts)
{
    virClock->virTime = ts;
}

void stepVirTime(struct virClock * virClock, tmv_t ts)
{
    //timespec_add_ns(&virClock->virTime, tmv_to_nanoseconds(ts));
    struct timespec x = tmv_to_timespec(ts);
    virClock->virTime.tv_sec = virClock->virTime.tv_sec + x.tv_sec;
    virClock->virTime.tv_nsec = virClock->virTime.tv_nsec + x.tv_nsec;
    if (virClock->virTime.tv_nsec >= 1000000000) {
        virClock->virTime.tv_nsec = virClock->virTime.tv_nsec - 1000000000;
        virClock->virTime.tv_sec ++;
    } else if (virClock->virTime.tv_nsec < 0) {
        virClock->virTime.tv_nsec = virClock->virTime.tv_nsec + 1000000000;
        virClock->virTime.tv_sec --;
    }
}

void getVirTime(struct virClock *virClock, TimeInternal *time)
{
    time->seconds = virClock->virTime.tv_sec;
    time->nanoseconds = virClock->virTime.tv_nsec;
}
void getVirTimeOffsetFromSystem(struct virClock *virClock, TimeInternal *time)
{
    clock_serv_t cclock;
    mach_timespec_t mts;
    host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &cclock);
    clock_get_time(cclock, &mts);
    mach_port_deallocate(mach_task_self(), cclock);
    
    struct timespec ts;
    ts.tv_sec = mts.tv_sec;
    ts.tv_nsec = mts.tv_nsec;
    
    struct timespec timeoffset;
    timeoffset.tv_sec = virClock->virTime.tv_sec - ts.tv_sec;
    timeoffset.tv_nsec = virClock->virTime.tv_nsec - ts.tv_nsec;
    
    time->seconds = timeoffset.tv_sec;
    time->nanoseconds = timeoffset.tv_nsec;
}

void systemtimeToVir(struct virClock *virClock, TimeInternal *time)
{
    TimeInternal *timeoffsetFromSys = NULL;
    getVirTimeOffsetFromSystem(virClock,timeoffsetFromSys);
    time->seconds = time->seconds + timeoffsetFromSys->seconds;
    time->nanoseconds = time->nanoseconds + timeoffsetFromSys->nanoseconds;
}

enum servo_state updateVirClock(struct virClock * virClock, TimeInternal offset_t,TimeInternal ingress_ts)
{
 //
    double adj;
	tmv_t ingress;
    tmv_t offset;
	enum servo_state state = SERVO_UNLOCKED;
    
    ingress = timeinternal_to_tmv(ingress_ts);
    offset = timeinternal_to_tmv(offset_t);

    adj = servo_sample(virClock->servo, tmv_to_nanoseconds(offset),
                       tmv_to_nanoseconds(ingress), &state);
    virClock->servo_state = state;
    switch (state) {
        case SERVO_UNLOCKED:
            break;
        case SERVO_JUMP:
            adjVirTime(virClock, -adj);
            stepVirTime(virClock, -offset);
        //    c->t1 = tmv_zero();
         //   c->t2 = tmv_zero();
            break;
        case SERVO_LOCKED:
            adjVirTime(virClock, -adj);
       //     if (c->clkid == CLOCK_REALTIME)
        //        sysclk_set_sync();
            break;
	}
	return state;

}

