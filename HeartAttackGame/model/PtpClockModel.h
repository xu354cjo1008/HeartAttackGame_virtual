//
//  PtpClockModel.h
//  HeartAttack
//
//  Created by 李勁璋 on 13/10/19.
//  Copyright (c) 2013年 李勁璋. All rights reserved.
//

#import <Foundation/Foundation.h>
#define NANOPERSECOND 1000000000
@interface PtpClockModel : NSObject

-(void) startTheClock:(int) mode;
-(void) stopTheClock;

-(NSString *)monitor;


@property (readonly, nonatomic) int offset_Second;
@property (readonly, nonatomic) int offset_Nanosecond;
@property (readonly, nonatomic) struct timespec virtual_time;
@property (readonly, nonatomic) unsigned char portState;
@property (readonly, nonatomic) BOOL isClockOn;


@end
