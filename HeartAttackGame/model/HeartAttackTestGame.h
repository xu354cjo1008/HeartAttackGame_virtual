//
//  HeartAttackTestGame.h
//  HeartAttack
//
//  Created by 李勁璋 on 13/10/17.
//  Copyright (c) 2013年 李勁璋. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "Card.h"
#import "Deck.h"
#import "GCDAsyncSocket.h"
#import "PtpClockModel.h"
@class HeartAttackTestGame;
@protocol HeartAttackTestGameDelegate
@optional
- (void)updateCard:(Card *)card count:(int)count;
- (void)updateToken:(BOOL)token;
- (void)slapCardEventFromPlayer:(int)player;
- (void)firstSlapCardEventFromPlayer:(int)player;
@end
@interface HeartAttackTestGame : NSObject

//host method
-(id)initWithMode:(NSString *)mode character:(NSString *)character hostSocket:(GCDAsyncSocket*)hostSocket PlayersSocket:(NSMutableArray*)playersSocket ptpClock:(PtpClockModel*)ptpClock;
//-(id)initGuestWithPlayersSocket : (AsyncSocket*)hostSocket ptpClock:(PtpClockModel*)ptpClock;

-(void)startGame;

- (void)setDelegate:(id)delegate;

-(void)stopGame;

-(void)flipCard;

-(void) judge;

//guest method
-(void)slapTheCard;

-(void)doPlayersInit:(NSInteger)playerNum;

@property (strong, nonatomic) Card* card;
@property (nonatomic,readonly) int GameState;
@property (strong, nonatomic) NSMutableArray *players;
@property (nonatomic) int playerId;
@property (nonatomic) int heartAttackCounter;

@end
