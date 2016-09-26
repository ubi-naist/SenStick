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
    
    override init() {
        super.init()
        self.sensorName = "gyro"
        self.csvHeader  = "Gyro.X,\tGyro.Y,\tGyro.Z"
        self.csvEmptyData = ",\t,\t"
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
        cell?.iconButton?.isSelected = (service?.settingData?.status != .stopping)
        
        // レンジの更新
        //        let k = M_PI / Double(180), 1/60
        if let setting = service?.settingData {
            self.duration = setting.samplingDuration
            updateRange(setting.range)
        }
    }
    
    func updateRange(_ range: RotationRange)
    {
        switch(range) {
        case .rotation_RANGE_250DPS:
            self.maxValue = 5
            self.minValue = -5
            
        case .rotation_RANGE_500DPS:
            self.maxValue = 10
            self.minValue = -10
            
        case .rotation_RANGE_1000DPS:
            self.maxValue = 20
            self.minValue = -20
            
        case .rotation_RANGE_2000DPS:
            self.maxValue = 40
            self.minValue = -40
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
        //        debugPrint("\(#function) \(sender)")
        if let array = service?.readLogData() {
            //        debugPrint("     \(array.count)")
            for data in array {
                addReadLog([data.x, data.y, data.z])
            }
        }
    }
    
    // MARK: - Event handler
    override func  iconButtonToutchUpInside(_ sender: UIButton) {
        let status :SenStickStatus = cell!.iconButton!.isSelected ? .stopping : .sensingAndLogging
        
        if let current_setting = self.service?.settingData {
            let setting = SensorSettingData<RotationRange>(status: status, samplingDuration: current_setting.samplingDuration, range: current_setting.range)
            service?.writeSetting(setting)
        }
        service?.readSetting()
    }
}
