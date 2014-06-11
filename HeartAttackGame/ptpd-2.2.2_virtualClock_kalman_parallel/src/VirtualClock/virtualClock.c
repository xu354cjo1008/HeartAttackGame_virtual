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


#define TIMER_INTERVAL 1000
#define TICKADJ 30000
#define ADJUSTMENTINTERVAL 1000000000

#define NS_PER_SEC 1000000000LL

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
 ///kalman filter///////////////////////////////////////////////
    virClock->kalmanFilter = kalmanFilter_create();
    kalmanInitialization(virClock->kalmanFilter,0,0);
    if (!virClock->kalmanFilter) {
		printf("Failed to create clock filter");
	}
////////////////////////////////////////////////////////////////
    virClock->virTime = ts;
    virClock->oringialVirClockPeriod = timeinternal_to_tmv(period);
    virClock->nominalVirClockPeriod = timeinternal_to_tmv(period);
    virClock->virClockPeriod = timeinternal_to_tmv(period);
    virClock->isInternalTimer = isInternalTimer;
    virClock->tickadj = TICKADJ;
    
    initVirTimer(virClock->isInternalTimer);
    
    virClock->filteredOffset.seconds = 0;
    virClock->filteredOffset.nanoseconds = 0;
    virClock->filteredFreqError = 0;
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
    int64_t adjPeriod = (int64_t)(freq*virClock->nominalVirClockPeriod/NS_PER_SEC);
    virClock->adjPeriod = adjPeriod;
    virClock->adjCount = 0;
    
   virClock->virClockPeriod = virClock->nominalVirClockPeriod + (int64_t)virClock->adjPeriod;
}

void adjNominalVirTime(struct virClock * virClock,double freq)
{
    int64_t adjPeriod = (int64_t)(freq*virClock->oringialVirClockPeriod/NS_PER_SEC);
    virClock->adjPeriod = adjPeriod;
    virClock->adjCount = 0;
    
    virClock->nominalVirClockPeriod = virClock->oringialVirClockPeriod + (int64_t)virClock->adjPeriod;
    virClock->virClockPeriod = virClock->nominalVirClockPeriod;
}


double adjVirTimeNTP(struct virClock * virClock,double freq)
{
    double amortization_rate = virClock->tickadj*NS_PER_SEC/virClock->nominalVirClockPeriod; ///ppb
    double delta_t = 0;
    
    if (virClock->adjCount) {
        virClock->virClockPeriod = virClock->nominalVirClockPeriod;
        virClock->adjCount = 0;
    }
    

    if (freq > 0) {
        virClock->adjPeriod = virClock->tickadj;
        delta_t = freq/(amortization_rate/ADJUSTMENTINTERVAL);
        virClock->adjCount = delta_t/virClock->nominalVirClockPeriod;
    } else if (freq < 0) {
        virClock->adjPeriod = -virClock->tickadj;
        delta_t = -freq/(amortization_rate/ADJUSTMENTINTERVAL);
        virClock->adjCount = delta_t/virClock->nominalVirClockPeriod;
    } else {
        virClock->adjPeriod = 0;
        delta_t = 0;
        virClock->adjCount = 0;
    }
  //  printf("%f\n",amortization_rate);
  //  printf("%d\n",virClock->adjCount);

    virClock->virClockPeriod = virClock->nominalVirClockPeriod + (int64_t)virClock->adjPeriod;
    
    if (freq > amortization_rate || freq < -amortization_rate) {
        return amortization_rate;
    } else {
        return freq;
    }
}

void adjVirTimeSwitch(struct virClock * virClock,double freq)
{
    double amortization_rate = virClock->tickadj*NS_PER_SEC/virClock->nominalVirClockPeriod; ///ppb
    double nominalFreqtoOringialFreq = virClock->nominalVirClockPeriod - virClock->oringialVirClockPeriod;
    printf("%f and %f\n",amortization_rate,freq - nominalFreqtoOringialFreq);
    if (freq - nominalFreqtoOringialFreq > 0 && amortization_rate < freq - nominalFreqtoOringialFreq) {
        adjNominalVirTime(virClock,freq - nominalFreqtoOringialFreq);
        printf("adjust nominal\n");
    } else if (freq - nominalFreqtoOringialFreq < 0 && -amortization_rate > freq - nominalFreqtoOringialFreq) {
        adjNominalVirTime(virClock,freq - nominalFreqtoOringialFreq);
        printf("adjust nominal\n");
    } else {
        adjVirTimeNTP(virClock,freq);
    }
}

void adjVirTimeunix(struct virClock * virClock,double offset)
{
    int64_t adjPeriod;
    if (virClock->adjCount) {
        virClock->virClockPeriod = virClock->nominalVirClockPeriod;
        virClock->adjCount = 0;
    }
    if (offset > 0) {
        adjPeriod = virClock->tickadj;
        virClock->adjCount = offset/adjPeriod;
    } else if (offset < 0) {
        adjPeriod = -virClock->tickadj;
        virClock->adjCount = -offset/adjPeriod;
    } else {
        adjPeriod = 0;
        virClock->adjCount = 0;
    }
    virClock->adjPeriod = adjPeriod;
    
    virClock->virClockPeriod = virClock->nominalVirClockPeriod + (int64_t)virClock->adjPeriod;
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
    
    TimeInternal tsa, tempVirtime;
    //TimeInternal *tempVirtime = NULL;
    ts_to_InternalTime(&ts, &tsa);
    ts_to_InternalTime(&virClock->virTime, &tempVirtime);


 //   TimeInternal *timeoffset;

    subTime(time, &tempVirtime, &tsa);
    
  //  time->seconds = timeoffset->tv_sec;
   // time->nanoseconds = timeoffset>tv_nsec;
}

void systemtimeToVir(struct virClock *virClock, TimeInternal *time)
{
    TimeInternal timeoffsetFromSys;
    getVirTimeOffsetFromSystem(virClock,&timeoffsetFromSys);
    addTime(time, time, &timeoffsetFromSys);
    //time->seconds = time->seconds + timeoffsetFromSys->seconds;
    //time->nanoseconds = time->nanoseconds + timeoffsetFromSys->nanoseconds;
}

void virtimeToSystem(struct virClock *virClock, TimeInternal *time)
{
    TimeInternal timeoffsetFromSys;
    getVirTimeOffsetFromSystem(virClock,&timeoffsetFromSys);
    subTime(time, time, &timeoffsetFromSys);
   // time->seconds = time->seconds - timeoffsetFromSys->seconds;
    //time->nanoseconds = time->nanoseconds - timeoffsetFromSys->nanoseconds;
}

void virOffsetToSystem(struct virClock *virClock, TimeInternal *offset, TimeInternal *sysOffset)
{
    TimeInternal timeoffsetFromSys;
    getVirTimeOffsetFromSystem(virClock,&timeoffsetFromSys);
    subTime(sysOffset, offset, &timeoffsetFromSys);
    // time->seconds = time->seconds - timeoffsetFromSys->seconds;
    //time->nanoseconds = time->nanoseconds - timeoffsetFromSys->nanoseconds;
}


enum servo_state updateVirClock(struct virClock * virClock, TimeInternal offset_t, TimeInternal ingress_ts, TimeInternal offset_sys)//,int isFilter)
{
 //
    double adj;
    double offset_in;
	tmv_t ingress;
    tmv_t offset, sysOffset;
	enum servo_state state = SERVO_UNLOCKED;
    
    static int preEstFreqPoint = 0;
    double timeTemp = 0;
    
    ingress = timeinternal_to_tmv(ingress_ts);
    offset = timeinternal_to_tmv(offset_t);
    sysOffset = timeinternal_to_tmv(offset_sys);

    offset_in = kalmanSample(virClock->kalmanFilter, tmv_to_nanoseconds(offset), tmv_to_nanoseconds(ingress));
    
    virClock->filteredOffset = tmv_to_timeinternal((tmv_t)offset_in);
    virClock->filteredFreqError = M(virClock->kalmanFilter->posterioriState,1,0);
 
    M(virClock->kalmanFilter->forceInput,0,0) = 0;
    M(virClock->kalmanFilter->forceInput,1,0) = 0;



      //  adj = servo_sample(virClock->servo, tmv_to_nanoseconds(offset), tmv_to_nanoseconds(ingress), &state);
        adj = servo_sample(virClock->servo, offset_in, tmv_to_nanoseconds(ingress), &state);
    
    virClock->servo_state = state;
    

    switch (state) {
        case SERVO_UNLOCKED:
            virClock->preEstFreq[preEstFreqPoint] = tmv_to_nanoseconds(sysOffset);
            preEstFreqPoint++;
            break;
        case SERVO_JUMP:
            //estimate noise covariance matrix
            for(int j = 0; j < preEstFreqLength - 2; j++)
            {
                timeTemp += (virClock->preEstFreq[j+2] - 2 * virClock->preEstFreq[j+1] + virClock->preEstFreq[j]) * (virClock->preEstFreq[j+2] - 2* virClock->preEstFreq[j+1] + virClock->preEstFreq[j]) / 1e18;
            }
            timeTemp = timeTemp/(2 * (preEstFreqLength - 2));
            timeTemp = timeTemp/3;
            
            printf("%f\n",timeTemp);
           /* M(virClock->kalmanFilter->measurementNoiseCovariance,0,0) = timeTemp/100;
            M(virClock->kalmanFilter->measurementNoiseCovariance,0,1) = 0;
            M(virClock->kalmanFilter->measurementNoiseCovariance,1,0) = 0;
            M(virClock->kalmanFilter->measurementNoiseCovariance,1,1) = timeTemp/100;
            */
            virClock->timeVariance = M(virClock->kalmanFilter->measurementNoiseCovariance,0,0);
            

            adjVirTime(virClock, -adj);
            //adjVirTimeNTP(virClock,-adj);

            stepVirTime(virClock, -offset_in);
            

            M(virClock->kalmanFilter->forceInput,0,0) = -offset_in;
            M(virClock->kalmanFilter->forceInput,1,0) = -adj - virClock->lastadj;

            break;
        case SERVO_CALIBRATION:
            adjNominalVirTime(virClock,-adj);
            break;
        case SERVO_LOCKED:
            /////////////////////////
            ////wait sample
            if(virClock->timeVarEstState < varianceEstLength -1)
            {
                virClock->sysOffsetBuf[virClock->timeVarEstState] = tmv_to_nanoseconds(sysOffset);
                virClock->timeVarEstState ++;
            } else if (virClock->timeVarEstState == varianceEstLength - 1) {
                virClock->sysOffsetBuf[virClock->timeVarEstState] = tmv_to_nanoseconds(sysOffset);
         /*       virClock->timeVariance = (virClock->sysOffsetBuf[2] - 2 * virClock->sysOffsetBuf[1] + virClock->sysOffsetBuf[0]) * (virClock->sysOffsetBuf[2] - 2* virClock->sysOffsetBuf[1] + virClock->sysOffsetBuf[0]) / (1e18 * 2 * (virClock->timeVarEstState + 1 - 2) * 3);*/
                
                for(int j = 0; j < varianceEstLength - 2; j++)
                {
                    timeTemp += (virClock->sysOffsetBuf[j+2] - 2 * virClock->sysOffsetBuf[j+1] + virClock->sysOffsetBuf[j]) * (virClock->sysOffsetBuf[j+2] - 2* virClock->sysOffsetBuf[j+1] + virClock->sysOffsetBuf[j]) / 1e18;
                    timeTemp = timeTemp/(2 * (varianceEstLength - 2));
                    timeTemp = timeTemp/3;

                }
                virClock->timeVariance = timeTemp;
                virClock->timeVarEstState = 0;
                printf("%f\n",virClock->timeVariance);
                M(virClock->kalmanFilter->measurementNoiseCovariance,0,0) = virClock->timeVariance * 100;
                M(virClock->kalmanFilter->measurementNoiseCovariance,0,1) = 0;
                M(virClock->kalmanFilter->measurementNoiseCovariance,1,0) = 0;
                M(virClock->kalmanFilter->measurementNoiseCovariance,1,1) = virClock->timeVariance * 100;

            }/*
            } else {
                virClock->sysOffsetBuf[0] = virClock->sysOffsetBuf[1];
                virClock->sysOffsetBuf[1] = virClock->sysOffsetBuf[2];
                virClock->sysOffsetBuf[2] = tmv_to_nanoseconds(sysOffset);
                virClock->timeVariance = (virClock->sysOffsetBuf[2] - 2 * virClock->sysOffsetBuf[1] + virClock->sysOffsetBuf[0]) * (virClock->sysOffsetBuf[2] - 2* virClock->sysOffsetBuf[1] + virClock->sysOffsetBuf[0]) / (1e18 * 2 * (virClock->timeVarEstState + 1 - 2) * 3) + virClock->timeVariance * (virClock->timeVarEstState - 2) / (virClock->timeVarEstState + 1 - 2);
                virClock->timeVarEstState ++;
            }*/
           // printf("%f\n",virClock->timeVariance);
            //////////////////////////
            adjVirTime(virClock, -adj);
            //adjVirTimeNTP(virClock,-adj);
            
            M(virClock->kalmanFilter->forceInput,0,0) = 0;
            M(virClock->kalmanFilter->forceInput,1,0) = -adj - virClock->lastadj;
            
            break;
	}
    
    virClock->lastadj = -adj;
	return state;

}

