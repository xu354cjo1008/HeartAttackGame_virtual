//
//  GameGroupViewController.m
//  HeartAttackGame
//
//  Created by 李勁璋 on 2013/11/28.
//  Copyright (c) 2013年 李勁璋. All rights reserved.
//

#import "GameGroupViewController.h"
#import "CommunicationBuilding.h"
#import "PtpClockModel.h"
#import "HeartAttackGameViewController.h"

@interface GameGroupViewController () <UITableViewDelegate, UITableViewDataSource>
@property (nonatomic) CommunicationBuilding *gameConnection;
@property (nonatomic) PtpClockModel *ptpClock;
@property (nonatomic) NSTimer *timer;
@property (weak, nonatomic) IBOutlet UITableView *tableView;
@property (nonatomic) IBOutlet UIView *gameView;
@end

@implementation GameGroupViewController

- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
    self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
    if (self) {
        // Custom initialization
    }
    return self;
}

#define Master 1
- (void)viewDidLoad
{
    [super viewDidLoad];
	// Do any additional setup after loading the view.
    self.gameConnection = [[CommunicationBuilding alloc] initWithMode:@"CS" character:@"Master"];
    self.ptpClock = [[PtpClockModel alloc] init];
    [self.ptpClock startTheClock:Master];
    self.timer = [NSTimer scheduledTimerWithTimeInterval:1              //Master不停的發送邀請廣播
                                                  target:self
                                                selector:@selector(reloadData)
                                                userInfo:nil
                                                 repeats:YES];
}

-(void) viewWillDisappear:(BOOL)animated {
    if ([self.navigationController.viewControllers indexOfObject:self]==NSNotFound) {
        // Navigation button was pressed. Do some stuff
        [self.gameConnection closeCommunication];
        self.gameConnection = nil;
        [self.ptpClock stopTheClock];
        self.ptpClock = nil;
        [self.navigationController popViewControllerAnimated:NO];
    }
    [self.timer invalidate];
    [super viewWillDisappear:animated];
}

-(void)reloadData
{
    [self.tableView reloadData];
}

- (IBAction)startGameButton:(UIButton *)sender {
    [self.gameConnection closeinitSocket];
    HeartAttackGameViewController *gameViewController = [[HeartAttackGameViewController alloc] initWithNibName:@"HeartAttackGameView" bundle:nil];
    [gameViewController doInitWithPlayers:self.gameConnection.connectedSockets host:self.gameConnection.connectedHost mode:@"CS" character:@"Master"  ptpClock:self.ptpClock];
    [self.view addSubview:gameViewController.view];
}


#pragma mark - Table view data source


- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section
{
    return [self.gameConnection.connectedSockets count];
}

-(NSString *)titleForRow:(NSInteger)row
{
    return [self.gameConnection.connectedSockets[row] connectedHost];
}
- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath
{
    static NSString *CellIdentifier = @"Game Partner";
    UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:CellIdentifier forIndexPath:indexPath];
    // UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:CellIdentifier];
    
    // Configure the cell...
    cell.textLabel.text = [self titleForRow:indexPath.row];
    return cell;
}
@end
