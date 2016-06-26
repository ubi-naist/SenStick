//
//  logListViewController.swift
//  SenStickViewer
//
//  Created by AkihiroUehara on 2016/06/26.
//  Copyright © 2016年 AkihiroUehara. All rights reserved.
//

import UIKit
import SenStickSDK

class LogListViewController: UITableViewController, SenStickDeviceDelegate, SenStickMetaDataServiceDelegate {

    var device: SenStickDevice?
    
    // View Controller life cycle
    override func viewWillAppear(animated: Bool) {
        super.viewWillAppear(animated)
        
        self.device?.delegate = self
        self.device?.metaDataService?.delegate = self
    }
    
    // SenStickDeviceDelegate
    func didServiceFound(sender: SenStickSDK.SenStickDevice)
    {
    }
    func didConnected(sender: SenStickSDK.SenStickDevice)
    {
    }
    func didDisconnected(sender: SenStickSDK.SenStickDevice)
    {
        self.navigationController?.popToRootViewControllerAnimated(true)
    }
    
    // SenStickSensorServiceDelegate
    func didUpdateMetaData(sender:SenStickMetaDataService)
    {
        if let cell = self.tableView.cellForRowAtIndexPath(NSIndexPath(forRow: Int(sender.logID), inSection: 0)) {
            cell.textLabel?.text = "Log ID:\(sender.logID)"

            let formatter = NSDateFormatter()
            formatter.dateFormat = "  yyyy年MM月dd日 HH時mm分ss秒 Z"
            cell.detailTextLabel?.text = formatter.stringFromDate(sender.dateTime)
        }
    }
    
    // segue
    override func prepareForSegue(segue: UIStoryboardSegue, sender: AnyObject?) {
        if let vc = segue.destinationViewController as? LogReaderViewController {
            vc.device = self.device
            vc.logID  = UInt8(self.tableView.indexPathForSelectedRow!.row)
        }
    }
    
    // MARK: - Table View
    override func numberOfSectionsInTableView(tableView: UITableView) -> Int
    {
        return 1
    }
    
    override func tableView(tableView: UITableView, numberOfRowsInSection section: Int) -> Int
    {
        if device?.controlService?.command == .Stopping {
            return Int(device!.controlService!.availableLogCount)
        } else {
            return max(0, Int(device!.controlService!.availableLogCount) - 1)
        }
    }
    
    override func tableView(tableView: UITableView, cellForRowAtIndexPath indexPath: NSIndexPath) -> UITableViewCell {
        let cell = tableView.dequeueReusableCellWithIdentifier("logListCell", forIndexPath: indexPath)
        cell.textLabel?.text = "Log ID:\(indexPath.row)"
        
        device?.metaDataService?.requestMetaData(UInt8(indexPath.row))
        
        return cell
    }
}
