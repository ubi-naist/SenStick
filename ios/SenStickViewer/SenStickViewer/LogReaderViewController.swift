//
//  LogReaderViewController.swift
//  SenStickViewer
//
//  Created by AkihiroUehara on 2016/06/26.
//  Copyright © 2016年 AkihiroUehara. All rights reserved.
//

import UIKit
import SenStickSDK

class LogReaderViewController: UITableViewController, SenStickDeviceDelegate, SensorDataModelDelegate {
    var device: SenStickDevice?
    var logID: UInt8?
    var dataModels : [SensorDataModel]?
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
        
        accelerationDataModel  = AccelerationDataModel(self)
        gyroDataModel          = GyroDataModel(self)
        magneticFieldDataModel = MagneticFieldDataModel(self)
        brightnessDataModel    = BrightnessDataModel(self)
        uvDataModel            = UVDataModel(self)
        humidityDataModel      = HumidityDataModel(self)
        pressureDataModel      = PressureDataModel(self)

        dataModels = [accelerationDataModel!, gyroDataModel!, magneticFieldDataModel!, brightnessDataModel!, uvDataModel!, humidityDataModel!, pressureDataModel!]
    }
    
    override func viewDidAppear(animated: Bool) {
        super.viewDidAppear(animated)
        
        device?.delegate = self

        // set services
        accelerationDataModel?.service  = device?.accelerationSensorService
        gyroDataModel?.service          = device?.gyroSensorService
        magneticFieldDataModel?.service = device?.magneticFieldSensorService
        brightnessDataModel?.service    = device?.brightnessSensorService
        uvDataModel?.service            = device?.uvSensorService
        humidityDataModel?.service      = device?.humiditySensorService
        pressureDataModel?.service      = device?.pressureSensorService

        for model in dataModels! {
            model.startToReadLog(self.logID!)
        }
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
    
    // SensorDataModelDelegate
    func didStopReadingLog(sender: SensorDataModel)
    {
        debugPrint("\(#function) \(sender.sensorName).")
    }
    
    // table view source/delegate
    override func tableView(tableView: UITableView, willDisplayCell cell: UITableViewCell, forRowAtIndexPath indexPath: NSIndexPath)
    {
        let dataCell = cell as? SensorDataCellView
        
        dataCell?.iconButton?.userInteractionEnabled = false
        dataModels![indexPath.row].cell = dataCell
    }
}

