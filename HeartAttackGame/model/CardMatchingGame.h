//
//  CardMatchingGame.h
//  Matchismo
//
//  Created by 李勁璋 on 13/9/29.
//  Copyright (c) 2013年 李勁璋. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "Card.h"
#import "Deck.h"

@interface CardMatchingGame : NSObject

//designated initializer
- (id)initWithCardCount:(NSUInteger)count
              usingDeck:(Deck *)deck;

- (void)flipCardAtIndex:(NSUInteger)index;

- (Card *)cardAtIndex : (NSUInteger)index;

@property (readonly, nonatomic) int score;

@end
