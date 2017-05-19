//
//  UVCellView.swift
//  SenStickViewer
//
//  Created by AkihiroUehara on 2016/05/25.
//  Copyright © 2016年 AkihiroUehara. All rights reserved.
//

import UIKit
import SenStickSDK

class UVDataModel : SensorDataModel<UVSensorRange, UVSensorData>
{
    override init() {
        super.init()
        
        self.sensorName   = "uv"
        self.csvHeader    = "UV"
        self.csvEmptyData = ""
        
        // レンジの更新
        self.maxValue = 500
        self.minValue = 0
    }
    
    // MARK: - Private methods
    
    override func dataToArray(_ data: UVSensorData) -> [Double]
    {
        return [data.uv]
    }
}
