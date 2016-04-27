//
//  DeviceTableView.swift
//  SenStickViewer
//
//  Created by AkihiroUehara on 2016/04/26.
//  Copyright © 2016年 AkihiroUehara. All rights reserved.
//

import UIKit
import SenStickSDK

class DeviceTableViewController: UITableViewController {
    
    var detailViewController: DetailViewController? = nil
    var objects = [AnyObject]()
    
    
    override func viewDidLoad() {
        super.viewDidLoad()
        
        self.refreshControl = UIRefreshControl()
//        self.refreshControl?.attributedTitle = NSAttributedString(string: "Scanning")
        self.refreshControl?.addTarget(self, action: #selector(onRefresh), forControlEvents: .ValueChanged)
    }
    
    override func viewWillAppear(animated: Bool) {
        super.viewWillAppear(animated)
        SenStickDeviceManager.sharedInstance.addObserver(self, forKeyPath: "devices", options: .New, context: nil)
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
            debugPrint("#(function)")
            if remaining <= 0 {
                self.refreshControl?.endRefreshing()
            }
        })
    }
    
    // MARK: - Segues    
    override func prepareForSegue(segue: UIStoryboardSegue, sender: AnyObject?) {
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
        if (context == nil) {
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
        return SenStickDeviceManager.sharedInstance.devices.count
    }
    
    override func tableView(tableView: UITableView, cellForRowAtIndexPath indexPath: NSIndexPath) -> UITableViewCell {
        let cell = tableView.dequeueReusableCellWithIdentifier("deviceCell", forIndexPath: indexPath) as! DeviceCellView

        cell.device = SenStickDeviceManager.sharedInstance.devices[indexPath.row]
        
        return cell
    }
}
