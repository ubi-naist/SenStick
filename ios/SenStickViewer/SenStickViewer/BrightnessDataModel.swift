//
//  BrightnessCellView.swift
//  SenStickViewer
//
//  Created by AkihiroUehara on 2016/05/25.
//  Copyright © 2016年 AkihiroUehara. All rights reserved.
//

import UIKit
import SenStickSDK

class BrightnessDataModel : SensorDataModel
{
    weak var service: BrightnessSensorService? {
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
        self.cell?.iconButton?.selected = (service?.settingData?.status != .Stopping)
        
        // レンジの更新
        self.maxValue = 2000
        self.minValue = 0
        
        if let setting = service?.settingData {
            self.duration = setting.samplingDuration
        }
    }
    
    override func didUpdateRealTimeData(sender: AnyObject)
    {
        if let data = service?.realtimeData {
            drawRealTimeData([data.brightness])
        }
    }
    
    override func didUpdateMetaData(sender: AnyObject)
    {
        if let count = service?.logMetaData?.availableSampleCount {
            cell?.graphView?.sampleCount = Int(count)
            cell?.iconButton?.enabled  = (count != 0)
            cell?.iconButton?.selected = (count != 0)
            if count == 0 {
                cell?.progressBar?.hidden    = true                
            }
        }
    }
    
    override func didUpdateLogData(sender: AnyObject)
    {
        if let array = service?.readLogData() {
            for data in array {
                addReadLog([data.brightness])
            }
        }
    }

    override func didFinishedLogData(sender: AnyObject)
    {        
        stopReadingLog("brightness", duration: service?.logMetaData?.samplingDuration)
    }

    // MARK: - Event handler
    override func iconButtonToutchUpInside(sender: UIButton) {
        let status :SenStickStatus = cell!.iconButton!.selected ? .Stopping : .SensingAndLogging
        
        if let current_setting = self.service?.settingData {
        let setting = SensorSettingData<BrightnessRange>(status: status, samplingDuration: current_setting.samplingDuration, range: current_setting.range)
            
            service?.writeSetting(setting)
        }
        service?.readSetting()
    }
}
