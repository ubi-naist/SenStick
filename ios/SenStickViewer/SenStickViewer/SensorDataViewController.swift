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
    var accelerationSensorCell: AccelerationCellView?
    
    override func viewWillAppear(animated: Bool) {
        super.viewWillAppear(animated)

        device?.delegate = self
        device?.connect()
    }
    override func viewWillDisappear(animated: Bool) {
        super.viewWillDisappear(animated)

        device?.delegate = nil
    }
    
    // MARK: - SenStickDeviceDelegate
    func didServiceFound(sender: SenStickDevice) {
        if let cell = self.tableView.cellForRowAtIndexPath(NSIndexPath(indexes: [0,0], length: 2)) as? SensorStatusCellView {
            cell.name = device?.name
            cell.service = device?.controlService
        }
        
        if let cell = self.tableView.cellForRowAtIndexPath(NSIndexPath(indexes: [0,1], length: 2)) as? AccelerationCellView {
            cell.service = device?.accelerationSensorService
        }
    }
}
