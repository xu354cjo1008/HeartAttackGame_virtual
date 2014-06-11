//
//  ViewController.m
//  HeartAttackGame
//
//  Created by 李勁璋 on 2013/11/27.
//  Copyright (c) 2013年 李勁璋. All rights reserved.
//

#import "HeartAttackGameViewController.h"
#import "HeartAttackTestGame.h"
#import "PlayingCard.h"
#import "PlayingCardView.h"
#import "Player.h"
@interface HeartAttackGameViewController () <HeartAttackTestGameDelegate>
@property (strong, nonatomic) IBOutlet UIView *heartAttackGameView;
@property (nonatomic) HeartAttackTestGame *heartAttackGame;
@property (strong, nonatomic)IBOutletCollection(UILabel) NSMutableArray *scoresLable;
@property (weak, nonatomic) IBOutlet PlayingCardView *playingCardView;
@property (weak, nonatomic) IBOutlet UILabel *gameCounter;
@property (weak, nonatomic) IBOutlet UIButton *flipButton;
@end

@implementation HeartAttackGameViewController
@synthesize heartAttackGameView;

#define INITSTATE 0
#define GAMESTATE 1
#define JUDGESTATE 2

- (NSMutableArray *)scoresLable
{
    if(!_scoresLable) _scoresLable = [[NSMutableArray alloc] init];
    return _scoresLable;
}

- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
    self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
    if (self) {
        // Custom initialization
    }
    return self;
}

- (void)viewDidLoad
{
    [super viewDidLoad];
	// Do any additional setup after loading the view, typically from a nib.
    [UIApplication sharedApplication].idleTimerDisabled = YES;
  /*  [[NSBundle mainBundle] loadNibNamed:@"HeartAttackGameView" owner:self options:nil];
    [self.view addSubview:self.heartAttackGameView];*/
    [self.heartAttackGame setDelegate:self];
    [self.heartAttackGame startGame];
}
-(void)viewDidAppear:(BOOL)animated
{
    for (int i = 0; i<self.heartAttackGame.players.count; i++) {
        if (i == 0) {
            UILabel *lable = [[UILabel alloc] initWithFrame:CGRectMake(10, 50, 200, 50)];
            lable.text = [[NSString alloc] initWithFormat:@"player %d",i];
            [self.view addSubview:lable];
            [self.scoresLable addObject:lable];
        } else if (i == 1) {
            UILabel *lable = [[UILabel alloc] initWithFrame:CGRectMake(self.view.frame.size.width-100, 50, 200, 50)];
            lable.text = [[NSString alloc] initWithFormat:@"player %d",i];
            [self.view addSubview:lable];
            [self.scoresLable addObject:lable];
        } else if (i == 2) {
            UILabel *lable = [[UILabel alloc] initWithFrame:CGRectMake(10, self.view.frame.size.height-160, 200, 50)];
            lable.text = [[NSString alloc] initWithFormat:@"player %d",i];
            [self.view addSubview:lable];
            [self.scoresLable addObject:lable];
        } else if (i == 3) {
            UILabel *lable = [[UILabel alloc] initWithFrame:CGRectMake(self.view.frame.size.width-100,self.view.frame.size.height-160, 200, 50)];
            lable.text = [[NSString alloc] initWithFormat:@"player %d",i];
            [self.view addSubview:lable];
            [self.scoresLable addObject:lable];
        }
    }
    [[self.scoresLable objectAtIndex:self.heartAttackGame.playerId] setTextColor:[UIColor blueColor]];
}

- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

-(void)doInitWithPlayers:(NSMutableArray *)players host:(GCDAsyncSocket *)host mode:(NSString *)mode character:(NSString*)character ptpClock:(PtpClockModel *)ptpClock
{
    self.heartAttackGame = [[HeartAttackTestGame alloc]initWithMode:mode character:character hostSocket:host PlayersSocket:players ptpClock:ptpClock];
}

-(void)doPlayerInit:(NSInteger)playerNum
{
    [self.heartAttackGame doPlayersInit:playerNum];
}
- (IBAction)flipTheCard:(UIButton *)sender {
    [self.heartAttackGame flipCard];
    [self updateUI];
}

-(void)updateUI
{
 //   if (self.heartAttackGame.GameState == GAMESTATE) {
        Card * card = self.heartAttackGame.card;
        if (card) {
            if ([card isKindOfClass:[PlayingCard class]]) {
                PlayingCard *playingCard = (PlayingCard *)card;
                
                
                self.playingCardView.rank = playingCard.rank;
                self.playingCardView.suit = playingCard.suit;
                self.playingCardView.faceUp = playingCard.isFaceUp;
                 
                self.gameCounter.text = [[NSString alloc] initWithFormat:@"%d", self.heartAttackGame.heartAttackCounter];
            }
        } else {
            self.playingCardView.faceUp = NO;
            [self.heartAttackGame stopGame];
        }
  //  }
    for (int i = 0; i<self.heartAttackGame.players.count; i++) {
        UILabel *lable = [self.scoresLable objectAtIndex:i];
        lable.text = [[NSString alloc] initWithFormat:@"%@%d:%d",@"player",i,[self.heartAttackGame.players[i] playerScore]];
    }
}

-(void)updateCard:(Card *)card count:(int)count
{
    [self updateUI];
}

- (void)updateToken:(BOOL)token
{
    self.flipButton.enabled = token;
}

-(void)firstSlapCardEventFromPlayer:(int)player
{
    [NSThread sleepForTimeInterval:1];
    [self.heartAttackGame judge];
    [self updateUI];
    [self.heartAttackGame startGame];
    
}
- (IBAction)slapTheCard:(UITapGestureRecognizer *)sender {
    [self.heartAttackGame slapTheCard];
}

@end
