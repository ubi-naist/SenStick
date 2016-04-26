//
//  SensorSetting.swift
//  SenStickSDK
//
//  Created by AkihiroUehara on 2016/04/19.
//  Copyright © 2016年 AkihiroUehara. All rights reserved.
//

import Foundation

public struct SensorSettingData<T: RawRepresentable where T.RawValue == UInt16> : CustomStringConvertible, PackableType
{
    let status:           SenStickStatus
    let samplingDuration: SamplingDurationType
    let range:            T
    
    public var description : String
    {
        return "SensorSetting: status \(status), sampling duration \(samplingDuration) ms, range \(range)."
    }
    
    public func pack(byteOrder byteOrder: ByteOrder = .LittleEndian) -> [Byte]
    {
        var b = [UInt8]()
        
        b += self.status.rawValue.pack()
        b += self.samplingDuration.pack()
        b += self.range.rawValue.pack()
        
        assert(b.count == 5)
        
        return b
    }

    public static func unpack<C: CollectionType where C.Generator.Element == Byte ,C.Index == Int>(data: C, byteOrder: ByteOrder = .LittleEndian) -> SensorSettingData?
    {
        let bytes = Array(data)
        
        guard bytes.count != 5 else {
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
        guard let range = T(rawValue: rangeRawValue) else {
            return nil
        }
        
        return SensorSettingData(status: status, samplingDuration: SamplingDurationType(milliSeconds: samplingDuration), range: range)
    }    
}
