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
    case AccelerationSensor              = 0
    case GyroSensor                      = 1
    case MagneticFieldSensor             = 2
    case BrightnessSensor                = 3
    case UltraVioletSensor               = 4
    case HumidityAndTemperatureSensor    = 5
    case AirPressureSensor               = 6
    
    static let allValues = [AccelerationSensor, GyroSensor, MagneticFieldSensor, BrightnessSensor, UltraVioletSensor, HumidityAndTemperatureSensor, AirPressureSensor]
    
    public var description : String
    {
        switch self {
        case .AccelerationSensor:              return "AccelerationSensor"
        case .GyroSensor:                      return "GyroSensor"
        case .MagneticFieldSensor:             return "MagneticFieldSensor"
        case .BrightnessSensor:                return "BrightnessSensor"
        case .UltraVioletSensor:               return "UltraVioletSensor"
        case .HumidityAndTemperatureSensor:    return "HumidityAndTemperatureSensor"
        case .AirPressureSensor:               return "AirPressureSensor"
        }
    }
}


