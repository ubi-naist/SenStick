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
    public let duration : NSTimeInterval
    
    public init(milliSeconds value: UInt16)
    {
        duration = 1e-3 * Double(value)
    }
    
    init(secondcs value: NSTimeInterval)
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
    public func pack(byteOrder byteOrder: ByteOrder = .LittleEndian) -> [Byte]
    {
        let durationMilliSec = Int16( duration * 1e3 )
        return durationMilliSec.pack(byteOrder: byteOrder)
    }
    public static func unpack<C: CollectionType where C.Generator.Element == Byte ,C.Index == Int>(data: C, byteOrder: ByteOrder = .LittleEndian) -> SamplingDurationType?
    {
        guard let value = UInt16.unpack(data, byteOrder: byteOrder) else {
            return nil
        }
        return SamplingDurationType.init(milliSeconds: value)
    }
}
