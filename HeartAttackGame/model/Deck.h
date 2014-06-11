//
//  Deck.h
//  Matchismo
//
//  Created by JJLee on 13/9/24.
//  Copyright (c) 2013年 EE. All rights reserved.
//

//#import <Foundation/Foundation.h>
#import "Card.h"

@interface Deck : NSObject

- (void)addCard:(Card *)card atTop:(BOOL)atTop;
- (Card *)drawRandomCard;
- (NSUInteger) amount;

@end
