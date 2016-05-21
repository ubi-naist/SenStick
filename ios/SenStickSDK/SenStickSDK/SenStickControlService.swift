//
//  SenStickControlService.swift
//  SenStickSDK
//
//  Created by AkihiroUehara on 2016/03/29.
//  Copyright © 2016年 AkihiroUehara. All rights reserved.
//

import Foundation
import CoreBluetooth

public protocol SenStickControlServiceDelegate : class
{
    func didStatusChanged(sender:SenStickControlService, status:SenStickStatus)
    func didAvailableLogCountChanged(sender:SenStickControlService, logCount: UInt8)
    func didDateTimeUpdate(sender:SenStickControlService, dateTime:NSDate)
    func didAbstractUpdate(sender:SenStickControlService, abstractText:String)
}

public class SenStickControlService : SenStickService
{
    // Variables
    unowned let device: SenStickDevice
    
    let statusChar:            CBCharacteristic
    let availableLogCountChar: CBCharacteristic
    let dateTimeChar:          CBCharacteristic
    let abstractChar:          CBCharacteristic

    public weak var delegate: SenStickControlServiceDelegate?
    
    // Properties, KVO-compatible
    var _status: SenStickStatus
    public private(set) var status: SenStickStatus {
        get {
            return _status
        }
        set(newValue) {
            _status = newValue
            delegate?.didStatusChanged(self, status: _status)
        }
    }
    
    var _availableLogCount: UInt8
    public private(set) var availableLogCount: UInt8 {
        get {
            return _availableLogCount
        }
        set(newValue) {
            _availableLogCount = newValue
            delegate?.didAvailableLogCountChanged(self, logCount: _availableLogCount)
        }
    }
    
    var _dateTime: NSDate
    public private(set) var dateTime: NSDate {
        get {
            return _dateTime
        }
        set(newValue) {
            _dateTime = newValue
            delegate?.didDateTimeUpdate(self, dateTime: _dateTime)
        }
    }

    var _abstractText: String
    public private(set) var abstractText: String {
        get {
            return _abstractText
        }
        set(newValue) {
            _abstractText = newValue
            delegate?.didAbstractUpdate(self, abstractText: _abstractText)
        }
    }
    
    // イニシャライザ
    required public init?(device:SenStickDevice)
    {
        self.device = device
        
        // サービス/キャラクタリスティクスがあることを確認
        guard let service = device.findService(SenStickUUIDs.ControlServiceUUID) else { return nil }
        guard let _statusChar            = device.findCharacteristic(service, uuid: SenStickUUIDs.StatusCharUUID) else { return nil }
        guard let _availableLogCountChar = device.findCharacteristic(service, uuid: SenStickUUIDs.AvailableLogCountCharUUID) else { return nil }
        guard let _dateTimeChar          = device.findCharacteristic(service, uuid: SenStickUUIDs.DateTimeCharUUID) else { return nil }
        guard let _abstractChar          = device.findCharacteristic(service, uuid: SenStickUUIDs.AbstractCharUUID) else { return nil }
        
        statusChar            = _statusChar
        availableLogCountChar = _availableLogCountChar
        dateTimeChar          = _dateTimeChar
        abstractChar          = _abstractChar
        
        _status               = .Stopping
        _availableLogCount    = 0
        _dateTime             = NSDate.distantPast()
        _abstractText         = ""
        
        self.delegate          = nil
        
        // Notifyを有効に。初期値読み出し。
        device.setNotify(statusChar, enabled: true)
        device.setNotify(availableLogCountChar, enabled: true)
        device.readValue(dateTimeChar)
        device.readValue(abstractChar)
    }
    
    // 値更新通知
    func didUpdateValue(characteristic: CBCharacteristic, data:[UInt8])
    {
        guard data.count > 0  else { return }
        
        switch characteristic.UUID {
        case SenStickUUIDs.StatusCharUUID:
            if let s = SenStickStatus(rawValue: data[0]) {
                self.status = s
            }
            
        case SenStickUUIDs.AvailableLogCountCharUUID:
            if availableLogCount != data[0] {
                self.dateTime = NSDate.distantPast()
            }
            availableLogCount = data[0]
            
        case SenStickUUIDs.DateTimeCharUUID:
            if let datetime = NSDate.unpack(data) {
                self.dateTime = datetime
            }
            
        case SenStickUUIDs.AbstractCharUUID:
            if let s = String(bytes: data, encoding: NSUTF8StringEncoding) {
                self.abstractText = s
            }
        default:
            debugPrint("\(#function) unexpedted characteristics UUID, \(characteristic).")
            break
        }
    }
    
    // Public methods
    public func readDateTime()
    {
        device.readValue(dateTimeChar)
    }

    public func writeDateTime(datetime: NSDate)
    {
        device.writeValue(dateTimeChar, value: datetime.pack())
        device.readValue(dateTimeChar)
    }
    
    public func readAbstract()
    {
        device.readValue(abstractChar)
    }
    public func writeAbstract(abstract: String)
    {
        device.writeValue(abstractChar, value: [UInt8](abstract.utf8))
        device.readValue(abstractChar)
    }
}