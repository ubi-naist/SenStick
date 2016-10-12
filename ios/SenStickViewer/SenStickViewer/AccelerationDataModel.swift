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

class AccelerationDataModel :SensorDataModel<CMAcceleration, AccelerationRange>
{
    override init() {
        super.init()
        
        self.sensorName   = "acceleration"
        self.csvHeader    = "Accs.X,\tAccs.Y,\tAccs.Z"
        self.csvEmptyData = ",\t,\t"
    }
        
    // MARK: - Private methods
    
    override func updateRange(_ range:AccelerationRange)
    {
        switch(range) {
        case .accelerationRange2G:
            self.maxValue = 2.5
            self.minValue = -2.5
            
        case .accelerationRange4G:
            self.maxValue = 4.5
            self.minValue = -4.5
            
        case .accelerationRange8G:
            self.maxValue = 8.0
            self.minValue = -8.0
            
        case .accelerationRange16G:
            self.maxValue = 16.0
            self.minValue = -16.0
        }
    }
    
    override func dataToArray(_ data: CMAcceleration) -> [Double]
    {
        return [data.x, data.y, data.z]
    }
}
