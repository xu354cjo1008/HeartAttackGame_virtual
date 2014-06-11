//
//  Card.h
//  Matchismo
//
//  Created by JJLee on 13/9/24.
//  Copyright (c) 2013年 EE. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface Card : NSObject
@property (strong) NSString *contents;

@property (nonatomic,getter = isFaceUp) BOOL faceUp;
@property (nonatomic, getter = isUnplayable) BOOL unplayable;

-(int)match:(NSArray *)otherCards;

@end
