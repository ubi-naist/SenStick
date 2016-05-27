//
//  AccelerationController.swift
//  SenStickViewer
//
//  Created by AkihiroUehara on 2016/05/24.
//  Copyright © 2016年 AkihiroUehara. All rights reserved.
//

import UIKit
import SenStickSDK
import CoreMotion

class AccelerationCellView : SensorDataCellView
{
    weak var service: AccelerationSensorService? {
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
        if let setting = service?.settingData {
            self.duration = setting.samplingDuration
            switch(setting.range) {
            case .ACCELERATION_RANGE_2G:
                self.maxValue = 2.5
                self.minValue = -2.5
                self.graphView?.maxValue = 2.5
                self.graphView?.minValue = -2.5

            case .ACCELERATION_RANGE_4G:
                self.maxValue = 4.5
                self.minValue = -4.5
                self.graphView?.maxValue = 4.5
                self.graphView?.minValue = -4.5

            case .ACCELERATION_RANGE_8G:
                self.maxValue = 8.0
                self.minValue = -8.0
                self.graphView?.maxValue = 8.0
                self.graphView?.minValue = -8.0

            case .ACCELERATION_RANGE_16G:
                self.maxValue = 16.0
                self.minValue = -16.0
                self.graphView?.maxValue = 16.0
                self.graphView?.minValue = -16.0
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
        debugPrint("\(#function), availableCount: \(service!.logMetaData!.availableSampleCount)")
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
        debugPrint("\(#function) availableCount: \(service!.logMetaData!.availableSampleCount) read count:\(super.logData![0].count)")
        stopReadingLog("acceleration", duration: service?.logMetaData?.samplingDuration)
    }
    
    // MARK: - Event handler
    @IBAction func  iconButtonToutchUpInside(sender: UIButton) {        
        let status :SenStickStatus = iconButton!.selected ? .Stopping : .SensingAndLogging
        
        if let current_setting = self.service?.settingData {
            let setting = SensorSettingData<AccelerationRange>(status: status, samplingDuration: current_setting.samplingDuration, range: current_setting.range)
            service?.writeSetting(setting)
        }
        service?.readSetting()
    }
}