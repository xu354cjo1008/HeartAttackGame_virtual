//
//  PtpClockModel.m
//  HeartAttack
//
//  Created by 李勁璋 on 13/10/19.
//  Copyright (c) 2013年 李勁璋. All rights reserved.
//

#import "PtpClockModel.h"
#import "../ptpd-2.2.2_virtualClock_kalman_parallel/src/iosptpd.h"


@interface PtpClockModel()
@property (readwrite, nonatomic) int offset_Second;
@property (readwrite, nonatomic) int offset_Nanosecond;
@property (readwrite, nonatomic) struct timespec virtual_time;

@property (readwrite, nonatomic) unsigned char portState;
@property (nonatomic) NSThread * ptpdThread;
@property (nonatomic) int ptpdMode;
@property (readwrite, nonatomic)BOOL isClockOn;

@property (strong,nonatomic) NSTimer * counter;
@property (nonatomic) BOOL firstSave;

@end

@implementation PtpClockModel

//Start the subthread of the ptp daemon
-(void) startTheClock:(int) mode
{
    self.isClockOn = YES;
    self.ptpdMode = mode;
    if (self.ptpdThread == nil) {
        self.ptpdThread = [[NSThread alloc] initWithTarget:self selector:@selector(ptpdaemon:) object:nil];
        [self.ptpdThread start];
    }
    if (!self.counter) {
        double interval = 1;  // 間隔多久執行一次 (秒)
        self.counter=[NSTimer scheduledTimerWithTimeInterval:interval
                                                      target:self
                                                    selector:@selector(monitorAndSave:)
                                                    userInfo:nil
                                                     repeats:YES];
    }
    
}
//define the ptp daemon how to work
-(void) ptpdaemon: (NSThread*) callerThread {
    int ptpdout;
    if (self.ptpdMode == 0) {  //free run
        char* ptpdparameter[5]={"ptpd2","-c","-D","-b","en0"};
        ptpdout = ptpdmain(5,ptpdparameter);
    } else if (self.ptpdMode == 1) { //Master
        char* ptpdparameter[6]={"ptpd2","-c","-W","-D","-b","en0"};
        ptpdout = ptpdmain(6,ptpdparameter);
    } else { // Slave
        char* ptpdparameter[6]={"ptpd2","-c","-g","-D","-b","en0"};
        ptpdout = ptpdmain(6,ptpdparameter);
    }
    if(ptpdout == 0)
    {
        NSLog(@"FINISH : PTPD2  STOP");
    }else{
        NSLog(@"ERROR : PTPD2  STOP");
    }
    [self.counter invalidate];
    self.counter = nil;
    if(![self.ptpdThread isCancelled])
    {
        [self.ptpdThread cancel];
    }
    if ([[NSThread currentThread] isCancelled])
    {
        self.ptpdThread=nil;
        [NSThread exit];
    }
}

//rise up the stop flag in subthread
-(void) stopTheClock {
    self.isClockOn = NO;
    [self.counter invalidate];
    self.counter = nil;
    ptpd_shutdown_flag = 1;
    if(![self.ptpdThread isCancelled])
    {
        [self.ptpdThread cancel];
    }
}

-(int) offset_Second {
    _offset_Second = G_offsetFromMaster->seconds;
    return _offset_Second;
}
-(int) offset_Nanosecond {
    _offset_Nanosecond = G_offsetFromMaster->nanoseconds;
    return _offset_Nanosecond;
}


-(struct timespec) virtual_time {
    _virtual_time = *G_virtualTime;
   // _virtual_time = G_virtualClock->virTime;
    return _virtual_time;
}
//printf("%lld.%.9ld", (long long)ts.tv_sec, ts.tv_nsec)

-(unsigned char) portState {
    _portState = *G_portState;
    return _portState;
}

-(int) timespec2str :(char *)buf len:(uint)len ts:(struct timespec*)ts {
    int ret;
    struct tm t;
    
    tzset();
    if (localtime_r(&(ts->tv_sec), &t) == NULL)
        return 1;
    
    ret = strftime(buf, len, "%F %T", &t);
    if (ret == 0)
        return 2;
    len -= ret - 1;
    
    ret = snprintf(&buf[strlen(buf)], len, ".%09ld", ts->tv_nsec);
    if (ret >= len)
        return 3;
    
    return 0;
}
/*     Returns the URL to the application's Documents directory. */
- (NSURL *)applicationDocumentsDirectory {
    return [[[NSFileManager defaultManager] URLsForDirectory:NSDocumentDirectory
                                                   inDomains:NSUserDomainMask] objectAtIndex:0];
}

-(void)monitorAndSave:(NSTimer *)sender
{
    /*[debug] print the virtual time. test the timer resolution*/
    const uint TIME_FMT = strlen("2012-12-31 12:59:59.123456789") + 1;
    char timestr[TIME_FMT];
    struct timespec ts = self.virtual_time;
    [self timespec2str:timestr len:TIME_FMT ts:&ts];
    int offset_second_abs = (self.offset_Second >= 0)? self.offset_Second : -self.offset_Second;
    int offset_nanosecond_abs = (self.offset_Nanosecond >= 0)? self.offset_Nanosecond : -self.offset_Nanosecond;

    NSString *offsetString;
    if (self.offset_Second < 0 || self.offset_Nanosecond < 0) {
        offsetString = [[NSString alloc] initWithFormat:@"-%d.%09d",offset_second_abs, offset_nanosecond_abs];
    } else {
        offsetString = [[NSString alloc] initWithFormat:@" %d.%09d",offset_second_abs, offset_nanosecond_abs];
    }
    NSLog(@"%s,%@", timestr, offsetString);
    ////////////////////////////////////////////////////////////////////////
    ///save the file to the document
    NSString *log = [[NSString alloc] initWithFormat:@"%s,%@\r", timestr, offsetString];
    NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory,NSUserDomainMask, YES);
    NSString *filePath = [paths objectAtIndex:0];

    NSString *path = [filePath stringByAppendingPathComponent:@"synchronization_log.txt"];
    if (!self.firstSave) {
        [log writeToFile:path atomically:YES encoding:NSUTF8StringEncoding error:nil];
        self.firstSave = YES;
    } else {
        NSFileHandle *fileHandle = [NSFileHandle fileHandleForWritingAtPath:path];
        [fileHandle seekToEndOfFile];
        [fileHandle writeData:[log dataUsingEncoding:NSUTF8StringEncoding]];
        [fileHandle closeFile];
    }

}

-(NSString *)monitor
{
    const uint TIME_FMT = strlen("2012-12-31 12:59:59.123456789") + 1;
    char timestr[TIME_FMT];
    struct timespec ts = self.virtual_time;
    [self timespec2str:timestr len:TIME_FMT ts:&ts];
    int offset_second_abs = (self.offset_Second >= 0)? self.offset_Second : -self.offset_Second;
    int offset_nanosecond_abs = (self.offset_Nanosecond >= 0)? self.offset_Nanosecond : -self.offset_Nanosecond;
    
    NSString *offsetString;
    if (self.offset_Second < 0 || self.offset_Nanosecond < 0) {
        offsetString = [[NSString alloc] initWithFormat:@"-%d.%09d",offset_second_abs, offset_nanosecond_abs];
    } else {
        offsetString = [[NSString alloc] initWithFormat:@" %d.%09d",offset_second_abs, offset_nanosecond_abs];
    }
    NSString *outString = [[NSString alloc] initWithFormat:@"%s,%@", timestr, offsetString];
    return outString;
}

@end
