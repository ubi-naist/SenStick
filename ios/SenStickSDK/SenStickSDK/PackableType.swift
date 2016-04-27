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
    func pack(byteOrder byteOrder: ByteOrder) -> [Byte]
    static func unpack(data: Array<Byte>, byteOrder: ByteOrder) -> Self?
}

public enum ByteOrder : CustomStringConvertible {
    case LittleEndian
    case BigEndian
    
    public var description: String {
        switch self {
        case .LittleEndian: return "LittleEndian"
        case .BigEndian: return "BigEndian"
        }
    }
}

extension UInt8 : PackableType {
    public func pack(byteOrder byteOrder: ByteOrder = .LittleEndian) -> [Byte]
    {
        return [self]
    }
    
    public static func unpack<C: CollectionType where C.Generator.Element == Byte ,C.Index == Int>(data: C, byteOrder: ByteOrder = .LittleEndian) -> UInt8?
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
    public func pack(byteOrder byteOrder: ByteOrder = .LittleEndian) -> [Byte]
    {
        return [UInt8.init(bitPattern: self)]
    }
    
    public static func unpack<C: CollectionType where C.Generator.Element == Byte ,C.Index == Int>(data: C, byteOrder: ByteOrder = .LittleEndian) -> Int8?
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
    public func pack(byteOrder byteOrder: ByteOrder = .LittleEndian) -> [Byte]
    {
        var buf = [UInt8](count: 2, repeatedValue: 0)
        
        switch byteOrder {
        case .LittleEndian:
            buf[0] = UInt8(UInt16(0x00ff) & self)
            buf[1] = UInt8(UInt16(0x00ff) & (self >> 8))
        case .BigEndian:
            buf[1] = UInt8(UInt16(0x00ff) & self)
            buf[0] = UInt8(UInt16(0x00ff) & (self >> 8))
        }
        
        return buf
    }
    
    public static func unpack<C: CollectionType where C.Generator.Element == Byte ,C.Index == Int>(data: C, byteOrder: ByteOrder = .LittleEndian) -> UInt16?
    {
        guard data.count >= 2 else {
            assert(false, #function)
            return nil
        }
        let bytes = Array(data)
        switch byteOrder {
        case .LittleEndian:
            return UInt16(bytes[0])      + UInt16(bytes[1]) << 8
        case .BigEndian:
            return UInt16(bytes[0]) << 8 + UInt16(bytes[1])
        }
    }
}

extension Int16 : PackableType {
    public func pack(byteOrder byteOrder: ByteOrder = .LittleEndian) -> [Byte]
    {
        let v = UInt16(bitPattern: self)
        return v.pack(byteOrder: byteOrder)
    }
    
    public static func unpack<C: CollectionType where C.Generator.Element == Byte ,C.Index == Int>(data: C, byteOrder: ByteOrder = .LittleEndian) -> Int16?
    {
        guard let value = UInt16.unpack(data, byteOrder: byteOrder) else {
            assert(false, #function)
            return nil
        }
        return Int16(bitPattern: value)
    }
}

extension UInt32 : PackableType {
    public func pack(byteOrder byteOrder: ByteOrder = .LittleEndian) -> [Byte]
    {
        var buf = [UInt8](count: 4, repeatedValue: 0)
        
        switch byteOrder {
        case .LittleEndian:
            buf[0] = UInt8(UInt32(0x00ff) & (self >> 0))
            buf[1] = UInt8(UInt32(0x00ff) & (self >> 8))
            buf[2] = UInt8(UInt32(0x00ff) & (self >> 16))
            buf[3] = UInt8(UInt32(0x00ff) & (self >> 24))
            
        case .BigEndian:
            buf[3] = UInt8(UInt32(0x00ff) & (self >> 0))
            buf[2] = UInt8(UInt32(0x00ff) & (self >> 8))
            buf[1] = UInt8(UInt32(0x00ff) & (self >> 16))
            buf[0] = UInt8(UInt32(0x00ff) & (self >> 24))
        }
        
        return buf
    }
    
    public static func unpack<C: CollectionType where C.Generator.Element == Byte ,C.Index == Int>(data: C, byteOrder: ByteOrder = .LittleEndian) -> UInt32?
    {
        guard data.count >= 4 else {
            assert(false, #function)
            return nil
        }
        
        let bytes = Array(data)
        switch byteOrder {
        case .LittleEndian:
            let b1 = UInt32(bytes[0])       + UInt32(bytes[1]) << 8
            let b2 = UInt32(bytes[2]) << 16 + UInt32(bytes[3]) << 24
            return b1 + b2
        case .BigEndian:
            let b1 = UInt32(bytes[0]) << 24 + UInt32(bytes[1]) << 16
            let b2 = UInt32(bytes[2]) << 8  + UInt32(bytes[3])
            return b1 + b2
        }
    }
}

extension Int32 : PackableType {
    public func pack(byteOrder byteOrder: ByteOrder = .LittleEndian) -> [Byte]
    {
        let v = UInt32(bitPattern: self)
        return v.pack(byteOrder: byteOrder)
    }
    
    public static func unpack<C: CollectionType where C.Generator.Element == Byte ,C.Index == Int>(data: C, byteOrder: ByteOrder = .LittleEndian) -> Int32?
    {
        guard let value = UInt32.unpack(data, byteOrder: byteOrder) else {
            assert(false, #function)            
            return nil
        }
        return Int32(bitPattern: value)
    }
}


extension _ArrayType where Generator.Element == UInt8{
    func toHexString() -> String
    {
        var s = String()
        for (_, value) in enumerate() {
            s += String(format:"0x%02x,", value)
        }
        return s
    }
}