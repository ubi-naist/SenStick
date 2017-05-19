//
//  GyroSensorService.swift
//  SenStickSDK
//
//  Created by AkihiroUehara on 2016/05/24.
//  Copyright © 2016年 AkihiroUehara. All rights reserved.
//

import Foundation
import CoreMotion

// ジャイロセンサーの範囲設定値。列挙側の値は、BLEでの設定値に合わせている。
public enum RotationRange : UInt16, CustomStringConvertible
{
    case rotationRange250DPS   = 0x00
    case rotationRange500DPS   = 0x01
    case rotationRange1000DPS  = 0x02
    case rotationRange2000DPS  = 0x03
    
    public var description : String {
        switch self {
        case .rotationRange250DPS:    return "250DPS"
        case .rotationRange500DPS:    return "500DPS"
        case .rotationRange1000DPS:   return "1000DPS"
        case .rotationRange2000DPS:   return "2000DPS"
        }
    }
}

// ジャイロのデータ構造体
// 16ビット 符号付き数値。フルスケールは設定レンジ値による。250, 500, 1000, 2000 DPS。
struct RotationRawData
{
    typealias T = Int16
    var xRawValue : Int16
    var yRawValue : Int16
    var zRawValue : Int16

    init(xRawValue:Int16, yRawValue:Int16, zRawValue:Int16)
    {
        self.xRawValue = xRawValue
        self.yRawValue = yRawValue
        self.zRawValue = zRawValue
    }
    
    // 物理センサーの1deg/sあたりのLBSの値
    static func getLSBperDegS(_ range: RotationRange) -> Double
    {
        switch range {
        case .rotationRange250DPS:    return (32768.0 / 250.0)
        case .rotationRange500DPS:    return (32768.0 / 500.0)
        case .rotationRange1000DPS:   return (32768.0 / 1000.0)
        case .rotationRange2000DPS:   return (32768.0 / 2000.0)
        }
    }
    
    static func unpack(_ data: [Byte]) -> RotationRawData
    {
        let x = Int16.unpack(data[0..<2])
        let y = Int16.unpack(data[2..<4])
        let z = Int16.unpack(data[4..<6])
        
        return RotationRawData(xRawValue: x!, yRawValue: y!, zRawValue: z!)
    }
}

extension CMRotationRate : SensorDataPackableType
{
    public typealias RangeType = RotationRange
    
    public static func unpack(_ range:RotationRange, value: [UInt8]) -> CMRotationRate?
    {
        guard value.count >= 6 else {
            return nil
        }
        
        let rawData   = RotationRawData.unpack(value)
        let lsbPerDeg = RotationRawData.getLSBperDegS(range)
        
        // FIXME 右手系/左手系などの座標変換など確認すること。
        // deg/s なので rad/sに変換
        let k = Double.pi / Double(180)
//debugPrint("x:\(rawData.xRawValue), y:\(rawData.yRawValue), z: \(rawData.zRawValue), lsbPerDeg:\(lsbPerDeg)")
        return CMRotationRate(x: k * Double(rawData.xRawValue) / Double(lsbPerDeg), y: k * Double(rawData.yRawValue) / Double(lsbPerDeg), z: k * Double(rawData.zRawValue) / Double(lsbPerDeg))
    }
}

// センサー各種のベースタイプ, Tはセンサデータ独自のデータ型, Sはサンプリングの型、
open class GyroSensorService: SenStickSensorService<CMRotationRate, RotationRange>
{
    required public init?(device:SenStickDevice)
    {
        super.init(device: device, sensorType: SenStickSensorType.gyroSensor)
    }
}
