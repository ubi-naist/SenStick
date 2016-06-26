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
    func didCommandChanged(sender:SenStickControlService, command:SenStickControlCommand)
    func didAvailableLogCountChanged(sender:SenStickControlService, logCount: UInt8)
    func didStorageStatusChanged(sender:SenStickControlService, storageStatus: Bool)
    func didDateTimeUpdate(sender:SenStickControlService, dateTime:NSDate)
    func didAbstractUpdate(sender:SenStickControlService, abstractText:String)
    func didDeviceNameUpdate(sender:SenStickControlService, deviceName:String)
}

public class SenStickControlService : SenStickService
{
    // Variables
    unowned let device: SenStickDevice
    
    let statusChar:            CBCharacteristic
    let storageStatusChar:     CBCharacteristic
    let availableLogCountChar: CBCharacteristic
    let dateTimeChar:          CBCharacteristic
    let abstractChar:          CBCharacteristic
    let deviceNameChar:        CBCharacteristic?
    
    public weak var delegate: SenStickControlServiceDelegate?
    
    // Properties, KVO-compatible
    public private(set) var command: SenStickControlCommand {
        didSet{
            dispatch_async(dispatch_get_main_queue(), {
                self.delegate?.didCommandChanged(self, command: self.command)
            })
            // FIXME ログカウント値が通知されないので、能動的にここで読み出す
            readAvailableLogCount()
        }
    }

    public private(set) var storageStatus: Bool {
        didSet {
            dispatch_async(dispatch_get_main_queue(), {
                self.delegate?.didStorageStatusChanged(self, storageStatus: self.storageStatus)
            })
        }
    }
    
    public private(set) var availableLogCount: UInt8 {
        didSet {
            dispatch_async(dispatch_get_main_queue(), {
                self.delegate?.didAvailableLogCountChanged(self, logCount: self.availableLogCount)
            })
        }
    }
    
    public private(set) var dateTime: NSDate {
        didSet {
            dispatch_async(dispatch_get_main_queue(), {
                self.delegate?.didDateTimeUpdate(self, dateTime: self.dateTime)
            })
        }
    }

    public private(set) var abstractText: String {
        didSet {
            dispatch_async(dispatch_get_main_queue(), {
                self.delegate?.didAbstractUpdate(self, abstractText: self.abstractText)
            })
        }
    }
    
    public private(set) var deviceName: String {
        didSet {
            dispatch_async(dispatch_get_main_queue(), {
                self.delegate?.didDeviceNameUpdate(self, deviceName: self.deviceName)
            })
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
        guard let _storageStatusChar     = device.findCharacteristic(service, uuid: SenStickUUIDs.StorageStatusCharUUID) else { return nil }
        guard let _dateTimeChar          = device.findCharacteristic(service, uuid: SenStickUUIDs.DateTimeCharUUID) else { return nil }
        guard let _abstractChar          = device.findCharacteristic(service, uuid: SenStickUUIDs.AbstractCharUUID) else { return nil }
        
        // FIXME: DFUのため、この追加キャラクタリスティクスがなくても動くように
//        guard let _deviceNameChar        = device.findCharacteristic(service, uuid: SenStickUUIDs.DeviceNameCharUUID) else { return nil }
        
        statusChar            = _statusChar
        availableLogCountChar = _availableLogCountChar
        storageStatusChar     = _storageStatusChar
        dateTimeChar          = _dateTimeChar
        abstractChar          = _abstractChar
//        deviceNameChar       = _deviceNameChar
        deviceNameChar = device.findCharacteristic(service, uuid: SenStickUUIDs.DeviceNameCharUUID)
        
        self.delegate         = nil
        
        self.command              = .Stopping
        self.availableLogCount    = 0
        self.storageStatus        = false
        self.dateTime             = NSDate.distantPast()
        self.abstractText         = ""
        self.deviceName           = ""
        
        // Notifyを有効に。初期値読み出し。
        device.setNotify(statusChar, enabled: true)
        device.setNotify(availableLogCountChar, enabled: true)
        device.setNotify(storageStatusChar, enabled: true)
        
        device.readValue(statusChar)
        device.readValue(availableLogCountChar)
        device.readValue(storageStatusChar)
        device.readValue(dateTimeChar)
        device.readValue(abstractChar)
        if deviceNameChar != nil {
            device.readValue(deviceNameChar!)
        }
    }
    
    // 値更新通知
    func didUpdateValue(characteristic: CBCharacteristic, data:[UInt8])
    {
        guard data.count > 0  else { return }
        
        switch characteristic.UUID {
        case SenStickUUIDs.StatusCharUUID:
            if let s = SenStickControlCommand(rawValue: data[0]) {
                self.command = s
            }
            
        case SenStickUUIDs.AvailableLogCountCharUUID:
            availableLogCount = data[0]
            
        case SenStickUUIDs.StorageStatusCharUUID:
            self.storageStatus = (data[0] != 0)
            
        case SenStickUUIDs.DateTimeCharUUID:
            if let datetime = NSDate.unpack(data) {
                self.dateTime = datetime
            }
            
        case SenStickUUIDs.AbstractCharUUID:
            if let s = String(bytes: data, encoding: NSUTF8StringEncoding) {
                self.abstractText = s
            }
            
        case SenStickUUIDs.DeviceNameCharUUID:
            if let s = String(bytes: data, encoding: NSUTF8StringEncoding) {
                self.deviceName = s
            }
            
        default:
            debugPrint("\(#function) unexpedted characteristics UUID, \(characteristic).")
            break
        }
    }
    
    // Public methods
    public func writeCommand(command:SenStickControlCommand)
    {
        device.writeValue(statusChar, value: command.rawValue.pack())
        device.readValue(statusChar)
    }
    
    public func readAvailableLogCount()
    {
        device.readValue(availableLogCountChar)
    }

    public func readStorageStatus()
    {
        device.readValue(storageStatusChar)
    }
    
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
    
    public func writeDeviceName(deviceName: String)
    {
        if deviceNameChar == nil {
            return
        }
        
        let data = [UInt8](deviceName.utf8)

        // MAGIC NUMBER, GATTに20バイト以上書き込むことはできない。
        if data.count > 0 || data.count <= 20 {
            device.writeValue(deviceNameChar!, value: data)
        }
        
        device.readValue(deviceNameChar!)
    }
}
