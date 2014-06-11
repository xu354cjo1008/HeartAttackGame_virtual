//
//  CommunicationBuilding.m
//  HeartAttack
//
//  Created by 李勁璋 on 2013/11/11.
//  Copyright (c) 2013年 李勁璋. All rights reserved.
//

#import "CommunicationBuilding.h"
#import "GCDAsyncUdpSocket.h"
#import "GCDAsyncSocket.h"

@interface CommunicationBuilding()

@property (strong,nonatomic) GCDAsyncUdpSocket *asyncUdpSocket;
@property (strong,nonatomic) GCDAsyncSocket *listenSocket;
@property (nonatomic) NSTimer* counter;
@property (nonatomic) BOOL start;
@end

@implementation CommunicationBuilding

#define Master @"Master"
#define Slave @"Slave"
#define FREE @"Free"
#define P2P @"P2P"
#define CS @"CS"

#define MAXPLAYERS 4


#define kUDP_PORT 50010
#define kTCP_PORT 50011

//#define kBroadCastIp @"224.10.1.130"
//#define kBroadCastIp @"224.0.0.1"
//#define kBroadCastIp @"233.252.124.100"
//#define kBroadCastIp @"172.20.10.15"     //hotspot
//#define kBroadCastIp @"172.20.10.255"     //hotspot

#define kBroadCastIp @"169.254.255.255"  //mac ad hoc
//#define kBroadCastIp @"192.168.0.255"  // lab207
//#define kBroadCastIp @"10.118.255.255"   //ntu_peap
//#define kBroadCastIp @"255.255.255.255"

-(BOOL)isStart
{
    return _start;
}
-(NSMutableArray *) connectedSockets
{
    if (!_connectedSockets) {
        _connectedSockets = [[NSMutableArray alloc] init];
    }
    return _connectedSockets;
}
-(NSMutableArray *) gamePartys
{
    if (!_gamePartys) {
        _gamePartys = [[NSMutableArray alloc] init];
    }
    return _gamePartys;
}

-(id)initWithMode:(NSString *)mode character : (NSString *)character   //initialize with Master or Slave
{
    self =[super init];
    if(self) {
        self.character = character;
        self.communicationMode = mode;
        [self initAsyncUdpSocket];              //slave開始監聽 邀請訊息
        if ([self.character  isEqual: Master]) {
            [self initAsyncListenSocket];                     //初始化TCP socket
            double interval = 1;  // 間隔多久執行一次 (秒)
            self.counter=[NSTimer scheduledTimerWithTimeInterval:interval              //Master不停的發送邀請廣播
                                                          target:self
                                                        selector:@selector(inviteBrocast:)
                                                        userInfo:nil
                                                         repeats:YES];
        } else if ([self.character  isEqual: Slave]) {
            //[self initAsyncSocket];                     //初始化TCP socket
           // [self.connectedSockets addObject:0];
        }
        self.connectedNumbers = 1;
    }
    return self;
}

- (void)inviteBrocast:(NSTimer *)sender
{
    NSMutableString *str = [[NSMutableString alloc] init];
    [str appendFormat:@"%@",@"Invite"];
    [self broadcastEntry:kBroadCastIp data:str];
    str = nil;
}

-(void)closeCommunication
{
    [self closeAsyncUdpSocket];
    [self closeAsyncSocket];
}

-(void)closeinitSocket
{
    [self.listenSocket disconnect];
    [self closeAsyncUdpSocket];
    if ([self.character isEqualToString:Master]) {
        NSString *str = [[NSString alloc]initWithFormat:@"start"];
        NSData * strData = [str dataUsingEncoding:NSUTF8StringEncoding];
        for (GCDAsyncSocket *socket in self.connectedSockets) {
            [socket writeData:strData withTimeout:-1 tag:0];
        }
    }
}
/*
-(BOOL)isCommunicated
{
    return (self.asyncSocket)? YES : NO;
}
 */
-(BOOL)isConnected
{
    return (self.connectedSockets)? YES : NO;
}

-(void)connectToHost:(NSString *)address
{
   // AsyncSocket *sock;
    self.connectedHost = [self asyncSocketInit:self.connectedHost];
    NSError *error = nil;
    [self.connectedHost connectToHost:address onPort:kTCP_PORT withTimeout:-1 error:&error];
    if ([self.communicationMode  isEqual: @"P2P"]) {
        [self initAsyncListenSocket];
    }
    //[self.connectedHost readDataWithTimeout:-1 tag:0];
}

-(void)connectToOther:(NSString *)address
{
    GCDAsyncSocket *sock;
    sock = [self asyncSocketInit:sock];
    NSError *error = nil;
    [sock connectToHost:address onPort:kTCP_PORT withTimeout:-1 error:&error];
}

-(void)clearPartyBuffer
{
    self.gamePartys = nil;
}


//UDP 連線
//////////////////////////////////////////////////////////////////////////////////
//初始化socket通信
- (void)initAsyncUdpSocket {
    //初始化udp
    self.asyncUdpSocket=[[GCDAsyncUdpSocket alloc] initWithDelegate:self delegateQueue:dispatch_get_main_queue()];
    [self.asyncUdpSocket setIPv6Enabled:NO];
    //绑定端口
    NSError *error = nil;
	[self.asyncUdpSocket bindToPort:kUDP_PORT error:&error];
    //发送广播设置
    [self.asyncUdpSocket enableBroadcast:YES error:&error];
    //加入群里，能接收到群里其他客户端的消息
   // [self.asyncUdpSocket joinMulticastGroup:kBroadCastIp error:&error];
    if ([self.character  isEqual: Master]) {
        ;
    } else if ([self.character  isEqual: Slave])
    {
        //如果是slave  启动接收线程
	    [self.asyncUdpSocket beginReceiving:&error];
    }
}


-(BOOL)closeAsyncUdpSocket {
    if ([self.counter isValid]) {
        [self.counter invalidate];
    }
    [self.asyncUdpSocket close];
    self.asyncUdpSocket = nil;
    return YES;
}

//发送广播，通知其他客户端
- (void)broadcastEntry:(NSString *)host data:(NSMutableString *)str{
    
    [self.asyncUdpSocket sendData:[str dataUsingEncoding:NSUTF8StringEncoding]
                           toHost:host
                             port:kUDP_PORT
                      withTimeout:-1
                              tag:0];
    
    str = nil;
}


#pragma mark AsyncUdpSocket Delegate Methods

//UDP接收消息
/**
 * Called when the socket has received the requested datagram.
 **/
- (void)udpSocket:(GCDAsyncUdpSocket *)sock didReceiveData:(NSData *)data
      fromAddress:(NSData *)address
withFilterContext:(id)filterContext
{
    NSString* reseiveStr = [[NSString alloc] initWithData:data encoding:NSUTF8StringEncoding];
    NSString *host;
    uint16_t port = 0;
    [GCDAsyncUdpSocket getHost:&host port:&port fromAddress:address];
    NSLog(@"receive messages-->%@ from %@",reseiveStr,host);
    if ([reseiveStr  isEqual: @"Invite"]) {
        BOOL flag = NO;
        for (NSString *udpSocket in self.gamePartys) {
            if ([udpSocket isEqualToString:host]) {
                flag = YES;
            }
        }
        if (flag == NO) {
            [self.gamePartys addObject:host];
        }
    }
}
- (void)udpSocket:(GCDAsyncUdpSocket *)sock didSendDataWithTag:(long)tag
{
    NSLog(@"Message send success!");
}

- (void)udpSocketDidClose:(GCDAsyncUdpSocket *)sock withError:(NSError *)error
{
    NSLog(@"udp socket closed!");
}

//TCP連線
////////////////////////////////////////////////////////////////

//初始化socket通信
- (GCDAsyncSocket *)asyncSocketInit:(GCDAsyncSocket *)asyncSocket{
    asyncSocket = [[GCDAsyncSocket alloc] initWithDelegate:self delegateQueue:dispatch_get_main_queue()];
    return asyncSocket;
}
//初始化socket通信
- (void)initAsyncListenSocket {
    //初始化udp
   // AsyncSocket *tempSocket=[[AsyncSocket alloc] initWithDelegate:self];
   // self.listenSocket = tempSocket;
    self.listenSocket = [[GCDAsyncSocket alloc]initWithDelegate:self delegateQueue:dispatch_get_main_queue()];
    //绑定端口
    NSError *error = nil;
    if(![self.listenSocket acceptOnPort:kTCP_PORT error:&error])
    {
        NSLog(@"Error: %@", error);
    }
}


-(BOOL)closeAsyncSocket {
    for (GCDAsyncSocket *socket in self.connectedSockets) {
        [socket setDelegate:nil];
        [socket disconnect];
    }
   // [self.connectedHost setDelegate:nil];
    //[self.connectedHost disconnect];
    return YES;
}

#pragma mark AsyncSocket Delegate Methods

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

- (void)processWithMsgType:(NSString *)msgType msg:(NSString *)msg from:(GCDAsyncSocket *)sock
{
    if ([self.character  isEqual: Master]) {

    } else if ([self.character  isEqual: Slave]) {
        if ([msgType isEqualToString:@"friendin"]) {
            if ([self.communicationMode  isEqual: P2P]) {
                [self connectToOther:msg];
            } else if ([self.communicationMode isEqualToString:CS]){
               // [self.connectedSockets addObject:2];
            }
            self.connectedNumbers = self.connectedNumbers + 1;
            [sock readDataWithTimeout:-1 tag:0];
        }
        if ([msgType isEqualToString:@"start"]) {
            self.start = YES;
        }
        if ([msgType isEqualToString:@"friendsNum"]) {
          //  if (self.connectedNumbers <= 2) {               //??
                self.connectedNumbers = [msg integerValue];
          //  } else {
          //      self.connectedNumbers = [msg integerValue] + self.connectedNumbers-2;
          //  }
            [sock readDataWithTimeout:-1 tag:0];
        }
    }
}

- (void)socket:(GCDAsyncSocket *)sock didReadData:(NSData *)data withTag:(long)tag;
{
    NSString* reseiveStr;
    NSString* msgType =[self unpackWithInput:data output:&reseiveStr];
    [self processWithMsgType:msgType msg:reseiveStr from:sock];
}
- (void)socket:(GCDAsyncSocket *)sock didAcceptNewSocket:(GCDAsyncSocket *)newSocket
{
    NSLog(@"didAcceptNewTCPSocket");
    self.connectedNumbers = self.connectedNumbers + 1;
    if ([self.character isEqualToString:Master]) {
        if ([self.communicationMode  isEqual: @"P2P"]) {
            ;
        }
        NSString *str = [[NSString alloc]initWithFormat:@"friendin/%@",newSocket.connectedHost];
        NSData * strData = [str dataUsingEncoding:NSUTF8StringEncoding];
        for (GCDAsyncSocket *friendScoket in self.connectedSockets) {
            [friendScoket writeData:strData withTimeout:-1 tag:0];
        }
        NSString *str2 = [[NSString alloc]initWithFormat:@"friendsNum/%d",self.connectedNumbers];
        NSData * strData2 = [str2 dataUsingEncoding:NSUTF8StringEncoding];
        [newSocket writeData:strData2 withTimeout:-1 tag:0];
    }
    [self.connectedSockets addObject:newSocket];
    if ([self.communicationMode  isEqual: @"P2P"]) {
        if ([self.character isEqualToString:Slave]) {
            // NSError *error = nil;
            //[self.asyncSocket acceptOnPort:kTCP_PORT error:&error];
            [self.listenSocket disconnect];
            self.listenSocket = nil;
        }
    }
}

- (void)socket:(GCDAsyncSocket *)sock didConnectToHost:(NSString *)host port:(uint16_t)port
{
    NSLog(@"TCP connect to ip: %@!",host);
    BOOL flag = NO;
    for (GCDAsyncSocket *connection in self.connectedSockets) {
        if ([connection.connectedHost isEqualToString:host]) {
            flag = YES;
        }
    }
    if (!flag) {
        [sock readDataWithTimeout:-1 tag:0];
        [self.connectedSockets addObject:sock];
        self.connectedNumbers = self.connectedNumbers + 1;
    }
}
- (void)socketDidDisconnect:(GCDAsyncSocket *)sock withError:(NSError *)err
{
    NSLog(@"TCP onSocketDidDisconnect");
    [self.connectedSockets removeObject:sock];
    self.connectedNumbers = self.connectedNumbers - 1;
}
- (void)socket:(GCDAsyncSocket *)sock didWriteDataWithTag:(long)tag
{
    NSLog(@"TCP message send to %@ success!",[sock connectedHost]);
}

@end
