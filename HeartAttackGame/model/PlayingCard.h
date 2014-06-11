//
//  PlayingCard.h
//  Matchismo
//
//  Created by JJLee on 13/9/24.
//  Copyright (c) 2013年 EE. All rights reserved.
//

#import "Card.h"

@interface PlayingCard : Card

@property (strong, nonatomic) NSString *suit;
@property (nonatomic) NSUInteger rank;

+ (NSArray *)validSuits;
+ (NSUInteger)maxRank;

@end
