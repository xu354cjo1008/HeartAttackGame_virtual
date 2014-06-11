//
//  Player.m
//  HeartAttack
//
//  Created by 李勁璋 on 2013/11/9.
//  Copyright (c) 2013年 李勁璋. All rights reserved.
//

#import "Player.h"

@implementation Player

-(id)initWithSocket :(GCDAsyncSocket*)socket Id : (unsigned int)playerId
{
    self =[super init];
    if(self) {
        _playerId = playerId;
        _connectSocket = socket;
        _slapTime = [NSDate distantFuture];
        _playerScore = 0;
    }
    return self;
}
@end
