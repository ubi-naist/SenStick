//
//  DeviceListTableViewController.swift
//  SenStickViewer
//
//  Created by AkihiroUehara on 2016/04/26.
//  Copyright © 2016年 AkihiroUehara. All rights reserved.
//

import UIKit
import SenStickSDK

class DeviceListTableViewController: UITableViewController {
    
    var detailViewController: DetailViewController? = nil
    
    override func viewDidLoad() {
        super.viewDidLoad()
        
        self.refreshControl = UIRefreshControl()
//        self.refreshControl?.attributedTitle = NSAttributedString(string: "Scanning")
        self.refreshControl?.addTarget(self, action: #selector(onRefresh), forControlEvents: .ValueChanged)
    }
    
    override func viewWillAppear(animated: Bool) {
        super.viewWillAppear(animated)
        SenStickDeviceManager.sharedInstance.addObserver(self, forKeyPath: "devices", options: .New, context:nil)
    }
    
    override func viewWillDisappear(animated: Bool) {
        super.viewWillDisappear(animated)
        SenStickDeviceManager.sharedInstance.removeObserver(self, forKeyPath: "devices")
    }
    
    override func didReceiveMemoryWarning() {
        super.didReceiveMemoryWarning()
        // Dispose of any resources that can be recreated.
    }
    
    // MARK
    func onRefresh()
    {
        SenStickDeviceManager.sharedInstance.scan(5.0, callback: { (remaining: NSTimeInterval)  in
//            debugPrint("\(#function)")
            if remaining <= 0 {
                self.refreshControl?.endRefreshing()
            }
        })
    }
    
    // MARK: - Segues    
    override func prepareForSegue(segue: UIStoryboardSegue, sender: AnyObject?) {
        if let dataview = segue.destinationViewController as? SensorDataViewController {
            let indexPath = self.tableView.indexPathForSelectedRow!
            dataview.device = SenStickDeviceManager.sharedInstance.devices[indexPath.row]
        }
        /*
         // 遷移時にステートを設定
         if([segue.destinationViewController isKindOfClass:[StateDetailTableViewController class]]) {
         StateDetailTableViewController *target = (StateDetailTableViewController *)segue.destinationViewController;
         NSIndexPath *indexPath = self.tableView.indexPathForSelectedRow;
         target.soundState = [[SoundStateManager sharedInstance] getState:(int)indexPath.section buttonIndex:(int)indexPath.row];
         }
         */
        /*
        if segue.identifier == "showDetail" {
            if let indexPath = self.tableView.indexPathForSelectedRow {
                let object = objects[indexPath.row] as! NSDate
                let controller = (segue.destinationViewController as! UINavigationController).topViewController as! DetailViewController
                controller.detailItem = object
                controller.navigationItem.leftBarButtonItem = self.splitViewController?.displayModeButtonItem()
                controller.navigationItem.leftItemsSupplementBackButton = true
            }
        }*/
    }
    
    // MARK: - KVO
    override func observeValueForKeyPath(keyPath: String?, ofObject object: AnyObject?, change: [String : AnyObject]?, context: UnsafeMutablePointer<Void>) {
        debugPrint("\(#function)")
        if (keyPath == "devices") {
            self.tableView.reloadData()
        } else {
            super.observeValueForKeyPath(keyPath, ofObject: object, change: change, context: context)
        }
    }
    
    // MARK: - Table View
    override func numberOfSectionsInTableView(tableView: UITableView) -> Int {
        return 1
    }
    
    override func tableView(tableView: UITableView, numberOfRowsInSection section: Int) -> Int {
        debugPrint("\(#function) count: \( SenStickDeviceManager.sharedInstance.devices.count)")
        return SenStickDeviceManager.sharedInstance.devices.count
    }
    
    override func tableView(tableView: UITableView, cellForRowAtIndexPath indexPath: NSIndexPath) -> UITableViewCell {
        let cell = tableView.dequeueReusableCellWithIdentifier("deviceCell", forIndexPath: indexPath) as! DeviceListCellView

        cell.device = SenStickDeviceManager.sharedInstance.devices[indexPath.row]
        
        return cell
    }
    override func tableView(tableView: UITableView, heightForRowAtIndexPath indexPath: NSIndexPath) -> CGFloat {
        return 74
    }
}
