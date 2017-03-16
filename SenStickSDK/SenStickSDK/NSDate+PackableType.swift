//
//  NSDate+serialize.swift
//  SenStickSDK
//
//  Created by AkihiroUehara on 2016/04/15.
//  Copyright © 2016年 AkihiroUehara. All rights reserved.
//

import Foundation

extension Date { //: PackableType {
    public func pack(byteOrder: ByteOrder = .littleEndian) -> [Byte]
    {
        let calendar = Calendar.current
        let dateComponents = (calendar as NSCalendar).components([.year , .month , .day , .hour , .minute , .second], from: self)
        
        var buf = [UInt8](repeating: 0, count: 0)

        buf.append( contentsOf: UInt16(dateComponents.year!).pack() )
        buf.append( contentsOf: UInt8(dateComponents.month!).pack() )
        buf.append( contentsOf: UInt8(dateComponents.day!).pack()   )
        buf.append( contentsOf: UInt8(dateComponents.hour!).pack()  )
        buf.append( contentsOf: UInt8(dateComponents.minute!).pack())
        buf.append( contentsOf: UInt8(dateComponents.second!).pack())
        
        return buf
    }
    
    public static func unpack<C: Collection>(_ data: C, byteOrder: ByteOrder = .littleEndian) -> Date? where C.Iterator.Element == Byte ,C.Index == Int
    {
        let bytes = Array(data)
        
        guard bytes.count == 7 else {
            assert(false, #function)            
            return nil
        }
        
        var dateComponents = DateComponents()
        dateComponents.year   = Int(UInt16.unpack(bytes[0...1])!)
        dateComponents.month  = Int(bytes[2])
        dateComponents.day    = Int(bytes[3])
        dateComponents.hour   = Int(bytes[4])
        dateComponents.minute = Int(bytes[5])
        dateComponents.second = Int(bytes[6])
        
        let date = Calendar.current.date(from: dateComponents)
        return date
    }
}
