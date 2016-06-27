//
//  NSDate+serialize.swift
//  SenStickSDK
//
//  Created by AkihiroUehara on 2016/04/15.
//  Copyright © 2016年 AkihiroUehara. All rights reserved.
//

import Foundation

extension NSDate { //: PackableType {
    public func pack(byteOrder byteOrder: ByteOrder = .LittleEndian) -> [Byte]
    {
        let calendar = NSCalendar.currentCalendar()
        let dateComponents = calendar.components([.Year , .Month , .Day , .Hour , .Minute , .Second], fromDate: self)
        
        var buf = [UInt8](count: 0, repeatedValue: 0)

        buf.appendContentsOf( UInt16(dateComponents.year).pack() )
        buf.appendContentsOf( UInt8(dateComponents.month).pack() )
        buf.appendContentsOf( UInt8(dateComponents.day).pack()   )
        buf.appendContentsOf( UInt8(dateComponents.hour).pack()  )
        buf.appendContentsOf( UInt8(dateComponents.minute).pack())
        buf.appendContentsOf( UInt8(dateComponents.second).pack())
        
        return buf
    }
    
    public static func unpack<C: CollectionType where C.Generator.Element == Byte ,C.Index == Int>(data: C, byteOrder: ByteOrder = .LittleEndian) -> NSDate?
    {
        let bytes = Array(data)
        
        guard bytes.count == 7 else {
            assert(false, #function)            
            return nil
        }
        
        let dateComponents = NSDateComponents()
        dateComponents.calendar = NSCalendar.currentCalendar()

        dateComponents.year   = Int(UInt16.unpack(bytes[0...1])!)
        dateComponents.month  = Int(bytes[2])
        dateComponents.day    = Int(bytes[3])
        dateComponents.hour   = Int(bytes[4])
        dateComponents.minute = Int(bytes[5])
        dateComponents.second = Int(bytes[6])
        
        return dateComponents.date
    }
}
