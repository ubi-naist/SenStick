//
//  LogReaderViewController.swift
//  SenStickViewer
//
//  Created by AkihiroUehara on 2016/06/26.
//  Copyright © 2016年 AkihiroUehara. All rights reserved.
//

import UIKit
import SenStickSDK

class LogReaderViewController: UITableViewController, SenStickDeviceDelegate {
    var device: SenStickDevice?
    var logID: UInt8?
    
    var accelerationDataModel:  AccelerationDataModel?
    var gyroDataModel:          GyroDataModel?
    var magneticFieldDataModel: MagneticFieldDataModel?
    var brightnessDataModel:    BrightnessDataModel?
    var uvDataModel:            UVDataModel?
    var humidityDataModel:      HumidityDataModel?
    var pressureDataModel:      PressureDataModel?
    
    // View controller life cycle
    override func viewDidLoad()
    {
        super.viewDidLoad()
        
        accelerationDataModel  = AccelerationDataModel()
        gyroDataModel          = GyroDataModel()
        magneticFieldDataModel = MagneticFieldDataModel()
        brightnessDataModel    = BrightnessDataModel()
        uvDataModel            = UVDataModel()
        humidityDataModel      = HumidityDataModel()
        pressureDataModel      = PressureDataModel()
    }
    
    override func viewDidAppear(animated: Bool) {
        super.viewWillAppear(animated)
        
        device?.delegate = self
        setServices()
        startToReadLog(self.logID!)
    }
    
    override func viewWillDisappear(animated: Bool) {
        super.viewWillDisappear(animated)

        device?.delegate = nil
    }

    // MARK: - SenStickDeviceDelegate
    
    func didServiceFound(sender: SenStickDevice)
    {
    }
    
    func didConnected(sender:SenStickDevice)
    {
    }
    
    func didDisconnected(sender:SenStickDevice)
    {
        self.navigationController?.popToRootViewControllerAnimated(true)
    }
    
    // methods
    
    func setServices()
    {
        accelerationDataModel?.service  = device?.accelerationSensorService
        gyroDataModel?.service          = device?.gyroSensorService
        magneticFieldDataModel?.service = device?.magneticFieldSensorService
        brightnessDataModel?.service    = device?.brightnessSensorService
        uvDataModel?.service            = device?.uvSensorService
        humidityDataModel?.service      = device?.humiditySensorService
        pressureDataModel?.service      = device?.pressureSensorService
    }
    
    func startToReadLog(logid: UInt8)
    {
        accelerationDataModel?.startToReadLog(logid)
        gyroDataModel?.startToReadLog(logid)
        magneticFieldDataModel?.startToReadLog(logid)
        brightnessDataModel?.startToReadLog(logid)
        uvDataModel?.startToReadLog(logid)
        humidityDataModel?.startToReadLog(logid)
        pressureDataModel?.startToReadLog(logid)
    }
    
    // table view source/delegate
    override func tableView(tableView: UITableView, willDisplayCell cell: UITableViewCell, forRowAtIndexPath indexPath: NSIndexPath)
    {
        switch (indexPath.row) {
        case 0:
            accelerationDataModel?.cell  = cell as? SensorDataCellView
        case 1:
            gyroDataModel?.cell          = cell as? SensorDataCellView
        case 2:
            magneticFieldDataModel?.cell = cell as? SensorDataCellView
        case 3:
            brightnessDataModel?.cell    = cell as? SensorDataCellView
        case 4:
            uvDataModel?.cell            = cell as? SensorDataCellView
        case 5:
            humidityDataModel?.cell      = cell as? SensorDataCellView
        case 6:
            pressureDataModel?.cell      = cell as? SensorDataCellView
        default: break
        }
    }
}

