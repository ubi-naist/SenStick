//
//  SensorSetting.swift
//  SenStickSDK
//
//  Created by AkihiroUehara on 2016/04/19.
//  Copyright © 2016年 AkihiroUehara. All rights reserved.
//

import Foundation

public struct SensorSettingData<RangeType: RawRepresentable> : CustomStringConvertible, PackableType where RangeType.RawValue == UInt16
{
    public let status:           SenStickStatus
    public let samplingDuration: SamplingDurationType
    public let range:            RangeType
    
    public init(status:  SenStickStatus, samplingDuration:SamplingDurationType, range: RangeType)
    {
        self.status = status
        self.samplingDuration = samplingDuration
        self.range = range
    }
    
    public var description : String
    {
        return "SensorSetting: status \(status), sampling duration \(samplingDuration) ms, range \(range)."
    }
    
    public func pack(byteOrder: ByteOrder = .littleEndian) -> [Byte]
    {
        var b = [UInt8]()
        
        b += self.status.rawValue.pack()
        b += self.samplingDuration.pack()
        b += self.range.rawValue.pack()
        
        assert(b.count == 5)
        
        return b
    }

    public static func unpack<C: Collection>(_ data: C, byteOrder: ByteOrder = .littleEndian) -> SensorSettingData? where C.Iterator.Element == Byte ,C.Index == Int
    {
        let bytes = Array(data)
        
        guard bytes.count == 5 else {
            assert(false, #function)
            return nil
        }
        guard let status = SenStickStatus(rawValue: bytes[0]) else {
            return nil
        }
        guard let samplingDuration =  UInt16.unpack(bytes[1...2]) else {
            return nil
        }
        guard let rangeRawValue = UInt16.unpack(bytes[3...4]) else {
            return nil
        }
        guard let range = RangeType(rawValue: rangeRawValue) else {
            return nil
        }
        
        return SensorSettingData(status: status, samplingDuration: SamplingDurationType(milliSeconds: samplingDuration), range: range)
    }    
}
