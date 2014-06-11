//
//  ViewController.h
//  HeartAttackGame
//
//  Created by 李勁璋 on 2013/11/27.
//  Copyright (c) 2013年 李勁璋. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "GCDAsyncSocket.h"
#import "PtpClockModel.h"

@interface HeartAttackGameViewController : UIViewController

-(void)doInitWithPlayers:(NSMutableArray *)players host:(GCDAsyncSocket *)host mode:(NSString *)mode character:(NSString*)character ptpClock:(PtpClockModel *)ptpClock;
//-(void)self.heartAttackGame = [[HeartAttackTestGame alloc] initWithMode:Slave hostSocket:self.connectedHost PlayersSocket:self.playersSocket ptpClock:self.ptpClock];
//[self.heartAttackGame setDelegate:self];

-(void)doPlayerInit:(NSInteger)playerNum;
@end
