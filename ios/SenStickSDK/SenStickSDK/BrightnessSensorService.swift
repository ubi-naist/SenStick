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
    case BRIGHTNESS_RANGE_DEFAULT = 0x00
    
    public var description : String {
        switch self {
        case .BRIGHTNESS_RANGE_DEFAULT: return "BRIGHTNESS_RANGE_DEFAULT"
        }
    }
}

// 磁界のデータ構造体
// 16-bit 符号付き数字 フルスケール f ±4800 μT
struct BrightnessRawData
{
    var rawValue : UInt16
    
    init(rawValue:UInt16)
    {
        self.rawValue = rawValue
    }
    
    // 物理センサーの1luxあたりのLBSの値
    static func getLSBperLux(range: BrightnessRange) -> Double
    {
        switch range {
        case .BRIGHTNESS_RANGE_DEFAULT: return 1
        }
    }
    
    static func unpack(data: [Byte]) -> BrightnessRawData
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
    
    public static func unpack(range:BrightnessRange, value: [UInt8]) -> BrightnessData?
    {
        guard value.count >= 2 else {
            return nil
        }
        
        let rawData    = BrightnessRawData.unpack(value)
        let LSBperLux  = BrightnessRawData.getLSBperLux(range)
        
        return BrightnessData(brightness: (Double(rawData.rawValue) / LSBperLux));
    }
}

// センサー各種のベースタイプ, Tはセンサデータ独自のデータ型, Sはサンプリングの型、
public class BrightnessSensorService: SenStickSensorService<BrightnessData, BrightnessRange>, SenStickService
{
    required public init?(device:SenStickDevice)
    {
        super.init(device: device, sensorType: SenStickSensorType.BrightnessSensor)
    }
}
