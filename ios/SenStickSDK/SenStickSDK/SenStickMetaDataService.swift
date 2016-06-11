//
//  SenStickMetaDataService.swift
//  SenStickSDK
//
//  Created by AkihiroUehara on 2016/04/15.
//  Copyright © 2016年 AkihiroUehara. All rights reserved.
//

import Foundation
import CoreBluetooth

public class SenStickMetaDataService : SenStickService
{
    // Variables
    unowned let device: SenStickDevice
    
    let targetLogIDChar:    CBCharacteristic
    let targetDateTimeChar: CBCharacteristic
    let targetAbstractChar: CBCharacteristic
    
    // Properties
    public private(set) var logID:    UInt8
    public private(set) var dateTime: NSDate
    public private(set) var abstract: String
    
    // イニシャライザ
    required public init?(device:SenStickDevice)
    {
        self.device = device

        guard let service = device.findService(SenStickUUIDs.MetaDataServiceUUID) else { return nil }
        guard let _targetLogIDChar    = device.findCharacteristic(service, uuid:SenStickUUIDs.TargetLogIDCharUUID) else { return nil }
        guard let _targetDateTimeChar = device.findCharacteristic(service, uuid:SenStickUUIDs.TargetDateTimeCharUUID) else { return nil }
        guard let _targetAbstractChar = device.findCharacteristic(service, uuid:SenStickUUIDs.TargetAbstractCharUUID) else { return nil }

        targetLogIDChar    = _targetLogIDChar
        targetDateTimeChar = _targetDateTimeChar
        targetAbstractChar = _targetAbstractChar
        
        self.logID = 0
        self.dateTime = NSDate.distantPast()
        self.abstract = ""
        
        // Notifyを有効に。初期値読み出し。
        requestMetaData(0)
    }
    
    // 値更新通知
    func didUpdateValue(characteristic: CBCharacteristic, data: [UInt8])
    {
        switch characteristic.UUID {
        case SenStickUUIDs.TargetLogIDCharUUID:
            logID = data[0]
            
        case SenStickUUIDs.TargetDateTimeCharUUID:
            if let date = NSDate.unpack(data) {
                self.dateTime = date
            }
            
        case SenStickUUIDs.TargetAbstractCharUUID:
            if let s = String(bytes: data, encoding: NSUTF8StringEncoding) {
                abstract = s
            } else {
                abstract = ""
            }

        default:
            debugPrint("\(#function): unexpected character: \(characteristic).")
            break
        }
    }
    
    // Public methods
    public func requestMetaData(logID: UInt8) {        
        device.writeValue(targetLogIDChar, value: [logID])
        device.readValue(targetLogIDChar)
        device.readValue(targetDateTimeChar)
        device.readValue(targetAbstractChar)
    }
}
