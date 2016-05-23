//
//  SensorDataViewController.swift
//  SenStickViewer
//
//  Created by AkihiroUehara on 2016/05/22.
//  Copyright © 2016年 AkihiroUehara. All rights reserved.
//

import UIKit
import SenStickSDK

class SensorDataViewController : UITableViewController, SenStickDeviceDelegate {
    weak var device: SenStickDevice?
    
    override func viewDidLoad() {
        super.viewDidLoad()
        self.tableView.delegate = self
    }
    
    override func viewWillAppear(animated: Bool) {
        super.viewWillAppear(animated)

        device?.connect()
    }
    
    override func viewWillDisappear(animated: Bool) {
        super.viewWillDisappear(animated)
    }
    
    override func prepareForSegue(segue: UIStoryboardSegue, sender: AnyObject?) {
    }
    
    // MARK: - SenStickDeviceDelegate
    func didIsConnectedChanged(sender: SenStickDevice, isConnected: Bool) {
        self.tableView.reloadData()
    }
    
    // MARK: - Table View
    override func numberOfSectionsInTableView(tableView: UITableView) -> Int {
        return 1
    }

    override func tableView(tableView: UITableView, numberOfRowsInSection section: Int) -> Int {
        return 2
    }

    override func tableView(tableView: UITableView, cellForRowAtIndexPath indexPath: NSIndexPath) -> UITableViewCell {
        var cell : UITableViewCell
        if indexPath.row == 0 {
            cell = tableView.dequeueReusableCellWithIdentifier("statusCell", forIndexPath: indexPath)
        } else {
            cell = tableView.dequeueReusableCellWithIdentifier("dataCell", forIndexPath: indexPath)
        }
        
        if let statusCell = cell as? SensorStatusCellView {
            statusCell.device = self.device
        }
        debugPrint("\(#function) \(cell)")
        return cell
    }
    
    override func tableView(tableView: UITableView, heightForRowAtIndexPath indexPath: NSIndexPath) -> CGFloat {
        return 148
    }
}
