//
//  MagneticSensorService.swift
//  SenStickSDK
//
//  Created by AkihiroUehara on 2016/05/24.
//  Copyright © 2016年 AkihiroUehara. All rights reserved.
//

import Foundation
import CoreMotion

public enum MagneticFieldRange : UInt16, CustomStringConvertible
{
    case magneticRangeDefault = 0x00
    
    public var description : String {
        switch self {
        case .magneticRangeDefault:    return "magneticRangeDefault"
        }
    }
}

// 磁界のデータ構造体
// 16-bit 符号付き数字 フルスケール f ±4800 μT
struct MagneticFieldRawData
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
    
    // 物理センサーの1uTあたりのLBSの値
    static func getLSBperuT(_ range: MagneticFieldRange) -> Double
    {
        switch range {
        case .magneticRangeDefault: return 1/0.15; // AK8963, 16-bit mode
        }
    }
    
    static func unpack(_ data: [Byte]) -> MagneticFieldRawData
    {
        let x = Int16.unpack(data[0..<2])
        let y = Int16.unpack(data[2..<4])
        let z = Int16.unpack(data[4..<6])
        
        return MagneticFieldRawData(xRawValue: x!, yRawValue: y!, zRawValue: z!)
    }
}

extension CMMagneticField : SensorDataPackableType
{
    public typealias RangeType = MagneticFieldRange
    
    public static func unpack(_ range:MagneticFieldRange, value: [UInt8]) -> CMMagneticField?
    {
        guard value.count >= 6 else {
            return nil
        }
        
        let rawData  = MagneticFieldRawData.unpack(value)
        let LSBperuT = MagneticFieldRawData.getLSBperuT(range)
        
        // FIXME 右手系/左手系などの座標変換など確認すること。
//debugPrint("x:\(rawData.xRawValue), y:\(rawData.yRawValue), z: \(rawData.zRawValue), lsbPerDeg:\(LSBperuT)")
        return CMMagneticField(x: Double(rawData.xRawValue) / Double(LSBperuT), y: Double(rawData.yRawValue) / Double(LSBperuT), z: Double(rawData.zRawValue) / Double(LSBperuT))
    }
}

// センサー各種のベースタイプ, Tはセンサデータ独自のデータ型, Sはサンプリングの型、
open class MagneticFieldSensorService: SenStickSensorService<CMMagneticField, MagneticFieldRange>, SenStickService
{
    required public init?(device:SenStickDevice)
    {
        super.init(device: device, sensorType: SenStickSensorType.magneticFieldSensor)
    }
}
