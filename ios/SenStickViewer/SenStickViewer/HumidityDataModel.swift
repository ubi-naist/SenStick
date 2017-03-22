//
//  HumidityCellView.swift
//  SenStickViewer
//
//  Created by AkihiroUehara on 2016/05/25.
//  Copyright © 2016年 AkihiroUehara. All rights reserved.
//

import UIKit
import SenStickSDK

class HumidityDataModel : SensorDataModel<HumiditySensorData, HumiditySensorRange>
{
    override init() {
        super.init()
        
        self.sensorName   = "humidity"
        self.csvHeader    = "Humidity,temp"
        self.csvEmptyData = ","
        
        // レンジの更新
        self.maxValue = 100
        self.minValue = 0
    }
    
    // MARK: - Private methods
    
    override func dataToArray(_ data: HumiditySensorData) -> [Double]
    {
        return [data.humidity, data.temperature]
    }
    
    
}
