//
//  SensorLogMetaData.swift
//  SenStickSDK
//
//  Created by AkihiroUehara on 2016/04/22.
//  Copyright © 2016年 AkihiroUehara. All rights reserved.
//

import Foundation

public struct SensorLogMetaData<T: RawRepresentable> : PackableType where T.RawValue == UInt16
{
    public let logID: UInt8
    public let samplingDuration: SamplingDurationType
    public let range: T
    public let availableSampleCount: UInt32
    public let position: UInt32
    public let remainingCapacity: UInt32
    
    public func pack(byteOrder: ByteOrder = .littleEndian) -> [Byte]
    {
        var b = [UInt8]()

        b += self.logID.pack()
        b += self.samplingDuration.pack()
        b += self.range.rawValue.pack()
        b += self.availableSampleCount.pack()
        b += self.position.pack()
        b += self.remainingCapacity.pack()
        
        assert(b.count == 17)
        
        return b
    }
    
    public static func unpack<C: Collection>(_ data: C, byteOrder: ByteOrder = .littleEndian) -> SensorLogMetaData? where C.Iterator.Element == Byte ,C.Index == Int
    {
        let bytes = Array(data)
        
        guard bytes.count == 17 else {
            assert(false, #function)
            return nil
        }
        
        let logID = bytes[0]
        let duration = UInt16.unpack(bytes[1..<3])!
        let rangeRawValue = UInt16.unpack(bytes[3..<5])!
        guard let range = T(rawValue: rangeRawValue) else {
            return nil
        }
        let availableSampleCount = UInt32.unpack(bytes[5..<9])!
        let position = UInt32.unpack(bytes[9..<13])!
//        let a = bytes[13...17]
//        debugPrint("\(a)")
        let remainingCapacity = UInt32.unpack(bytes[13..<17])!
        
        return SensorLogMetaData<T>(logID: logID, samplingDuration: SamplingDurationType(milliSeconds: duration),
                                    range:range, availableSampleCount: availableSampleCount, position: position, remainingCapacity: remainingCapacity)
    }
    
}
