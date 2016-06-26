//
//  PressureCellView.swift
//  SenStickViewer
//
//  Created by AkihiroUehara on 2016/05/25.
//  Copyright © 2016年 AkihiroUehara. All rights reserved.
//

import UIKit
import SenStickSDK

class PressureDataModel : SensorDataModel
{
    weak var service: PressureSensorService? {
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
        self.maxValue = 1300
        self.minValue = 800
        
        if let setting = service?.settingData {
            self.duration = setting.samplingDuration
        }        
    }
    
    override func didUpdateRealTimeData(sender: AnyObject)
    {
        if let data = service?.realtimeData {
            drawRealTimeData([data.pressure])
        }
    }
    
    override func didUpdateMetaData(sender: AnyObject)
    {
        if let count = service?.logMetaData?.availableSampleCount {
            cell?.graphView?.sampleCount = Int(count)
            if count == 0 {
                cell?.progressBar?.hidden    = true
            }
        }
    }
    
    override func didUpdateLogData(sender: AnyObject)
    {      
        if let array = service?.readLogData() {
            for data in array {
                addReadLog([data.pressure])
            }
        }
    }

    override func didFinishedLogData(sender: AnyObject)
    {      
        stopReadingLog("pressure", duration: service?.logMetaData?.samplingDuration)
    }
    
    // MARK: - Event handler
    override func iconButtonToutchUpInside(sender: UIButton) {
        let status :SenStickStatus = cell!.iconButton!.selected ? .Stopping : .SensingAndLogging
        
        if let current_setting = self.service?.settingData {
            let setting = SensorSettingData<PressureRange>(status: status, samplingDuration: current_setting.samplingDuration, range: current_setting.range)
            service?.writeSetting(setting)
        }
        service?.readSetting()
    }
}
