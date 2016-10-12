//
//  BrightnessCellView.swift
//  SenStickViewer
//
//  Created by AkihiroUehara on 2016/05/25.
//  Copyright © 2016年 AkihiroUehara. All rights reserved.
//

import UIKit
import SenStickSDK

class BrightnessDataModel : SensorDataModel<BrightnessData, BrightnessRange>
{
    override init() {
        super.init()
        
        self.sensorName   = "brightness"
        self.csvHeader    = "Brightness"
        self.csvEmptyData = ""
        
        // レンジの更新
        self.maxValue = 2000
        self.minValue = 0
    }
    
    // MARK: - Private methods
    
    override func dataToArray(_ data: BrightnessData) -> [Double]
    {
        return [data.brightness]
    }
    
}
