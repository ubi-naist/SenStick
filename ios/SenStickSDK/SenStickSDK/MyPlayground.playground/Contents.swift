//: Playground - noun: a place where people can play

import Cocoa

// http://codereview.stackexchange.com/questions/114730/type-to-byte-array-conversion-in-swift
enum ByteOrder {
    case LittleEndian
    case BigEndian
}

typealias Byte = UInt8

protocol PackableType {
    func pack(byteOrder byteOrder: ByteOrder) -> [Byte]
    static func unpack(valueByteArray: [Byte], byteOrder: ByteOrder) -> Self?
}

extension UInt8 : PackableType {
    func pack(byteOrder byteOrder: ByteOrder = .LittleEndian) -> [Byte]
    {
        return [self]
    }
    
    static func unpack(valueByteArray: [Byte], byteOrder: ByteOrder = .LittleEndian) -> UInt8?
    {
        guard valueByteArray.count >= 1 else {
            return nil
        }
        return valueByteArray[0]
    }
}

extension Int8 : PackableType {
    func pack(byteOrder byteOrder: ByteOrder = .LittleEndian) -> [Byte]
    {
        return [UInt8.init(bitPattern: self)]
    }
    
    static func unpack(valueByteArray: [Byte], byteOrder: ByteOrder = .LittleEndian) -> Int8?
    {
        guard valueByteArray.count >= 1 else {
            return nil
        }
        return Int8.init(bitPattern: valueByteArray[0])
    }
}

extension UInt16 : PackableType {
    func pack(byteOrder byteOrder: ByteOrder = .LittleEndian) -> [Byte]
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
    
    static func unpack(valueByteArray: [Byte], byteOrder: ByteOrder = .LittleEndian) -> UInt16?
    {
        let buf = valueByteArray
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
    func pack(byteOrder byteOrder: ByteOrder = .LittleEndian) -> [Byte]
    {
        let v = UInt16(bitPattern: self)
        return v.pack(byteOrder: byteOrder)
    }
    
    static func unpack(valueByteArray: [Byte], byteOrder: ByteOrder = .LittleEndian) -> Int16?
    {
        guard let value = UInt16.unpack(valueByteArray, byteOrder: byteOrder) else {
            return nil
        }
        return Int16(bitPattern: value)
    }
}

extension UInt32 : PackableType {
    func pack(byteOrder byteOrder: ByteOrder = .LittleEndian) -> [Byte]
    {
        var buf = [UInt8](count: 4, repeatedValue: 0)
        
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
    
    static func unpack(valueByteArray: [Byte], byteOrder: ByteOrder = .LittleEndian) -> UInt32?
    {
        let buf = valueByteArray
        guard buf.count >= 4 else {
            return nil
        }
        switch byteOrder {
        case .LittleEndian:
            return UInt32(buf[0])       + UInt32(buf[1]) << 8  + UInt32(buf[2]) << 16 + UInt32(buf[3]) << 24
        case .BigEndian:
            return UInt32(buf[0]) << 24 + UInt32(buf[1]) << 16 + UInt32(buf[2]) << 8  + UInt32(buf[3])
        }
    }
}

extension Int32 : PackableType {
    func pack(byteOrder byteOrder: ByteOrder = .LittleEndian) -> [Byte]
    {
        let v = UInt32(bitPattern: self)
        return v.pack(byteOrder: byteOrder)
    }
    
    static func unpack(valueByteArray: [Byte], byteOrder: ByteOrder = .LittleEndian) -> Int16?
    {
        guard let value = UInt32.unpack(valueByteArray, byteOrder: byteOrder) else {
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

print(a_buf.count >= 2)
let value = UInt16(a_buf[1]) << 8 + UInt16(a_buf[0])

print(a == aa)

let b = Int16(bitPattern: 0xabcd)
let b_buf = b.pack(byteOrder: .LittleEndian)
print(String(format:"0x%02x", b), b_buf.toHexString())
let bb = Int16.unpack(b_buf, byteOrder: .LittleEndian)
print(b == bb)

