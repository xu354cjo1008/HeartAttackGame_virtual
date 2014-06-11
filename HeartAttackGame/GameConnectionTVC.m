//
//  GameConnectionTVC.m
//  HeartAttackGame
//
//  Created by 李勁璋 on 2013/11/28.
//  Copyright (c) 2013年 李勁璋. All rights reserved.
//

#import "GameConnectionTVC.h"
#import "CommunicationBuilding.h"
#import "GameGroupViewController.h"
@interface GameConnectionTVC ()
@property (nonatomic) CommunicationBuilding *gameConnection;
@property (nonatomic) NSTimer *timer;
@end

@implementation GameConnectionTVC


- (void)viewDidLoad
{
    [super viewDidLoad];
    self.gameConnection = [[CommunicationBuilding alloc] initWithMode:@"CS" character:@"Slave"];
    [self.refreshControl addTarget:self action:@selector(loadGamePartyData) forControlEvents:UIControlEventValueChanged];
    self.timer = [NSTimer scheduledTimerWithTimeInterval:5              //Master不停的發送邀請廣播
                                                  target:self
                                                selector:@selector(loadGamePartyData)
                                                userInfo:nil
                                                 repeats:YES];
    [self loadGamePartyData];
}
/*
- (void)viewWillDisappear:(BOOL)animated
{
    [self.gameConnection closeCommunication];
    self.gameConnection = nil;
}
*/
-(void) viewWillDisappear:(BOOL)animated {
    if ([self.navigationController.viewControllers indexOfObject:self]==NSNotFound) {
        // Navigation button was pressed. Do some stuff
        [self.gameConnection closeCommunication];
        self.gameConnection = nil;
        [self.navigationController popViewControllerAnimated:NO];
    }
    [self.timer invalidate];
    [super viewWillDisappear:animated];
}

- (void)loadGamePartyData
{
    [self.refreshControl beginRefreshing];
    dispatch_queue_t loaderQ = dispatch_queue_create("game party loader", NULL);
    dispatch_async(loaderQ, ^{
        [self.gameConnection clearPartyBuffer];
        [NSThread sleepForTimeInterval:2.0];
        dispatch_async(dispatch_get_main_queue(), ^{
            [self.tableView reloadData];
            [self.refreshControl endRefreshing];
        });
    });

}
#pragma mark - Segue

-(void)prepareForSegue:(UIStoryboardSegue *)segue sender:(id)sender
{
    if ([sender isKindOfClass:[UITableViewCell class]]) {
        NSIndexPath *indexPath = [self.tableView indexPathForCell:sender];
        if (indexPath) {
            if ([segue.identifier isEqualToString:@"join game"]) {
                [self.gameConnection connectToHost:self.gameConnection.gamePartys[indexPath.row]];
                [self.gameConnection closeAsyncUdpSocket];
                //[self.gameConnection closeinitSocket];
                 if ([segue.destinationViewController respondsToSelector:@selector(gameConnection)]) {
                     [segue.destinationViewController setGameConnection:self.gameConnection];
                 }
            }
        }
    }
}

#pragma mark - Table view data source


- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section
{
    return [self.gameConnection.gamePartys count];
}

-(NSString *)titleForRow:(NSInteger)row
{
    return self.gameConnection.gamePartys[row];
}
/*
-(NSString *)subtitleForRow:(NSInteger)row
{
    return [self.photos[row][FLICKR_PHOTO_OWNER] description];
}
*/
- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath
{
    static NSString *CellIdentifier = @"Game Party";
    UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:CellIdentifier forIndexPath:indexPath];
   // UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:CellIdentifier];
    
    // Configure the cell...
    cell.textLabel.text = [self titleForRow:indexPath.row];
    return cell;
}


@end
