//
//  Packable.swift
//  SenStickSDK
//
//  Created by AkihiroUehara on 2016/04/19.
//  Copyright © 2016年 AkihiroUehara. All rights reserved.
//

import Foundation

import Foundation

public typealias Byte = UInt8

// おそらくSwiftのライブラリのバグだと思いますが
// ArraySliceのさらにSliceを作ろうとすると、元の配列でのインデックスでスライスを作ろうとして、でも今のスライスの範囲にはそれがないので、インデックス範囲外で例外発生します。
// 対処として内部でArrayに変換して使うようにします。
// http://qiita.com/sora0077@github/items/873f11f7e8d409130e28
/*
 let arr = [1, 2, 3, 4, 5]
 let sliced = arr[2..<4]
 // [3, 4]
 sliced[0..<1] // fatal error: Negative ArraySlice index is out of range 
 */

public protocol PackableType {
    func pack(byteOrder: ByteOrder) -> [Byte]
    static func unpack(_ data: Array<Byte>, byteOrder: ByteOrder) -> Self?
}

public enum ByteOrder : CustomStringConvertible {
    case littleEndian
    case bigEndian
    
    public var description: String {
        switch self {
        case .littleEndian: return "LittleEndian"
        case .bigEndian: return "BigEndian"
        }
    }
}

extension UInt8 : PackableType {
    public func pack(byteOrder: ByteOrder = .littleEndian) -> [Byte]
    {
        return [self]
    }
    
    public static func unpack<C: Collection>(_ data: C, byteOrder: ByteOrder = .littleEndian) -> UInt8? where C.Iterator.Element == Byte ,C.Index == Int
    {
        guard data.count >= 1 else {
            assert(false, #function)
            return nil
        }
        let bytes = Array(data)
        return bytes[0]
    }
}

extension Int8 : PackableType {
    public func pack(byteOrder: ByteOrder = .littleEndian) -> [Byte]
    {
        return [UInt8.init(bitPattern: self)]
    }
    
    public static func unpack<C: Collection>(_ data: C, byteOrder: ByteOrder = .littleEndian) -> Int8? where C.Iterator.Element == Byte ,C.Index == Int
    {
        guard data.count >= 1 else {
            assert(false, #function)
            return nil
        }
        let bytes = Array(data)
        return Int8.init(bitPattern: bytes[0])
    }
}

extension UInt16 : PackableType {
    public func pack(byteOrder: ByteOrder = .littleEndian) -> [Byte]
    {
        var buf = [UInt8](repeating: 0, count: 2)
        
        switch byteOrder {
        case .littleEndian:
            buf[0] = UInt8(UInt16(0x00ff) & self)
            buf[1] = UInt8(UInt16(0x00ff) & (self >> 8))
        case .bigEndian:
            buf[1] = UInt8(UInt16(0x00ff) & self)
            buf[0] = UInt8(UInt16(0x00ff) & (self >> 8))
        }
        
        return buf
    }
    
    public static func unpack<C: Collection>(_ data: C, byteOrder: ByteOrder = .littleEndian) -> UInt16? where C.Iterator.Element == Byte ,C.Index == Int
    {
        guard data.count >= 2 else {
            assert(false, #function)
            return nil
        }
        let bytes = Array(data)
        switch byteOrder {
        case .littleEndian:
            return UInt16(bytes[0])      + UInt16(bytes[1]) << 8
        case .bigEndian:
            return UInt16(bytes[0]) << 8 + UInt16(bytes[1])
        }
    }
}

extension Int16 : PackableType {
    public func pack(byteOrder: ByteOrder = .littleEndian) -> [Byte]
    {
        let v = UInt16(bitPattern: self)
        return v.pack(byteOrder: byteOrder)
    }
    
    public static func unpack<C: Collection>(_ data: C, byteOrder: ByteOrder = .littleEndian) -> Int16? where C.Iterator.Element == Byte ,C.Index == Int
    {
        guard let value = UInt16.unpack(data, byteOrder: byteOrder) else {
            assert(false, #function)
            return nil
        }
        return Int16(bitPattern: value)
    }
}

extension UInt32 : PackableType {
    public func pack(byteOrder: ByteOrder = .littleEndian) -> [Byte]
    {
        var buf = [UInt8](repeating: 0, count: 4)
        
        switch byteOrder {
        case .littleEndian:
            buf[0] = UInt8(UInt32(0x00ff) & (self >> 0))
            buf[1] = UInt8(UInt32(0x00ff) & (self >> 8))
            buf[2] = UInt8(UInt32(0x00ff) & (self >> 16))
            buf[3] = UInt8(UInt32(0x00ff) & (self >> 24))
            
        case .bigEndian:
            buf[3] = UInt8(UInt32(0x00ff) & (self >> 0))
            buf[2] = UInt8(UInt32(0x00ff) & (self >> 8))
            buf[1] = UInt8(UInt32(0x00ff) & (self >> 16))
            buf[0] = UInt8(UInt32(0x00ff) & (self >> 24))
        }
        
        return buf
    }
    
    public static func unpack<C: Collection>(_ data: C, byteOrder: ByteOrder = .littleEndian) -> UInt32? where C.Iterator.Element == Byte ,C.Index == Int
    {
        guard data.count >= 4 else {
            assert(false, #function)
            return nil
        }
        
        let bytes = Array(data)
        switch byteOrder {
        case .littleEndian:
            let b1 = UInt32(bytes[0])       + UInt32(bytes[1]) << 8
            let b2 = UInt32(bytes[2]) << 16 + UInt32(bytes[3]) << 24
            return b1 + b2
        case .bigEndian:
            let b1 = UInt32(bytes[0]) << 24 + UInt32(bytes[1]) << 16
            let b2 = UInt32(bytes[2]) << 8  + UInt32(bytes[3])
            return b1 + b2
        }
    }
}

extension Int32 : PackableType {
    public func pack(byteOrder: ByteOrder = .littleEndian) -> [Byte]
    {
        let v = UInt32(bitPattern: self)
        return v.pack(byteOrder: byteOrder)
    }
    
    public static func unpack<C: Collection>(_ data: C, byteOrder: ByteOrder = .littleEndian) -> Int32? where C.Iterator.Element == Byte ,C.Index == Int
    {
        guard let value = UInt32.unpack(data, byteOrder: byteOrder) else {
            assert(false, #function)            
            return nil
        }
        return Int32(bitPattern: value)
    }
}


extension _ArrayType where Iterator.Element == UInt8{
    func toHexString() -> String
    {
        var s = String()
        for (_, value) in enumerated() {
            s += String(format:"0x%02x,", value)
        }
        return s
    }
}
