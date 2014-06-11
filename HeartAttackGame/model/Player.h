//
//  Player.h
//  HeartAttack
//
//  Created by 李勁璋 on 2013/11/9.
//  Copyright (c) 2013年 李勁璋. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "GCDAsyncSocket.h"

@interface Player : NSObject

@property (nonatomic) unsigned int playerId;
@property (nonatomic, strong) GCDAsyncSocket *connectSocket;
@property (strong,nonatomic) NSDate *slapTime;
@property (nonatomic) int playerScore;

-(id)initWithSocket :(GCDAsyncSocket*)socket Id : (unsigned int)playerId;

@end
