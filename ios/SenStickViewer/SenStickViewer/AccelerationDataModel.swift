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

class AccelerationDataModel :SensorDataModel
{
    weak var service: AccelerationSensorService? {
        didSet {
            self.service?.delegate = self
            didUpdateSetting(self)
        }
    }
    
    override init() {
        super.init()
        self.sensorName = "acceleration"
        self.csvHeader  = "Accs.X,\tAccs.Y,\tAccs.Z"
        self.csvEmptyData = ",\t,\t"
    }
    
    override func startToReadLog(_ logid: UInt8)
    {
        super.startToReadLog(logid)
        
        let logID = SensorLogID(logID: logid, skipCount: 0, position: 0)
        service?.writeLogID(logID)
    }
    
    // MARK: - SenStickSensorServiceDelegate
    override func didUpdateSetting(_ sender:AnyObject)
    {
        cell?.iconButton?.isEnabled       = (self.service != nil)
        self.cell?.iconButton?.isSelected = (service?.settingData?.status != .stopping)
        
        // レンジの更新
        if let setting = service?.settingData {
            self.duration = setting.samplingDuration
            updateRange(setting.range)
        }
    }
    
    func updateRange(_ range:AccelerationRange)
    {
        switch(range) {
        case .acceleration_RANGE_2G:
            self.maxValue = 2.5
            self.minValue = -2.5
            
        case .acceleration_RANGE_4G:
            self.maxValue = 4.5
            self.minValue = -4.5
            
        case .acceleration_RANGE_8G:
            self.maxValue = 8.0
            self.minValue = -8.0
            
        case .acceleration_RANGE_16G:
            self.maxValue = 16.0
            self.minValue = -16.0
        }
    }
    
    override func didUpdateRealTimeData(_ sender: AnyObject)
    {
        if let data = service?.realtimeData {
            drawRealTimeData([data.x, data.y, data.z])
        }
    }
    
    override func didUpdateMetaData(_ sender: AnyObject)
    {
        guard let metaData = service?.logMetaData else {
            return
        }
        
        //        debugPrint("\(#function), availableCount: \(service!.logMetaData!.availableSampleCount)")
        self.duration = metaData.samplingDuration
        updateRange(metaData.range)
        
        let count = metaData.availableSampleCount
        cell?.graphView?.sampleCount = Int(count)
        cell?.iconButton?.isEnabled    = (count != 0)
        cell?.iconButton?.isSelected   = (count != 0)
        cell?.progressBar?.isHidden    = (count == 0)
    }
    
    override func didUpdateLogData(_ sender: AnyObject)
    {
        if let array = service?.readLogData() {
            for data in array {
                addReadLog([data.x, data.y, data.z])
            }
        }
    }
    
    // MARK: - Event handler
    override func iconButtonToutchUpInside(_ sender: UIButton) {
        let status :SenStickStatus = cell!.iconButton!.isSelected ? .stopping : .sensingAndLogging
        
        if let current_setting = self.service?.settingData {
            let setting = SensorSettingData<AccelerationRange>(status: status, samplingDuration: current_setting.samplingDuration, range: current_setting.range)
            service?.writeSetting(setting)
        }
        service?.readSetting()
    }
}
