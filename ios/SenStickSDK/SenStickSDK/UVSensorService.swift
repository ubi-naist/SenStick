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
    case UV_RANGE_DEFAULT = 0x00
    
    public var description : String {
        switch self {
        case .UV_RANGE_DEFAULT: return "UV_RANGE_DEFAULT"
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
    static func getLSBperuWcm2(range: UVSensorRange) -> Double
    {
        switch range {
        case .UV_RANGE_DEFAULT: return (1.0 / 5.0)
        }
    }
    
    static func unpack(data: [Byte]) -> UVRawData
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
    
    public static func unpack(range:UVSensorRange, value: [UInt8]) -> UVSensorData?
    {
        guard value.count >= 2 else {
            return nil
        }
        
        let rawData    = UVRawData.unpack(value)
        let LSBperuWcm  = UVRawData.getLSBperuWcm2(range)
        
        return UVSensorData(uv: (Double(rawData.rawValue) / LSBperuWcm));
    }
}

// センサー各種のベースタイプ, Tはセンサデータ独自のデータ型, Sはサンプリングの型、
public class UVSensorService: SenStickSensorService<UVSensorData, UVSensorRange>, SenStickService
{
    required public init?(device:SenStickDevice)
    {
        super.init(device: device, sensorType: SenStickSensorType.BrightnessSensor)
    }
}
