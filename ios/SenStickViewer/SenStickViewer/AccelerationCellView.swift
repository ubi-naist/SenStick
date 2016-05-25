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

public class AccelerationCellView : SensorDataCellView
{
    weak var service: AccelerationSensorService? {
        didSet {
            self.service?.delegate = self
            
            if self.service == nil {
                self.iconButton?.enabled = false
            } else {
                self.iconButton?.enabled = true
            }
        }
    }
    
    override public func didUpdateSetting(sender:AnyObject)
    {
        self.iconButton?.selected = (service?.settingData?.status != .Stopping)
        
        // レンジの更新
        if let setting = service?.settingData {
            switch(setting.range) {
            case .ACCELERATION_RANGE_2G:
                self.maxValue = 2.0
                self.minValue = -2.0
                
            case .ACCELERATION_RANGE_4G:
                self.maxValue = 4.0
                self.minValue = -4.0

            case .ACCELERATION_RANGE_8G:
                self.maxValue = 8.0
                self.minValue = -8.0

            case .ACCELERATION_RANGE_16G:
                self.maxValue = 16.0
                self.minValue = -16.0
            }
        }
    }
    
    @IBAction func  iconButtonToutchUpInside(sender: UIButton) {        
        let status :SenStickStatus = iconButton!.selected ? .Stopping : .SensingAndLogging
        
        let current_setting = self.service!.settingData!
        let setting = SensorSettingData<AccelerationRange>(status: status, samplingDuration: current_setting.samplingDuration, range: current_setting.range)
        service?.writeSetting(setting)
    }
}