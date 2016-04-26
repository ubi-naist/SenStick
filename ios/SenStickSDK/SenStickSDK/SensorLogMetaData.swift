//
//  SensorLogMetaData.swift
//  SenStickSDK
//
//  Created by AkihiroUehara on 2016/04/22.
//  Copyright © 2016年 AkihiroUehara. All rights reserved.
//

import Foundation

public struct SensorLogMetaData<T: RawRepresentable where T.RawValue == UInt16> : PackableType
{
    let samplingDuration: SamplingDurationType
    let range: T
    let availableSampleCount: UInt32
    let position: UInt32
    let remainingCapacity: UInt8
    
    public func pack(byteOrder byteOrder: ByteOrder = .LittleEndian) -> [Byte]
    {
        var b = [UInt8]()
        
        b += self.samplingDuration.pack()
        b += self.range.rawValue.pack()
        b += self.availableSampleCount.pack()
        b += self.position.pack()
        b += self.remainingCapacity.pack()
        
        assert(b.count == 13)
        
        return b
    }
    
    public static func unpack<C: CollectionType where C.Generator.Element == Byte ,C.Index == Int>(data: C, byteOrder: ByteOrder = .LittleEndian) -> SensorLogMetaData?
    {
        let bytes = Array(data)
        
        guard bytes.count != 13 else {
            assert(false, #function)
            return nil
        }
        guard let duration = UInt16.unpack(bytes[0...1]) else {
            return nil
        }
        guard let rangeRawValue = UInt16.unpack(bytes[2...3]) else {
            return nil
        }
        guard let range = T(rawValue: rangeRawValue) else {
            return nil
        }
        guard let availableSampleCount = UInt32.unpack(bytes[4...7]) else {
            return nil
        }
        guard let position = UInt32.unpack(bytes[8...11]) else {
            return nil
        }
        let remainingCapacity = bytes[12]
        
        return SensorLogMetaData<T>(samplingDuration: SamplingDurationType(milliSeconds: duration),
                                    range:range, availableSampleCount: availableSampleCount, position: position, remainingCapacity: remainingCapacity)
    }
    
}
