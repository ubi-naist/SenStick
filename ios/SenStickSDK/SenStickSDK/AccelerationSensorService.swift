//
//  AccelerationSensorService.swift
//  SenStickSDK
//
//  Created by AkihiroUehara on 2016/04/23.
//  Copyright © 2016年 AkihiroUehara. All rights reserved.
//

import Foundation
import CoreMotion

// 加速度センサーの範囲設定値。列挙側の値は、BLEでの設定値に合わせている。
public enum AccelerationRange : UInt16, CustomStringConvertible
{
    case ACCELERATION_RANGE_2G   = 0x00 // +- 2g
    case ACCELERATION_RANGE_4G   = 0x01 // +- 4g
    case ACCELERATION_RANGE_8G   = 0x02 // +- 8g
    case ACCELERATION_RANGE_16G  = 0x03 // +- 16g
    
    public var description : String
    {
        switch self {
        case .ACCELERATION_RANGE_2G: return "2G"
        case .ACCELERATION_RANGE_4G: return "4G"
        case .ACCELERATION_RANGE_8G: return "8G"
        case .ACCELERATION_RANGE_16G:return "16G"
        }
    }
}

// センサーからの生データ。16ビット 符号付き数値。フルスケールは設定レンジ値による。2, 4, 8, 16G。
struct AccelerationRawData
{
    var xRawValue : Int16
    var yRawValue : Int16
    var zRawValue : Int16

    init(xRawValue:Int16, yRawValue:Int16, zRawValue:Int16)
    {
        self.xRawValue = xRawValue
        self.yRawValue = yRawValue
        self.zRawValue = zRawValue
    }
    
    // 物理センサーの1GあたりのLBSの値
    static func getLSBperG(range: AccelerationRange) -> Double
    {
        switch range {
        case .ACCELERATION_RANGE_2G: return 16384
        case .ACCELERATION_RANGE_4G: return 8192
        case .ACCELERATION_RANGE_8G: return 4096
        case .ACCELERATION_RANGE_16G:return 2048
        }
    }
    
    static func unpack(data: [Byte]) -> AccelerationRawData
    {
        let x = Int16.unpack(data[0..<2])
        let y = Int16.unpack(data[2..<4])
        let z = Int16.unpack(data[4..<6])

        return AccelerationRawData(xRawValue: x!, yRawValue: y!, zRawValue: z!)
    }
}

extension CMAcceleration : SensorDataPackableType
{
    public typealias RangeType = AccelerationRange

    public static func unpack(range:AccelerationRange, value: [UInt8]) -> CMAcceleration?
    {
        guard value.count >= 6 else {
            return nil
        }
        
        let rawData  = AccelerationRawData.unpack(value)
        let lsbPerG  = AccelerationRawData.getLSBperG(range)

        // FIXME 右手系/左手系などの座標変換など確認すること。
        
        return CMAcceleration(x: Double(rawData.xRawValue) / Double(lsbPerG), y: Double(rawData.yRawValue) / Double(lsbPerG), z: Double(rawData.zRawValue) / Double(lsbPerG))
    }
}

// センサー各種のベースタイプ, Tはセンサデータ独自のデータ型, Sはサンプリングの型、
public class AccelerationSensorService: SenStickSensorService<CMAcceleration, AccelerationRange>, SenStickService
{
    required public init?(device:SenStickDevice)
    {
        super.init(device: device, sensorType: SenStickSensorType.AccelerationSensor)
    }
}
