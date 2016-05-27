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

class GyroCellView : SensorDataCellView
{
    weak var service: GyroSensorService? {
        didSet {
            self.service?.delegate = self
            
            if self.service == nil {
                self.iconButton?.enabled = false
            } else {
                self.iconButton?.enabled = true
                didUpdateSetting(self)                
            }
        }
    }
    
    override func startToReadLog(logid: UInt8)
    {
        super.startToReadLog(logid)
        
        let logID = SensorLogID(logID: logid, skipCount: 0, position: 0)
        service?.writeLogID(logID)
    }
    
    // MARK: - SenStickSensorServiceDelegate
    override  func didUpdateSetting(sender:AnyObject)
    {
        self.iconButton?.selected = (service?.settingData?.status != .Stopping)
        
        // レンジの更新
        //        let k = M_PI / Double(180), 1/60
        if let setting = service?.settingData {
            self.duration = setting.samplingDuration
            switch(setting.range) {                
            case .ROTATION_RANGE_250DPS:
                self.maxValue = 5
                self.minValue = -5
                self.graphView?.maxValue = 5
                self.graphView?.minValue = -5
                
            case .ROTATION_RANGE_500DPS:
                self.maxValue = 10
                self.minValue = -10
                self.graphView?.maxValue = 10
                self.graphView?.minValue = -10
                
            case .ROTATION_RANGE_1000DPS:
                self.maxValue = 20
                self.minValue = -20
                self.graphView?.maxValue = 20
                self.graphView?.minValue = -20
                
            case .ROTATION_RANGE_2000DPS:
                self.maxValue = 40
                self.minValue = -40
                self.graphView?.maxValue = 40
                self.graphView?.minValue = -40
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
    }
    
    override func didUpdateLogData(sender: AnyObject)
    {
        if let array = service?.readLogData() {
            let sampleCount = service!.logMetaData!.availableSampleCount
            let progress = Double(super.logData![0].count + array.count) / Double(sampleCount)
            
            for data in array {
                addReadLog([data.x, data.y, data.z], progress: progress)
            }
        }
    }
    override func didFinishedLogData(sender: AnyObject)
    {
        stopReadingLog("gyro", duration: service?.logMetaData?.samplingDuration)
    }
    
    
    // MARK: - Event handler
    @IBAction func  iconButtonToutchUpInside(sender: UIButton) {
        let status :SenStickStatus = iconButton!.selected ? .Stopping : .SensingAndLogging
        
        if let current_setting = self.service?.settingData {
            let setting = SensorSettingData<RotationRange>(status: status, samplingDuration: current_setting.samplingDuration, range: current_setting.range)
            service?.writeSetting(setting)
        }
        service?.readSetting()
    }
}