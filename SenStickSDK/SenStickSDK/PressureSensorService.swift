//
//  PressureSensorService.swift
//  SenStickSDK
//
//  Created by AkihiroUehara on 2016/05/24.
//  Copyright © 2016年 AkihiroUehara. All rights reserved.
//

import Foundation

public enum PressureRange : UInt16, CustomStringConvertible
{
    case pressureRangeDefault = 0x00
    
    public var description : String {
        switch self {
        case .pressureRangeDefault: return "pressureRangeDefault"
        }
    }
}

public struct PressureData : SensorDataPackableType
{
    public var pressure: Double //hPa
    
    public init(pressure: Double) {
        self.pressure = pressure
    }
    
    public typealias RangeType = PressureRange
    
    public static func unpack(_ range:PressureRange, value: [UInt8]) -> PressureData?
    {
        guard value.count >= 4 else {
            return nil
        }
        
        let rawData    = UInt32.unpack(value[0..<4])
//debugPrint("Pressure: raw,\(rawData) value, \((Double(rawData!) / Double(4096)))")
        return PressureData(pressure: (Double(rawData!) / Double(4096)));
    }
}

// センサー各種のベースタイプ, Tはセンサデータ独自のデータ型, Sはサンプリングの型、
open class PressureSensorService: SenStickSensorService<PressureData, PressureRange>
{
    required public init?(device:SenStickDevice)
    {
        super.init(device: device, sensorType: SenStickSensorType.airPressureSensor)
    }
}
