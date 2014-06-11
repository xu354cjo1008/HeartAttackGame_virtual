//
//  HeartAttackTestGame.m
//  HeartAttack
//
//  Created by 李勁璋 on 13/10/17.
//  Copyright (c) 2013年 李勁璋. All rights reserved.
//

#import "HeartAttackTestGame.h"
#import "PlayingCard.h"
#import "PlayingCardDeck.h"
#import "PlayingCard.h"
#import "Player.h"

@interface HeartAttackTestGame()
@property (strong, nonatomic) Deck* heartattackDeck;
@property (nonatomic) NSString *heartAttackMode;
@property (nonatomic) NSString *heartAttackCharacter;
@property (strong, nonatomic) NSDate* lastFlipTime;
@property (nonatomic,readwrite) int GameState;
@property (strong, nonatomic) Player *gameHost;
@property (nonatomic) unsigned int playersNumbers;
@property (nonatomic) id theDelegate;

@property (nonatomic) BOOL token;
@property (nonatomic) BOOL nexToken;
@property (nonatomic) NSInteger nextPerson;

@property (nonatomic) BOOL isAnyOneSlapBefore;


@property (strong, nonatomic) PtpClockModel* ptpClock;

@end

@implementation HeartAttackTestGame

#define Master @"Master"
#define Slave @"Slave"
#define MAXPLAYERS 4

#define P2P @"P2P"
#define CS @"CS"

#define INITSTATE 0
#define GAMESTATE 1
#define JUDGESTATE 2


- (NSMutableArray *)players
{
    if(!_players) _players = [[NSMutableArray alloc] init];
    return _players;
}


-(id)initWithMode:(NSString *)mode character:(NSString *)character hostSocket:(GCDAsyncSocket*)hostSocket PlayersSocket:(NSMutableArray*)playersSocket ptpClock:(PtpClockModel*)ptpClock
{
    self =[super init];
    if(self) {
        self.heartAttackMode = mode;
        self.heartAttackCharacter = character;
        self.GameState = INITSTATE;
        self.ptpClock = ptpClock;
        self.playersNumbers = 0;
        self.token = NO;
        self.nexToken = NO;
        if ([character  isEqual: Master]) {
            self.token = YES;
            if (!playersSocket) {
                return self;
            }
            Player *player = [[Player alloc] initWithSocket:hostSocket Id:self.playersNumbers];
            self.playersNumbers ++ ;
            [self.players addObject:player];
            player =nil;
            for (GCDAsyncSocket *socket in playersSocket) {
                [socket setDelegate:self];
                [socket readDataWithTimeout:-1 tag:0];
                Player *player = [[Player alloc] initWithSocket:socket Id:self.playersNumbers];
                [self.players addObject:player];
                self.playersNumbers ++ ;
                player = nil;
            }
            self.nextPerson = 1;
            NSString *strData = [[NSString alloc] init];
            strData = @"next";
            [[self.players[1] connectSocket] writeData:[strData dataUsingEncoding:NSUTF8StringEncoding] withTimeout:-1 tag:0];
        } else if ([character  isEqual: Slave]) {
            [hostSocket setDelegate:self];
            self.gameHost = [[Player alloc] initWithSocket:hostSocket Id:self.playersNumbers];
            [hostSocket readDataWithTimeout:-1 tag:0];
        }
    }
    return self;
}
-(void)doPlayersInit:(NSInteger)playerNum
{
    Player *selfPlayer = [[Player alloc] init];
    [self.players addObject:selfPlayer];
    Player *hostPlayer = [[Player alloc] init];
    [self.players addObject:hostPlayer];
    for (int i=2; i<playerNum; i++) {
        Player *player = [[Player alloc] init];
        [self.players addObject:player];
    }
}

- (void)setDelegate:(id)delegate
{
#if DEBUG_THREAD_SAFETY
	[self checkForThreadSafety];
#endif
	
	self.theDelegate = delegate;
}
// game host method
-(void)startGame
{
    if ([self.heartAttackCharacter  isEqual: Master]) {                 //判斷是否為主持人
        self.GameState = GAMESTATE;
        if (!self.heartattackDeck) {                                    //假如是主持人 則創建牌堆
            self.heartattackDeck = [[PlayingCardDeck alloc] init];
        }
        //將players的slapTime設為無窮遠  方便比對計算
        //告知所有玩家開始遊戲
      //  NSString *strData = [[NSString alloc] initWithFormat:@"start/%lu",(unsigned long)self.players.count];
      //  strData = @"start";
        for (int i = 0; i < self.players.count; i++) {
            [[self.players objectAtIndex:i] setSlapTime:[NSDate distantFuture]];
            NSString *strData = [[NSString alloc] initWithFormat:@"start/%d",i];
            [[self.players[i] connectSocket] writeData:[strData dataUsingEncoding:NSUTF8StringEncoding] withTimeout:-1 tag:0];
        }
    } else if ([self.heartAttackCharacter  isEqual: Slave]) {
        NSLog(@"You are not game host");
    }
    if([self.theDelegate respondsToSelector:@selector(updateToken:)])
    {
        [self.theDelegate updateToken:self.token];
    }
    self.heartAttackCounter = 0;                                     //初始化遊戲計數器 和遊戲狀態
    self.isAnyOneSlapBefore = NO;
}
-(void)stopGame
{
    self.heartattackDeck = nil;                                      //清除牌堆 並告之所有玩家遊戲停止
    self.heartAttackCounter = 0;
    self.GameState = INITSTATE;
    NSString *strData = [[NSString alloc] init];
    strData = @"stop";
    for (int i = 0; i < self.players.count; i++) {
        [[self.players[i] connectSocket] writeData:[strData dataUsingEncoding:NSUTF8StringEncoding] withTimeout:-1 tag:0];
    }
}
//通用翻牌--告知主持人我翻牌了
- (void)flipCard
{//假如有拿到令牌 並且是在遊戲狀態 則 1. 更新令牌 2. 告知主持人翻牌
    if (self.token && self.GameState == GAMESTATE) {
        self.token = NO;
        if([self.theDelegate respondsToSelector:@selector(updateToken:)])
        {
            [self.theDelegate updateToken:self.token];
        }
       // self.nexToken = NO;
        if ([self.heartAttackCharacter isEqualToString:Master]) {
            [self flipTheCard];
        } else {
            NSString *str = [[NSString alloc]initWithFormat:@"flip"];
            NSData *strData = [str dataUsingEncoding:NSUTF8StringEncoding];
            [self.gameHost.connectSocket writeData:strData withTimeout:-1 tag:0];
        }
    }
}
//主持人翻牌 --實際翻牌命令    1. 翻牌 並告知所有玩家檯面上的牌是什麼 2. 計算下一個拿到令牌之玩家 並告知其玩家下次會拿到令牌
- (void)flipTheCard
{
    self.heartAttackCounter ++;
    self.card = [self.heartattackDeck drawRandomCard];
    if (self.card) {
        self.card.faceUp = !self.card.isFaceUp;
    } else {
        self.card = nil;
    }
    self.lastFlipTime = [[NSDate alloc] initWithTimeIntervalSince1970:(double)self.ptpClock.virtual_time.tv_sec + (double)self.ptpClock.virtual_time.tv_nsec / NANOPERSECOND];//[NSDate date];     //(modify) change system time to virtual time
    
    //master 告知所有人翻開的牌
    PlayingCard *playingCard = (PlayingCard *)self.card;
    /////*lastfliptime transform*////
    NSDateFormatter *dateFormatter = [[NSDateFormatter alloc] init];
    [dateFormatter setDateFormat:@"yyyy-MM-dd HH:mm:ss.SSSSSSSSS"];
    NSString *strDate = [dateFormatter stringFromDate:self.lastFlipTime];  //20140320 modify hosttime to currenttime
    NSLog(@"%@", strDate);
    //////////////////////////
    NSString *str = [[NSString alloc]initWithFormat:@"flip/%@,%lu,%d,%@",playingCard.suit,(unsigned long)playingCard.rank,self.heartAttackCounter,strDate];
    NSData *strData = [str dataUsingEncoding:NSUTF8StringEncoding];
    for (Player *player in self.players) {
        [player.connectSocket writeData:strData withTimeout:-1 tag:0];
    }
    self.nextPerson = (self.nextPerson+1)%self.players.count;
    if (self.nextPerson == 0) {
        self.nexToken = YES;
    } else {
       NSString *str2 = [[NSString alloc] init];
       str2 = @"next";
       [[self.players[self.nextPerson] connectSocket] writeData:[str2 dataUsingEncoding:NSUTF8StringEncoding] withTimeout:-1 tag:0];
    }
    if([self.theDelegate respondsToSelector:@selector(updateToken:)])
    {
        [self.theDelegate updateToken:self.token];
    }
    playingCard = nil;
    self.isAnyOneSlapBefore = NO;

}
//setter of heartattack counter
-(void) setHeartAttackCounter:(int)heartAttackCounter
{
    if (heartAttackCounter > [PlayingCard maxRank]) {
        _heartAttackCounter = heartAttackCounter % [PlayingCard maxRank];
    } else {
        _heartAttackCounter = heartAttackCounter;
    }
}


// 裁判--裁判贏家與輸家 並計算分數 告知所有玩家更新計分板
#define FIRSE_BOUNS 1
#define LAST_PENALTY 1
#define MISMATCH_PENALTY 2

-(void) judge
{
    [NSThread sleepForTimeInterval:1];
    NSInteger fastIndex;
    NSInteger lastIndex;
    NSDate* fastTime = [NSDate distantFuture];
    NSDate* lastTime = [NSDate distantPast];
    for (Player* player in self.players)
    {
        if ([player.slapTime compare:fastTime] == NSOrderedAscending)
        {
            fastTime = player.slapTime;
            fastIndex = [self.players indexOfObject:player];
        }
        if ([player.slapTime compare:lastTime] == NSOrderedDescending)
        {
            lastTime = player.slapTime;
            lastIndex = [self.players indexOfObject:player];
        }
    }
    if (((PlayingCard*)self.card).rank == self.heartAttackCounter) {
        [[self.players objectAtIndex:fastIndex] setPlayerScore:[[self.players objectAtIndex:fastIndex] playerScore]+FIRSE_BOUNS];
        if (lastIndex != fastIndex) {
            [[self.players objectAtIndex:lastIndex] setPlayerScore:[[self.players objectAtIndex:lastIndex] playerScore]-LAST_PENALTY];
        }
    } else {
        [[self.players objectAtIndex:fastIndex] setPlayerScore:[[self.players objectAtIndex:fastIndex] playerScore]-MISMATCH_PENALTY];
    }
    NSString *strData = [[NSString alloc]initWithFormat:@"score/"];
    for (Player *player in self.players) {
        NSString *score = [[NSString alloc] initWithFormat:@"%d,",player.playerScore];
        strData = [strData stringByAppendingString:score];
    }
    for (Player *player in self.players) {
        [player.connectSocket writeData:[strData dataUsingEncoding:NSUTF8StringEncoding] withTimeout:-1 tag:0];
    }
    self.card.faceUp = NO;
    self.heartAttackCounter = 0;
    self.isAnyOneSlapBefore = NO;
}


// game player method


//拍牌   1. 以虛擬同步時鐘記錄拍牌時間 2. 檢查拍牌正確性 -- 是否違逆因果  3. 假如是主持人拍牌 則直接進入裁判狀態 並告知所有玩家有人拍牌  4. 普通玩家拍牌--告知主持人我拍牌了 請求進入裁判
-(void)slapTheCard
{
    //先將資訊轉成string再轉成NSData
    NSDateFormatter *dateFormatter = [[NSDateFormatter alloc] init];
    [dateFormatter setDateFormat:@"yyyy-MM-dd HH:mm:ss.SSSSSSSSS"];
  //  NSDate *currentTime = [NSDate date];
    NSDate *currentTime = [[NSDate alloc] initWithTimeIntervalSince1970:(double)self.ptpClock.virtual_time.tv_sec + (double)self.ptpClock.virtual_time.tv_nsec / NANOPERSECOND];
    NSTimeInterval timeInterval = [currentTime timeIntervalSinceDate:self.lastFlipTime];
    if (timeInterval >= 0 && self.card.faceUp == YES) {
        if ([self.heartAttackCharacter isEqualToString:Master]) {
                [self.players[0] setSlapTime:currentTime];
                self.GameState = JUDGESTATE;
                NSString *strData = [[NSString alloc]initWithFormat:@"slap"];
                for (Player *player in self.players) {
                    [player.connectSocket writeData:[strData dataUsingEncoding:NSUTF8StringEncoding] withTimeout:-1 tag:0];
                }
                if (self.isAnyOneSlapBefore == NO) {
                    if([self.theDelegate respondsToSelector:@selector(firstSlapCardEventFromPlayer:)])
                    {
                        [self.theDelegate firstSlapCardEventFromPlayer:0];
                    }
                    self.isAnyOneSlapBefore = YES;
                }
        } else {
           // NSDate *hostTime = [currentTime dateByAddingTimeInterval:-(self.ptpClock.offset_Second+self.ptpClock.offset_Nanosecond*0.000000001)];
            NSString *strDate = [dateFormatter stringFromDate:currentTime];  //20140320 modify hosttime to currenttime
            NSLog(@"%@", strDate);
            NSString *strData = [[NSString alloc]initWithFormat:@"slap/%@",strDate];
            dateFormatter =nil;
            [self.gameHost.connectSocket writeData:[strData dataUsingEncoding:NSUTF8StringEncoding] withTimeout:-1 tag:0];
            strDate = nil;
           // hostTime = nil;   ///20140320
        }
        if([self.theDelegate respondsToSelector:@selector(slapCardEventFromPlayer:)])
        {
            [self.theDelegate slapCardEventFromPlayer:0];
        }
    }
    currentTime = nil;
}

//message 解碼 --- msgType/msg
- (NSString *)unpackWithInput:(NSData *)data output:(NSString **)outString
{
    NSString* reseiveStr = [[NSString alloc] initWithData:data encoding:NSUTF8StringEncoding];
    NSArray *list = [reseiveStr componentsSeparatedByString:@"/"];
   // NSString *msgType = [list[0] string];
    //NSString *receivedData = [list[1] string];
    NSString *msgType = [[NSString alloc] initWithString:list[0]];
    if (list.count > 1) {
        NSString *receivedData = [[NSString alloc] initWithString:list[1]];
        *outString = receivedData;
    }
    return msgType;
}
// message process router
- (void)processWithMsgType:(NSString *)msgType msg:(NSString *)msg from:(GCDAsyncSocket *)sock
{
    if ([self.heartAttackCharacter  isEqual: Master]) {
        if ([msgType isEqualToString:@"start"]) {
            ;
        } else if ([msgType isEqualToString:@"flip"]) {
            self.token = self.nexToken;
            self.nexToken = NO;
            [self flipTheCard];
            if([self.theDelegate respondsToSelector:@selector(updateToken:)])
            {
                [self.theDelegate updateToken:self.token];
            }
            if([self.theDelegate respondsToSelector:@selector(updateCard:count:)])
            {
                [self.theDelegate updateCard:self.card count:self.heartAttackCounter];
            }
        } else if ([msgType isEqualToString:@"slap"]) {
            [self slapProcessWithMsg:msg from:sock];
        } else if ([msgType isEqualToString:@"stop"]) {
            
        } else if ([msgType isEqualToString:@"next"]) {
            self.nexToken = YES;
        }
    } else if ([self.heartAttackCharacter  isEqual: Slave]) {
        if ([msgType isEqualToString:@"start"]) {
            [self startProcessWithMsg:msg from:sock];
        } else if ([msgType isEqualToString:@"flip"]) {
            self.token = self.nexToken;
            self.nexToken = NO;
            [self flipPrrocessWithMsg:msg from:sock];
        } else if ([msgType isEqualToString:@"slap"]) {
            self.GameState = JUDGESTATE;
        } else if ([msgType isEqualToString:@"stop"]) {
            [self stopPrrocessWithMsg:msg from:sock];
        } else if ([msgType isEqualToString:@"next"]) {
            self.nexToken = YES;
        } else if ([msgType isEqualToString:@"nextstart"]) {
            self.nexToken = YES;
            [self startProcessWithMsg:msg from:sock];
        } else if ([msgType isEqualToString:@"nextflip"]) {
            self.nexToken = YES;
            [self flipPrrocessWithMsg:msg from:sock];
        } else if ([msgType isEqualToString:@"score"]) {
            [self scorePrrocessWithMsg:msg from:sock];
        }
    }
}
- (void)startProcessWithMsg:(NSString *)msg from:(GCDAsyncSocket *)sock
{
    [self.players[[msg integerValue]] setPlayerId:[msg integerValue]];
    self.playerId = [msg integerValue];
    self.heartAttackCounter = 0;
    self.GameState = GAMESTATE;
    self.isAnyOneSlapBefore = NO;
}
- (void)stopPrrocessWithMsg:(NSString *)msg from:(GCDAsyncSocket *)sock
{
    self.heartattackDeck = nil;
    self.heartAttackCounter = 0;
    self.GameState = INITSTATE;
    self.card.faceUp = NO;
    if([self.theDelegate respondsToSelector:@selector(updateCard:count:)])
	{
        [self.theDelegate updateCard:self.card count:self.heartAttackCounter];
	}
}
- (void)slapProcessWithMsg:(NSString *)msg from:(GCDAsyncSocket *)sock
{
    NSDateFormatter *dateFormatter = [[NSDateFormatter alloc] init];
    [dateFormatter setDateFormat:@"yyyy-MM-dd HH:mm:ss.SSSSSSSSS"];
    NSDate *slapTime = [dateFormatter dateFromString:msg];
    NSLog(@"%@", msg);
    dateFormatter = nil;
    msg = nil;
    //假如收到的資料是過去的  則忽略
    //是正確的資料則放到slapTime array中
    NSTimeInterval timeInterval = [slapTime timeIntervalSinceDate:self.lastFlipTime];
    if (timeInterval >= 0) {
        self.GameState = JUDGESTATE;
        NSString *strData = [[NSString alloc]initWithFormat:@"slap"];
        for (Player *player in self.players) {
            [player.connectSocket writeData:[strData dataUsingEncoding:NSUTF8StringEncoding] withTimeout:-1 tag:0];
        }
        int slapPlayer;
      //  BOOL isAnyOneSlapBefore = NO;
        for (int i = 0; i<self.players.count; i++) {
            if (sock == [[self.players objectAtIndex:i] connectSocket]) {
                [[self.players objectAtIndex:i] setSlapTime:slapTime];
                slapPlayer = i;
            }
        }
        if (self.isAnyOneSlapBefore == NO) {
            if([self.theDelegate respondsToSelector:@selector(firstSlapCardEventFromPlayer:)])
            {
                [self.theDelegate firstSlapCardEventFromPlayer:slapPlayer];
            }
            self.isAnyOneSlapBefore = YES;
        }
        if([self.theDelegate respondsToSelector:@selector(slapCardEventFromPlayer:)])
        {
            [self.theDelegate slapCardEventFromPlayer:slapPlayer];
        }
    }
}

- (void)flipPrrocessWithMsg:(NSString *)msg from:(GCDAsyncSocket *)sock
{
    NSArray *list = [msg componentsSeparatedByString:@","];
    PlayingCard * playingCard = [[PlayingCard alloc] init];
    playingCard.suit = list[0];
    playingCard.rank = [list[1] integerValue];
    self.heartAttackCounter = [list[2] integerValue];
    NSDateFormatter *dateFormatter = [[NSDateFormatter alloc] init];
    [dateFormatter setDateFormat:@"yyyy-MM-dd HH:mm:ss.SSSSSSSSS"];
    NSDate *lastFlipTime = [dateFormatter dateFromString:list[3]];
    self.lastFlipTime = lastFlipTime;
    self.card = playingCard;
    self.card.faceUp = YES;
   // self.heartAttackCounter ++;
    playingCard = nil;
    if([self.theDelegate respondsToSelector:@selector(updateCard:count:)])
	{
        [self.theDelegate updateCard:self.card count:self.heartAttackCounter];
	}
    if([self.theDelegate respondsToSelector:@selector(updateToken:)])
    {
        [self.theDelegate updateToken:self.token];
    }
}

- (void)scorePrrocessWithMsg:(NSString *)msg from:(GCDAsyncSocket *)sock
{
    NSArray *list = [msg componentsSeparatedByString:@","];
    for (int i = 0;i < self.players.count; i++) {
        [self.players[i] setPlayerScore:[list[i] integerValue]];
    }
    self.card.faceUp = NO;
    self.heartAttackCounter = 0;

    if([self.theDelegate respondsToSelector:@selector(updateCard:count:)])
	{
        [self.theDelegate updateCard:self.card count:self.heartAttackCounter];
	}
}


//TCP連線
////////////////////////////////////////////////////////////////

#pragma mark AsyncSocket Delegate Methods
//監聽網路封包  假如收到資料則呼叫 unpackWithInput 和 processWithMsgType 處理message
- (void)socket:(GCDAsyncSocket *)sock didReadData:(NSData *)data withTag:(long)tag
{
    NSString* reseiveStr;
    NSString* msgType =[self unpackWithInput:data output:&reseiveStr];
    [self processWithMsgType:msgType msg:reseiveStr from:sock];
    [sock readDataWithTimeout:-1 tag:0];
}
- (void)socketDidDisconnect:(GCDAsyncSocket *)sock withError:(NSError *)err
{
    
}
- (void)socket:(GCDAsyncSocket *)sock didConnectToHost:(NSString *)host port:(uint16_t)port
{
    
}

- (void)socket:(GCDAsyncSocket *)sock didWriteDataWithTag:(long)tag
{
    
}


@end
