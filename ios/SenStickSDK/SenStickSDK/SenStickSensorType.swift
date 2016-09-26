//
//  SenStickSensorType.swift
//  SenStickSDK
//
//  Created by AkihiroUehara on 2016/04/20.
//  Copyright © 2016年 AkihiroUehara. All rights reserved.
//

import Foundation


// センサータイプ
public enum SenStickSensorType: UInt8, CustomStringConvertible
{
    case accelerationSensor              = 0
    case gyroSensor                      = 1
    case magneticFieldSensor             = 2
    case brightnessSensor                = 3
    case ultraVioletSensor               = 4
    case humidityAndTemperatureSensor    = 5
    case airPressureSensor               = 6
    
    static let allValues = [accelerationSensor, gyroSensor, magneticFieldSensor, brightnessSensor, ultraVioletSensor, humidityAndTemperatureSensor, airPressureSensor]
    
    public var description : String
    {
        switch self {
        case .accelerationSensor:              return "AccelerationSensor"
        case .gyroSensor:                      return "GyroSensor"
        case .magneticFieldSensor:             return "MagneticFieldSensor"
        case .brightnessSensor:                return "BrightnessSensor"
        case .ultraVioletSensor:               return "UltraVioletSensor"
        case .humidityAndTemperatureSensor:    return "HumidityAndTemperatureSensor"
        case .airPressureSensor:               return "AirPressureSensor"
        }
    }
}


