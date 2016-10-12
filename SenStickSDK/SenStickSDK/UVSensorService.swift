//
//  UVSensorService.swift
//  SenStickSDK
//
//  Created by AkihiroUehara on 2016/05/24.
//  Copyright © 2016年 AkihiroUehara. All rights reserved.
//

import Foundation

public enum UVSensorRange : UInt16, CustomStringConvertible
{
    case uvRangeDefault = 0x00
    
    public var description : String {
        switch self {
        case .uvRangeDefault: return "uvRangeDefault"
        }
    }
}

struct UVRawData
{
    var rawValue : UInt16
    
    init(rawValue:UInt16)
    {
        self.rawValue = rawValue
    }
    
    // 物理センサーの1uW/cm^2あたりのLBSの値
    static func getLSBperuWcm2(_ range: UVSensorRange) -> Double
    {
        switch range {
        case .uvRangeDefault: return (1.0 / 5.0)
        }
    }
    
    static func unpack(_ data: [Byte]) -> UVRawData
    {
        let value = UInt16.unpack(data[0..<2])
        
        return UVRawData(rawValue: value!)
    }
}

public struct UVSensorData : SensorDataPackableType
{
    public var uv: Double
    public init(uv: Double) {
        self.uv = uv
    }
    
    public typealias RangeType = UVSensorRange
    
    public static func unpack(_ range:UVSensorRange, value: [UInt8]) -> UVSensorData?
    {
        guard value.count >= 2 else {
            return nil
        }
        
        let rawData    = UVRawData.unpack(value)
        let LSBperuWcm  = UVRawData.getLSBperuWcm2(range)
//debugPrint("\(rawData), LSBperuWcm:\(LSBperuWcm)")
        return UVSensorData(uv: (Double(rawData.rawValue) / LSBperuWcm));
    }
}

// センサー各種のベースタイプ, Tはセンサデータ独自のデータ型, Sはサンプリングの型、
open class UVSensorService: SenStickSensorService<UVSensorData, UVSensorRange>
{
    required public init?(device:SenStickDevice)
    {
        super.init(device: device, sensorType: SenStickSensorType.ultraVioletSensor)
    }
}
