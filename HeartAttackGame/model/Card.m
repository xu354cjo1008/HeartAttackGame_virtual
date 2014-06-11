//
//  Card.m
//  Matchismo
//
//  Created by JJLee on 13/9/24.
//  Copyright (c) 2013年 EE. All rights reserved.
//

#import "Card.h"

@implementation Card

@synthesize faceUp = _faceUp;
@synthesize unplayable = _unplayable;


-(int)match:(NSArray *)otherCards
{
    int score = 0;
    
    for (Card *card in otherCards) {
        if ([card.contents isEqualToString:self.contents])
        {
            score=1;
        }
    }
    return score;
}

@end
