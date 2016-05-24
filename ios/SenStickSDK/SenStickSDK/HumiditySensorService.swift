//
//  Humidity.swift
//  SenStickSDK
//
//  Created by AkihiroUehara on 2016/05/24.
//  Copyright © 2016年 AkihiroUehara. All rights reserved.
//

import Foundation

public enum HumiditySensorRange : UInt16, CustomStringConvertible
{
    case HUMIDITY_RANGE_DEFAULT = 0x00
    
    public var description : String {
        switch self {
        case .HUMIDITY_RANGE_DEFAULT:    return "HUMIDITY_RANGE_DEFAULT"
        }
    }
}

struct HumiditySensorRawData
{
    internal var humidityRawValue : Int16
    internal var temperatureRawValue     : Int16

    
    init(humidityRawValue:Int16, temperatureRawValue:Int16)
    {
        self.humidityRawValue    = humidityRawValue
        self.temperatureRawValue = temperatureRawValue
    }
    
    static func unpack(data: [Byte]) -> HumiditySensorRawData
    {
        let humidityRaw    = Int16.unpack(data[0..<2])
        let temperatureRaw = Int16.unpack(data[2..<4])

        return HumiditySensorRawData(humidityRawValue: humidityRaw!, temperatureRawValue: temperatureRaw!)
    }
}

public struct HumiditySensorData : SensorDataPackableType
{
    public var humidity: Double
    public var temperature: Double
    
    public init(humidity: Double, temperature: Double) {
        self.humidity    = humidity
        self.temperature = temperature
    }
    
    public typealias RangeType = HumiditySensorRange
    
    public static func unpack(range:HumiditySensorRange, value: [UInt8]) -> HumiditySensorData?
    {
        guard value.count >= 4 else {
            return nil
        }
        
        let rawData  = HumiditySensorRawData.unpack(value)
        
        let RH = -6.0   + 125.0  * Double(rawData.humidityRawValue)     / pow(2,16)
        let T  = -46.85 + 175.72 * Double(rawData.temperatureRawValue) / pow(2, 16)
        
        return HumiditySensorData(humidity: RH, temperature: T)
    }
}

// センサー各種のベースタイプ, Tはセンサデータ独自のデータ型, Sはサンプリングの型、
public class HumiditySensorService: SenStickSensorService<HumiditySensorData, HumiditySensorRange>, SenStickService
{
    required public init?(device:SenStickDevice)
    {
        super.init(device: device, sensorType: SenStickSensorType.HumidityAndTemperatureSensor)
    }
}
