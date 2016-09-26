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
    
    override init() {
        super.init()
        self.sensorName = "brightness"
        self.csvHeader  = "Brightness"
        self.csvEmptyData = ""
    }
    
    override func startToReadLog(_ logid: UInt8)
    {
        _ = service?.readLogData()
        super.startToReadLog(logid)
        
        let logID = SensorLogID(logID: logid, skipCount: 0, position: 0)
        service?.writeLogID(logID)
    }
    
    // MARK: - SenStickSensorServiceDelegate
    override func didUpdateSetting(_ sender:AnyObject)
    {
        cell?.iconButton?.isEnabled = (self.service != nil)
        self.cell?.iconButton?.isSelected = (service?.settingData?.status != .stopping)
        
        // レンジの更新
        self.maxValue = 2000
        self.minValue = 0
        
        if let setting = service?.settingData {
            self.duration = setting.samplingDuration
        }
    }
    
    override func didUpdateRealTimeData(_ sender: AnyObject)
    {
        if let data = service?.realtimeData {
            drawRealTimeData([data.brightness])
        }
    }
    
    override func didUpdateMetaData(_ sender: AnyObject)
    {
        guard let metaData = service?.logMetaData  else {
            return
        }
        
        self.duration = metaData.samplingDuration
        // レンジの更新
        self.maxValue = 2000
        self.minValue = 0
        
        let count = metaData.availableSampleCount
        cell?.graphView?.sampleCount = Int(count)
        cell?.iconButton?.isEnabled    = (count != 0)
        cell?.iconButton?.isSelected   = (count != 0)
        cell?.progressBar?.isHidden    = (count == 0 )                
    }
    
    override func didUpdateLogData(_ sender: AnyObject)
    {
        if let array = service?.readLogData() {
            for data in array {
                addReadLog([data.brightness])
            }
        }
    }
    
    // MARK: - Event handler
    override func iconButtonToutchUpInside(_ sender: UIButton) {
        let status :SenStickStatus = cell!.iconButton!.isSelected ? .stopping : .sensingAndLogging
        
        if let current_setting = self.service?.settingData {
            let setting = SensorSettingData<BrightnessRange>(status: status, samplingDuration: current_setting.samplingDuration, range: current_setting.range)
            
            service?.writeSetting(setting)
        }
        service?.readSetting()
    }
}
