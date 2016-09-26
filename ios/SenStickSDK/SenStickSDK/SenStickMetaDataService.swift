//
//  SenStickMetaDataService.swift
//  SenStickSDK
//
//  Created by AkihiroUehara on 2016/04/15.
//  Copyright © 2016年 AkihiroUehara. All rights reserved.
//

import Foundation
import CoreBluetooth

public protocol SenStickMetaDataServiceDelegate : class
{
    func didUpdateMetaData(_ sender:SenStickMetaDataService)
}

open class SenStickMetaDataService : SenStickService
{
    open weak var delegate: SenStickMetaDataServiceDelegate?
    
    // Variables
    unowned let device: SenStickDevice
    
    let targetLogIDChar:    CBCharacteristic
    let targetDateTimeChar: CBCharacteristic
    let targetAbstractChar: CBCharacteristic
    
    // Properties
    open fileprivate(set) var logID:    UInt8
    open fileprivate(set) var dateTime: Date
    open fileprivate(set) var abstract: String
    
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
        self.dateTime = Date.distantPast
        self.abstract = ""
        
        // Notifyを有効に。初期値読み出し。
        requestMetaData(0)
    }
    
    // 値更新通知
    func didUpdateValue(_ characteristic: CBCharacteristic, data: [UInt8])
    {
        switch characteristic.uuid {
        case SenStickUUIDs.TargetLogIDCharUUID:
            DispatchQueue.main.async(execute: {
                self.logID = data[0]
                self.delegate?.didUpdateMetaData(self)
            })
            
        case SenStickUUIDs.TargetDateTimeCharUUID:
            if let date = Date.unpack(data) {
                DispatchQueue.main.async(execute: {
                    self.dateTime = date
                })
            }
            
        case SenStickUUIDs.TargetAbstractCharUUID:
            if let s = String(bytes: data, encoding: String.Encoding.utf8) {
                DispatchQueue.main.async(execute: {
                    self.abstract = s
                })
            } else {
                DispatchQueue.main.async(execute: {
                    self.abstract = ""
                })
            }

        default:
            debugPrint("\(#function): unexpected character: \(characteristic).")
            break
        }
    }
    
    // Public methods
    open func requestMetaData(_ logID: UInt8) {        
        device.writeValue(targetLogIDChar, value: [logID])
        device.readValue(targetDateTimeChar)
//        device.readValue(targetAbstractChar)
        device.readValue(targetLogIDChar)
    }
}
