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
    var gyroSensorCell: GyroCellView?
    var magneticFieldCell: MagneticFieldCellView?
    var brightnessCell: BrightnessCellView?
    var uvCell: UVCellView?
    var humidityCell: HumidityCellView?
    var pressureCell: PressureCellView?
    
    override func viewWillAppear(animated: Bool) {
        super.viewWillAppear(animated)
        
        device?.delegate = self
        device?.connect()
        if device != nil {
            didServiceFound(device!)
        }
    }
    
    override func viewWillDisappear(animated: Bool) {
        super.viewWillDisappear(animated)
        
        device?.delegate = nil
    }
    
    func startToReadLog(logid: UInt8) {
        accelerationSensorCell?.startToReadLog(logid)
        gyroSensorCell?.startToReadLog(logid)
        magneticFieldCell?.startToReadLog(logid)
        brightnessCell?.startToReadLog(logid)
        uvCell?.startToReadLog(logid)
        humidityCell?.startToReadLog(logid)
        pressureCell?.startToReadLog(logid)
    }
    
    func setServices(target :AnyObject)
    {
        if let cell = target as? SensorStatusCellView {
            statusCell = cell
            cell.controller = self
            cell.name = device?.name
            cell.service = device?.controlService
        }
        
        if let cell = target as? AccelerationCellView {
            accelerationSensorCell = cell
            cell.service = device?.accelerationSensorService
        }
        
        if let cell = target as? GyroCellView {
            gyroSensorCell = cell
            cell.service = device?.gyroSensorService
        }
        
        if let cell = target as? MagneticFieldCellView {
            magneticFieldCell = cell
            cell.service = device?.magneticFieldSensorService
        }
        
        if let cell = target as? BrightnessCellView {
            brightnessCell = cell
            cell.service = device?.brightnessSensorService
        }
        
        if let cell = target as? UVCellView {
            uvCell = cell
            cell.service = device?.uvSensorService
        }
        
        if let cell = target as? HumidityCellView {
            humidityCell = cell
            cell.service = device?.humiditySensorService
        }
        
        if let cell = target as? PressureCellView {
            pressureCell = cell
            cell.service = device?.pressureSensorService
        }
        
    }
    // MARK: - SenStickDeviceDelegate
    func didServiceFound(sender: SenStickDevice) {
        if let cell = self.tableView.cellForRowAtIndexPath(NSIndexPath(indexes: [0,0], length: 2)) as? SensorStatusCellView {
            statusCell = cell
            cell.controller = self
            cell.name = device?.name
            cell.service = device?.controlService
        }
        
        if let cell = self.tableView.cellForRowAtIndexPath(NSIndexPath(indexes: [0,1], length: 2)) as? AccelerationCellView {
            accelerationSensorCell = cell
            cell.service = device?.accelerationSensorService
        }
        
        if let cell = self.tableView.cellForRowAtIndexPath(NSIndexPath(indexes: [0,2], length: 2)) as? GyroCellView {
            gyroSensorCell = cell
            cell.service = device?.gyroSensorService
        }
        
        if let cell = self.tableView.cellForRowAtIndexPath(NSIndexPath(indexes: [0,3], length: 2)) as? MagneticFieldCellView {
            magneticFieldCell = cell
            cell.service = device?.magneticFieldSensorService
        }
        
        if let cell = self.tableView.cellForRowAtIndexPath(NSIndexPath(indexes: [0,4], length: 2)) as? BrightnessCellView {
            brightnessCell = cell
            cell.service = device?.brightnessSensorService
        }
        
        if let cell = self.tableView.cellForRowAtIndexPath(NSIndexPath(indexes: [0,5], length: 2)) as? UVCellView {
            uvCell = cell
            cell.service = device?.uvSensorService
        }
        
        if let cell = self.tableView.cellForRowAtIndexPath(NSIndexPath(indexes: [0,6], length: 2)) as? HumidityCellView {
            humidityCell = cell
            cell.service = device?.humiditySensorService
        }
        
        if let cell = self.tableView.cellForRowAtIndexPath(NSIndexPath(indexes: [0,7], length: 2)) as? PressureCellView {
            pressureCell = cell
            cell.service = device?.pressureSensorService
        }
    }
    
    func didConnected(sender:SenStickDevice)
    {}
    
    func didDisconnected(sender:SenStickDevice)
    {
        self.navigationController?.popToRootViewControllerAnimated(true)
    }
    
    override func tableView(tableView: UITableView, willDisplayCell cell: UITableViewCell, forRowAtIndexPath indexPath: NSIndexPath) {
        setServices(cell)
    }
    
    // MARK: - Segues
    override func shouldPerformSegueWithIdentifier(identifier: String, sender: AnyObject?) -> Bool {
        // 詳細表示遷移できるのはログ停止時だけ
        if let control = device?.controlService {
            return (control.command == .Stopping)
        } else {
            return false
        }
    }
    
    override func prepareForSegue(segue: UIStoryboardSegue, sender: AnyObject?) {
        
        debugPrint("  \(segue.destinationViewController)")
        if let vc = segue.destinationViewController as? SamplingDurationViewController {
            let indexPath = self.tableView.indexPathForSelectedRow!
            switch(indexPath.row) {
            case 1:
                vc.target = device?.accelerationSensorService
            case 2:
                vc.target = device?.gyroSensorService
            case 3:
                vc.target = device?.magneticFieldSensorService
            case 4:
                vc.target = device?.brightnessSensorService
            case 5:
                vc.target = device?.uvSensorService
            case 6:
                vc.target = device?.humiditySensorService
            case 7:
                vc.target = device?.pressureSensorService
            default: break
            }
        }
    }
    
}
