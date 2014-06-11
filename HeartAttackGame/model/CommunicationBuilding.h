//
//  CommunicationBuilding.h
//  HeartAttack
//
//  Created by 李勁璋 on 2013/11/11.
//  Copyright (c) 2013年 李勁璋. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "GCDAsyncSocket.h"


@interface CommunicationBuilding : NSObject

-(id)initWithMode:(NSString *)mode character : (NSString *)character;

-(void)closeCommunication;

-(void)closeinitSocket;

-(BOOL)closeAsyncUdpSocket;

-(BOOL)isConnected;

-(BOOL)isStart;


-(void)connectToHost: (NSString *)address;

-(void)clearPartyBuffer;

@property (nonatomic)NSString *communicationMode;

@property (nonatomic) NSString *character;

@property (strong, nonatomic) NSMutableArray *gamePartys;

@property (nonatomic) GCDAsyncSocket *connectedHost;

@property (nonatomic) NSMutableArray *connectedSockets;

@property (nonatomic) NSInteger connectedNumbers;


@end
