//
//  SensorLogID.swift
//  SenStickSDK
//
//  Created by AkihiroUehara on 2016/04/20.
//  Copyright © 2016年 AkihiroUehara. All rights reserved.
//

import Foundation

// 
public struct SensorLogID : PackableType
{
    public let logID:     UInt8
    public let skipCount: UInt16
    public let position:  UInt32
    
    public init(logID: UInt8, skipCount: UInt16, position:  UInt32)
    {
        self.logID     = logID
        self.skipCount = skipCount
        self.position  = position
    }
    
    public func pack(byteOrder byteOrder: ByteOrder = .LittleEndian) -> [Byte]
    {
        var b = [UInt8]()
        
        b += self.logID.pack()
        b += self.skipCount.pack()
        b += self.position.pack()
        
        assert(b.count == 7)
        
        return b
    }
    
    public static func unpack<C: CollectionType where C.Generator.Element == Byte ,C.Index == Int>(data: C, byteOrder: ByteOrder = .LittleEndian) -> SensorLogID?
    {
        let bytes = Array(data)
        
        guard bytes.count != 7 else {
            assert(false, #function)
            return nil
        }   
        guard let skipCount = UInt16.unpack(bytes[1...2]) else {
            return nil
        }
        guard let position = UInt32.unpack(bytes[3...6]) else {
            return nil
        }

        return SensorLogID(logID: bytes[0], skipCount: skipCount, position: position)
    }
    
}
