//
//  SenStickControlService.swift
//  SenStickSDK
//
//  Created by AkihiroUehara on 2016/03/29.
//  Copyright © 2016年 AkihiroUehara. All rights reserved.
//

import Foundation
import CoreBluetooth

public class SenStickControlService : NSObject, SenStickService
{
    // Variables
    unowned let device: SenStickDevice
    
    let statusChar:            CBCharacteristic
    let controlPointChar:      CBCharacteristic
    let availableLogCountChar: CBCharacteristic
    let dateTimeChar:          CBCharacteristic
    let abstractChar:          CBCharacteristic
    
    // Properties, KVO-compatible
    
    public private(set) var status: SenStickStatus
    public private(set) var availableLogCount: UInt8
    public private(set) var dateTime: NSDate
    public private(set) var abstract: String
    
    // イニシャライザ
    required public init?(device:SenStickDevice)
    {
        self.device = device
        
        // サービス/キャラクタリスティクスがあることを確認
        guard let service = device.findService(SenStickUUIDs.ControlServiceUUID) else { return nil }
        guard let _statusChar            = device.findCharacteristic(service, uuid: SenStickUUIDs.StatusCharUUID) else { return nil }
        guard let _controlChar           = device.findCharacteristic(service, uuid: SenStickUUIDs.ControlPointCharUUID) else { return nil }
        guard let _availableLogCountChar = device.findCharacteristic(service, uuid: SenStickUUIDs.AvailableLogCountCharUUID) else { return nil }
        guard let _dateTimeChar          = device.findCharacteristic(service, uuid: SenStickUUIDs.DateTimeCharUUID) else { return nil }
        guard let _abstractChar          = device.findCharacteristic(service, uuid: SenStickUUIDs.AbstractCharUUID) else { return nil }
        
        statusChar            = _statusChar
        controlPointChar      = _controlChar
        availableLogCountChar = _availableLogCountChar
        dateTimeChar          = _dateTimeChar
        abstractChar          = _abstractChar
        
        self.status            = .Stopping
        self.availableLogCount = 0
        self.dateTime          = NSDate.distantPast()
        self.abstract          = ""
        
        super.init()
        
        // Notifyを有効に。初期値読み出し。
        device.setNotify(statusChar, enabled: true)
        device.setNotify(controlPointChar, enabled: true)
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
                status = s
            }
            
        case SenStickUUIDs.AvailableLogCountCharUUID:
            if availableLogCount != data[0] {
                dateTime = NSDate.distantPast()
                abstract = ""
            }
            availableLogCount = data[0]
            
        case SenStickUUIDs.DateTimeCharUUID:
            if let datetime = NSDate.unpack(data) {
                dateTime = datetime
            }
            
        case SenStickUUIDs.AbstractCharUUID:
            if let s = String(bytes: data, encoding: NSUTF8StringEncoding) {
                abstract = s
            }
        default:
            debugPrint("\(#function) unexpedted characteristics UUID, \(characteristic).")
            break
        }
    }
    
    // Public methods
    public func writeDateTime(datetime: NSDate)
    {
        device.writeValue(dateTimeChar, value: datetime.pack())
        device.readValue(dateTimeChar)
    }
    
    public func writeAbstract(abstract: String)
    {
        device.writeValue(abstractChar, value: [UInt8](abstract.utf8))
        device.readValue(abstractChar)
    }
    
    public func writeCommand(command: SenStickControlCommand)
    {
        device.writeValue(controlPointChar, value: [command.rawValue])
    }
}