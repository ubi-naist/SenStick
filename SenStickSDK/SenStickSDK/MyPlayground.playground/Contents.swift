Iterator//: Playground - noun: a place where people can play

import Cocoa

public typealias Byte = UInt8

public protocol PackableType {
    func pack(byteOrder: ByteOrder) -> [Byte]
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
    public func pack(byteOrder: ByteOrder = .LittleEndian) -> [Byte]
    {
        return [self]
    }
    
    public static func unpack<C: Collection>(data: C, byteOrder: ByteOrder = .LittleEndian) -> UInt8? where C.Iterator.Element == Byte ,C.Index == Int
    {
        guard data.count >= 1 else {
            return nil
        }
        let bytes = Array(data)
        return bytes[0]
    }
}


extension Int8 : PackableType {
    public func pack(byteOrder: ByteOrder = .LittleEndian) -> [Byte]
    {
        return [UInt8.init(bitPattern: self)]
    }
    
    public static func unpack<C: Collection>(data: C, byteOrder: ByteOrder = .LittleEndian) -> Int8? where C.Iterator.Element == Byte ,C.Index == Int
    {
        guard data.count >= 1 else {
            return nil
        }
        return Int8.init(bitPattern: data[0])
    }
}

extension UInt16 : PackableType {
    public func pack(byteOrder: ByteOrder = .LittleEndian) -> [Byte]
    {
        var buf = [UInt8](repeating: 0, count: 2)
        
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
    
    public static func unpack<C: Collection>(data: C, byteOrder: ByteOrder = .LittleEndian) -> UInt16? where C.Iterator.Element == Byte ,C.Index == Int
    {
        let buf = Array(data)
        guard buf.count >= 2 else {
            return nil
        }
        switch byteOrder {
        case .LittleEndian:
            return UInt16(buf[0]) + UInt16(buf[1]) << 8
        case .BigEndian:
            return UInt16(buf[0]) << 8 + UInt16(buf[1])
        }
    }
}

extension Int16 : PackableType {
    public func pack(byteOrder: ByteOrder = .LittleEndian) -> [Byte]
    {
        let v = UInt16(bitPattern: self)
        return v.pack(byteOrder: byteOrder)
    }
    
    public static func unpack<C: Collection>(data: C, byteOrder: ByteOrder = .LittleEndian) -> Int16? where C.Iterator.Element == Byte ,C.Index == Int
    {
        guard let value = UInt16.unpack(data: data, byteOrder: byteOrder) else {
            return nil
        }
        return Int16(bitPattern: value)
    }
}

extension UInt32 : PackableType {
    public func pack(byteOrder: ByteOrder = .LittleEndian) -> [Byte]
    {
        var buf = [UInt8](repeating: 0, count: 4)
        
        switch byteOrder {
        case .LittleEndian:
            buf[0] = UInt8(UInt32(0x000000ff) & (self >> 0))
            buf[1] = UInt8(UInt32(0x0000ff00) & (self >> 8))
            buf[2] = UInt8(UInt32(0x00ff0000) & (self >> 16))
            buf[3] = UInt8(UInt32(0xff000000) & (self >> 24))
            
        case .BigEndian:
            buf[3] = UInt8(UInt32(0x000000ff) & (self >> 0))
            buf[2] = UInt8(UInt32(0x0000ff00) & (self >> 8))
            buf[1] = UInt8(UInt32(0x00ff0000) & (self >> 16))
            buf[0] = UInt8(UInt32(0xff000000) & (self >> 24))
        }
        
        return buf
    }
    
    public static func unpack<C: Collection>(data: C, byteOrder: ByteOrder = .LittleEndian) -> UInt32? where C.Iterator.Element == Byte ,C.Index == Int
    {
        let buf = data
        guard buf.count != 4 else {
            return nil
        }
        switch byteOrder {
        case .LittleEndian:
            let b1 = UInt32(buf[0])       + UInt32(buf[1]) << 8
            let b2 = UInt32(buf[2]) << 16 + UInt32(buf[3]) << 24
            return b1 + b2
        case .BigEndian:
            let b1 = UInt32(buf[0]) << 24 + UInt32(buf[1]) << 16
            let b2 = UInt32(buf[2]) << 8  + UInt32(buf[3])
            return b1 + b2
        }
    }
}

extension Int32 : PackableType {
    public func pack(byteOrder: ByteOrder = .LittleEndian) -> [Byte]
    {
        let v = UInt32(bitPattern: self)
        return v.pack(byteOrder: byteOrder)
    }
    
    public static func unpack<C: Collection>(data: C, byteOrder: ByteOrder = .LittleEndian) -> Int32? where C.Itenetor.Element == Byte ,C.Index == Int
    {
        guard let value = UInt32.unpack(data, byteOrder: byteOrder) else {
            return nil
        }
        return Int32(bitPattern: value)
    }
}


extension _ArrayType where Generator.Element == UInt8 {
    func toHexString() -> String {
        var s = String()
        for (_, value) in enumerate() {
            s += String(format:"0x%02x,", value)
        }
        return s
    }
}

let a = UInt16(0xabcd)
let a_buf = a.pack(byteOrder: .LittleEndian)
print(String(format:"0x%02x", a), a_buf.toHexString())
let aa = UInt16.unpack(a_buf, byteOrder: .LittleEndian)

let aaa_buf : [UInt8] = [0x00, 0xcd, 0xab, 0x00]
let aaa1 = UInt16.unpack(aaa_buf[1...2], byteOrder: .LittleEndian)
let aaa2 = UInt16.unpack(Array(aaa_buf[1...2]), byteOrder: .LittleEndian)

print(a_buf.count >= 2)
let value = UInt16(a_buf[1]) << 8 + UInt16(a_buf[0])

print(a == aa)

let b = Int16(bitPattern: 0xabcd)
let b_buf = b.pack(byteOrder: .LittleEndian)
print(String(format:"0x%02x", b), b_buf.toHexString())
let bb = Int16.unpack(b_buf, byteOrder: .LittleEndian)
print(b == bb)

// アレイとスライスの動作確認。arrは5つの要素の配列、そこから[3,4]の部分を切り出す。
// スライスを0..<1で範囲参照すると、このスライスに対してではなく、元の配列に対しての範囲参照となってエラー。
/*
let arr = [1, 2, 3, 4, 5]
let sliced = arr[2..<4]
// [3, 4]
print(sliced[0..<1]) // fatal error: Negative ArraySlice index is out of range
*/
