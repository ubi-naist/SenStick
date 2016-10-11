//
//  SamplingDurationType.swift
//  SenStickSDK
//
//  Created by AkihiroUehara on 2016/04/20.
//  Copyright © 2016年 AkihiroUehara. All rights reserved.
//

import Foundation

// サンプリング・レート
public struct SamplingDurationType : CustomStringConvertible
{
    public let duration : TimeInterval
    
    public init(milliSeconds value: UInt16)
    {
        duration = 1e-3 * Double(value)
    }
    
    init(secondcs value: TimeInterval)
    {
        duration = value
    }
    
    public var description: String {
        return "\(duration * 1e3) milliseconds"
    }
    
    func milliSecondsAsUInt16() -> UInt16 {
        return UInt16(duration * 1e3)
    }
}

extension SamplingDurationType : PackableType
{
    public func pack(byteOrder: ByteOrder = .littleEndian) -> [Byte]
    {
        let durationMilliSec = Int16( duration * 1e3 )
        return durationMilliSec.pack(byteOrder: byteOrder)
    }
    public static func unpack<C: Collection>(_ data: C, byteOrder: ByteOrder = .littleEndian) -> SamplingDurationType? where C.Iterator.Element == Byte ,C.Index == Int
    {
        guard let value = UInt16.unpack(data, byteOrder: byteOrder) else {
            return nil
        }
        return SamplingDurationType.init(milliSeconds: value)
    }
}
