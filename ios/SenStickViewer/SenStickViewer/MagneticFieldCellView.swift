//
//  MagneticFieldCell.swift
//  SenStickViewer
//
//  Created by AkihiroUehara on 2016/05/25.
//  Copyright © 2016年 AkihiroUehara. All rights reserved.
//

import UIKit
import SenStickSDK
import CoreMotion

class MagneticFieldCellView : SensorDataCellView
{
    weak var service: MagneticFieldSensorService? {
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
        service?.readLogData()         
        super.startToReadLog(logid)
        
        let logID = SensorLogID(logID: logid, skipCount: 0, position: 0)
        service?.writeLogID(logID)
    }
    
    // MARK: - SenStickSensorServiceDelegate
    override  func didUpdateSetting(sender:AnyObject)
    {
        self.iconButton?.selected = (service?.settingData?.status != .Stopping)
        
        // レンジの更新
        self.maxValue = 100
        self.minValue = -100
        self.graphView?.maxValue = 100
        self.graphView?.minValue = -100
        
        if let setting = service?.settingData {
            self.duration = setting.samplingDuration
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
        didUpdateLogData(self)         
        stopReadingLog("magnetic", duration: service?.logMetaData?.samplingDuration)
    }
    
    // MARK: - Event handler
    @IBAction func  iconButtonToutchUpInside(sender: UIButton) {
        let status :SenStickStatus = iconButton!.selected ? .Stopping : .SensingAndLogging
        
        if let current_setting = self.service?.settingData {
            let setting = SensorSettingData<MagneticFieldRange>(status: status, samplingDuration: current_setting.samplingDuration, range: current_setting.range)

            service?.writeSetting(setting)
        }
        service?.readSetting()
    }
}