//
//  GyroCellView.swift
//  SenStickViewer
//
//  Created by AkihiroUehara on 2016/05/25.
//  Copyright © 2016年 AkihiroUehara. All rights reserved.
//

import UIKit
import SenStickSDK
import CoreMotion

class GyroDataModel : SensorDataModel
{
    weak var service: GyroSensorService? {
        didSet {
            self.service?.delegate = self
            didUpdateSetting(self)
        }
    }
    
    override func startToReadLog(logid: UInt8)
    {
        service?.readLogData()
        super.startToReadLog(logid)
        
        let logID = SensorLogID(logID: logid, skipCount: 0, position: 0)
        service?.writeLogID(logID)
    }
    
    // MARK: - SenStickSensorServiceDelegate
    override func didUpdateSetting(sender:AnyObject)
    {
        cell?.iconButton?.enabled = (self.service != nil)
        cell?.iconButton?.selected = (service?.settingData?.status != .Stopping)
        
        // レンジの更新
        //        let k = M_PI / Double(180), 1/60
        if let setting = service?.settingData {
            self.duration = setting.samplingDuration
            switch(setting.range) {
            case .ROTATION_RANGE_250DPS:
                self.maxValue = 5
                self.minValue = -5
                
            case .ROTATION_RANGE_500DPS:
                self.maxValue = 10
                self.minValue = -10
                
            case .ROTATION_RANGE_1000DPS:
                self.maxValue = 20
                self.minValue = -20
                
            case .ROTATION_RANGE_2000DPS:
                self.maxValue = 40
                self.minValue = -40
            }
        }
    }
    
    override func didUpdateRealTimeData(sender: AnyObject)
    {
        if let data = service?.realtimeData {
            drawRealTimeData([data.x, data.y, data.z])
        }
    }
    
    override func didUpdateMetaData(sender: AnyObject)
    {
        //        debugPrint("\(#function), availableCount: \(service!.logMetaData!.availableSampleCount)")
        self.duration = (service?.logMetaData?.samplingDuration)!
        
        let count = (service?.logMetaData?.availableSampleCount)!
        cell?.graphView?.sampleCount = Int(count)
        cell?.iconButton?.enabled    = (count != 0)
        cell?.iconButton?.selected   = (count != 0)
        cell?.progressBar?.hidden    = (count == 0)
    }
    
    override func didUpdateLogData(sender: AnyObject)
    {
        //        debugPrint("\(#function) \(sender)")
        if let array = service?.readLogData() {
            //        debugPrint("     \(array.count)")
            for data in array {
                addReadLog([data.x, data.y, data.z])
            }
        }
    }
    
    override func didFinishedLogData(sender: AnyObject)
    {
        //        debugPrint("\(#function) availableCount: \(service!.logMetaData!.availableSampleCount) read count:\(super.logData[0].count)")
        stopReadingLog("gyro", duration: service?.logMetaData?.samplingDuration)
    }
    
    
    // MARK: - Event handler
    override func  iconButtonToutchUpInside(sender: UIButton) {
        let status :SenStickStatus = cell!.iconButton!.selected ? .Stopping : .SensingAndLogging
        
        if let current_setting = self.service?.settingData {
            let setting = SensorSettingData<RotationRange>(status: status, samplingDuration: current_setting.samplingDuration, range: current_setting.range)
            service?.writeSetting(setting)
        }
        service?.readSetting()
    }
}
