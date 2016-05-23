//
//  SensorDataViewController.swift
//  SenStickViewer
//
//  Created by AkihiroUehara on 2016/05/22.
//  Copyright © 2016年 AkihiroUehara. All rights reserved.
//

import UIKit
import SenStickSDK
import CoreMotion

class SensorDataViewController : UITableViewController, SenStickDeviceDelegate {
    var device: SenStickDevice?
    
    var statusCell: SensorStatusCellView?
    var accelerationSensorDataController: SensorDataController<CMAcceleration, AccelerationRange>?
    
    override func viewDidLoad() {
        super.viewDidLoad()
        self.tableView.delegate = self
        
        self.accelerationSensorDataController = SensorDataController<CMAcceleration, AccelerationRange>()
    }
    
    override func viewWillAppear(animated: Bool) {
        super.viewWillAppear(animated)

        device?.delegate = self
        device?.connect()
        
        self.statusCell?.updateView()
        self.accelerationSensorDataController?.service = device?.accelerationSensorService
    }
    
    override func viewWillDisappear(animated: Bool) {
        super.viewWillDisappear(animated)

        device?.delegate = nil
    }
    
    override func prepareForSegue(segue: UIStoryboardSegue, sender: AnyObject?) {
    }
    
    // MARK: - SenStickDeviceDelegate
    func didIsConnectedChanged(sender: SenStickDevice, isConnected: Bool) {
        self.statusCell?.updateView()
        self.accelerationSensorDataController?.service = device?.accelerationSensorService
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

        if let s = cell as? SensorStatusCellView {
            s.device        = self.device
            self.statusCell = s
        }
//        debugPrint("\(#function) \(cell)")
        return cell
    }
    
    override func tableView(tableView: UITableView, heightForRowAtIndexPath indexPath: NSIndexPath) -> CGFloat {
        return 148
    }
}
