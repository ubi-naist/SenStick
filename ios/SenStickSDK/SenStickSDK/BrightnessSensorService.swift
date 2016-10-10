//
//  BrightnessSensorService.swift
//  SenStickSDK
//
//  Created by AkihiroUehara on 2016/05/24.
//  Copyright © 2016年 AkihiroUehara. All rights reserved.
//

import Foundation

public enum BrightnessRange : UInt16, CustomStringConvertible
{
    case brightnessRangeDefault = 0x00
    
    public var description : String {
        switch self {
        case .brightnessRangeDefault: return "brightnessRangeDefault"
        }
    }
}

struct BrightnessRawData
{
    var rawValue : UInt16
    
    init(rawValue:UInt16)
    {
        self.rawValue = rawValue
    }
    
    // 物理センサーの1luxあたりのLBSの値
    static func getLSBperLux(_ range: BrightnessRange) -> Double
    {
        switch range {
        case .brightnessRangeDefault: return 1
        }
    }
    
    static func unpack(_ data: [Byte]) -> BrightnessRawData
    {
        let value = UInt16.unpack(data[0..<2])
        
        return BrightnessRawData(rawValue: value!)
    }
}

public struct BrightnessData : SensorDataPackableType
{
    public var brightness: Double
    public init(brightness: Double) {
        self.brightness = brightness
    }
    
    public typealias RangeType = BrightnessRange
    
    public static func unpack(_ range:BrightnessRange, value: [UInt8]) -> BrightnessData?
    {
        guard value.count >= 2 else {
            return nil
        }
        
        let rawData    = BrightnessRawData.unpack(value)
        let LSBperLux  = BrightnessRawData.getLSBperLux(range)
//debugPrint("Brightness, raw:\(rawData), lsbPerLux:\(LSBperLux), value:\((Double(rawData.rawValue) / LSBperLux))")
        return BrightnessData(brightness: (Double(rawData.rawValue) / LSBperLux));
    }
}

// センサー各種のベースタイプ, Tはセンサデータ独自のデータ型, Sはサンプリングの型、
open class BrightnessSensorService: SenStickSensorService<BrightnessData, BrightnessRange>, SenStickService
{
    required public init?(device:SenStickDevice)
    {
        super.init(device: device, sensorType: SenStickSensorType.brightnessSensor)
    }
}
