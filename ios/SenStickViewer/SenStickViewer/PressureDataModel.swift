//
//  PressureCellView.swift
//  SenStickViewer
//
//  Created by AkihiroUehara on 2016/05/25.
//  Copyright © 2016年 AkihiroUehara. All rights reserved.
//

import UIKit
import SenStickSDK

class PressureDataModel : SensorDataModel<PressureData, PressureRange>
{    
    override init() {
        super.init()
        
        self.sensorName   = "pressure"
        self.csvHeader    = "AirPressure"
        self.csvEmptyData = ""
        
        // レンジの更新
        self.maxValue = 1300
        self.minValue = 800        
    }
    
    // MARK: - Private methods
    
    override func dataToArray(_ data: PressureData) -> [Double]
    {
        return [data.pressure]
    }
    
}
