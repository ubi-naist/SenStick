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

class GyroDataModel : SensorDataModel<CMRotationRate, RotationRange>
{
    override init() {
        super.init()
        self.sensorName   = "gyro"
        self.csvHeader    = "Gyro.X,Gyro.Y,Gyro.Z"
        self.csvEmptyData = ",,"
    }
        
    // MARK: - Private methods
    
    override func updateRange(_ range: RotationRange)
    {
        switch(range) {
        case .rotationRange250DPS:
            self.maxValue = 5
            self.minValue = -5
            
        case .rotationRange500DPS:
            self.maxValue = 10
            self.minValue = -10
            
        case .rotationRange1000DPS:
            self.maxValue = 20
            self.minValue = -20
            
        case .rotationRange2000DPS:
            self.maxValue = 40
            self.minValue = -40
        }
    }

    override func dataToArray(_ data: CMRotationRate) -> [Double]
    {
        return [data.x, data.y, data.z]
    }
}
