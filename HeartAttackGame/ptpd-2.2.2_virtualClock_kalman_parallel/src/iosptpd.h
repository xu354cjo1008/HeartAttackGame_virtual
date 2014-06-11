//
//  iosptpd.h
//  iosptpd2
//
//  Created by 李勁璋 on 13/9/17.
//  Copyright (c) 2013年 李勁璋. All rights reserved.
//

#ifndef iosptpd2_iosptpd_h
#define iosptpd2_iosptpd_h
#include "ptpd.h"
#include "VirtualClock/virtualClock.h"
extern int ptpdmain(int argc, char **argv);
extern PtpClock *G_ptpClock;
extern struct virClock *G_virtualClock;

extern int ptpd_shutdown_flag;
extern RunTimeOpts rtOpts;
extern TimeInternal *G_offsetFromMaster;
extern struct timespec *G_virtualTime;

extern unsigned char *G_portState;

#endif
