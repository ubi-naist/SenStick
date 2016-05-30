//
//  DeviceListTableViewController.swift
//  SenStickViewer
//
//  Created by AkihiroUehara on 2016/04/26.
//  Copyright © 2016年 AkihiroUehara. All rights reserved.
//

import UIKit
import SenStickSDK

class DeviceListTableViewController: UITableViewController, SenStickDeviceDelegate {
    
    var detailViewController: DetailViewController? = nil
    
    override func viewDidLoad() {
        super.viewDidLoad()
        
        self.refreshControl = UIRefreshControl()
        self.refreshControl?.addTarget(self, action: #selector(onRefresh), forControlEvents: .ValueChanged)
    }
    
    override func viewWillAppear(animated: Bool) {
        super.viewWillAppear(animated)

        self.tableView.alpha = 1
        self.tableView.userInteractionEnabled = true
        
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
    }
    
    // MARK: - KVO
    override func observeValueForKeyPath(keyPath: String?, ofObject object: AnyObject?, change: [String : AnyObject]?, context: UnsafeMutablePointer<Void>) {
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
    
    override func tableView(tableView: UITableView, didSelectRowAtIndexPath indexPath: NSIndexPath) {
        let device = SenStickDeviceManager.sharedInstance.devices[indexPath.row]

        // 接続していれば画面遷移
        if device.isConnected {
            performSegueWithIdentifier("dataView", sender: self)
        } else {
            // 今のVCを半透明、操作無効
            self.tableView.alpha = 0.5
            self.tableView.userInteractionEnabled = false
            self.refreshControl?.endRefreshing()
            
            device.delegate = self
            device.connect()
            
            dispatch_after(dispatch_time(DISPATCH_TIME_NOW, Int64(5 * NSEC_PER_SEC)), dispatch_get_main_queue(), {
                if !device.isConnected {
                    device.cancelConnection()
                    self.showAlert()
                }
            })
        }
    }
    
    func showAlert()
    {
        self.tableView.alpha = 1
        self.tableView.userInteractionEnabled = true
        
        let alert = UIAlertController(title: "Failed to connect", message: "Failed to connect.", preferredStyle: .Alert)
        let okAction = UIAlertAction(title: "OK", style: .Default, handler: nil)
        alert.addAction(okAction)
        
        presentViewController(alert, animated: true, completion: nil)
    }
    
    // public protocol SenStickDeviceDelegate
    func didServiceFound(sender:SenStickDevice)
    {
    }
    
    func didConnected(sender:SenStickDevice)
    {
        performSegueWithIdentifier("dataView", sender: self)
    }
    
    func didDisconnected(sender:SenStickDevice)
    {
        showAlert()
    }
}
