//
//  serialize.swift
//  SenStickSDK
//
//  Created by AkihiroUehara on 2016/03/18.
//  Copyright © 2016年 AkihiroUehara. All rights reserved.
//

import Foundation

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
    
    static func unpack(valueByteArray: [Byte], byteOrder: ByteOrder = .LittleEndian) -> Self?
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
    
    static func unpack(valueByteArray: [Byte], byteOrder: ByteOrder = .LittleEndian) -> Self?
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
    
    static func unpack(valueByteArray: [Byte], byteOrder: ByteOrder = .LittleEndian) -> Self?
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
    
    static func unpack(valueByteArray: [Byte], byteOrder: ByteOrder = .LittleEndian) -> Self?
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
    
    static func unpack(valueByteArray: [Byte], byteOrder: ByteOrder = .LittleEndian) -> Self?
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
    
    static func unpack(valueByteArray: [Byte], byteOrder: ByteOrder = .LittleEndian) -> Self?
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

extension SamplingDurationType : PackableType
{
    func pack(byteOrder byteOrder: ByteOrder) -> [Byte]
    {
        let durationMilliSec = Int16( duration * 1e3 )
        return durationMilliSec.pack(byteOrder)
    }
    static func unpack(valueByteArray: [Byte], byteOrder: ByteOrder = .LittleEndian) -> Self?
    {
        guard let value = Int16.unpack(valueByteArray, byteOrder) else {
            return nil
        }
        return SamplingDuration.init(milliSec: value)
    }
}

extension SensorSetting : PackableType {
    func pack(byteOrder byteOrder: ByteOrder) -> [Byte]
    {
        var b = [UInt8]()
        
        b += accelerationRange.rawValue.pack()
        b += rotationRange.rawValue.pack()
        b += accelerationSamplingPeriod.pack()
        b += gyroSamplingPeriod.pack()
        b += magneticFieldSamplingPeriod.pack()
        b += humidityAndTemperatureSamplingPeriod.pack()
        b += airPressureSamplingPeriod.pack()
        b += brightnessSamplingPeriod.pack()
        b += ultraVioletSamplingPeriod.pack()
        
        assert(b.count == 16)
        
        return b
    }
    
    static func unpack(valueByteArray: [Byte], byteOrder: ByteOrder) -> Self?
    {
        var setting = SensorSetting
        
        guard data.count >= 16 else {
            return nil
        }
        guard setting.accelerationRange = AccelerationRange(rawValue: data[0]) else {
            return nil
        }
        guard setting.accelerationRange = RotationRange(rawValue: data[1]) else {
            return nil
        }
        
        
        accelerationSamplingPeriod  = SamplingRateType(data[2])
        gyroSamplingPeriod          = SamplingRateType(data[4])
        magneticFieldSamplingPeriod = SamplingRateType(data[6])
        humidityAndTemperatureSamplingPeriod = SamplingRateType(data[8])
        airPressureSamplingPeriod   = SamplingRateType(data[10])
        brightnessSamplingPeriod    = SamplingRateType(data[12])
        ultraVioletSamplingPeriod   = SamplingRateType(data[14])
    }
}
