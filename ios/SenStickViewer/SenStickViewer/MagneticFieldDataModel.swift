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

class MagneticFieldDataModel : SensorDataModel<MagneticFieldRange, CMMagneticField>
{
    override init() {
        super.init()
        
        self.sensorName   = "magnetic"
        self.csvHeader    = "Magnetic.X,Magnetic.Y,Magnetic.Z"
        self.csvEmptyData = ",,"
        
        // レンジの更新
        self.maxValue = 100
        self.minValue = -100
    }
    
    // MARK: - Private methods
    
    override func dataToArray(_ data: CMMagneticField) -> [Double]
    {
        return [data.x, data.y, data.z]
    }
    
}
