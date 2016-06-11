//
//  DeviceInformationService.swift
//  SenStickSDK
//
//  Created by AkihiroUehara on 2016/06/11.
//  Copyright © 2016年 AkihiroUehara. All rights reserved.
//

import Foundation
import CoreBluetooth

public class DeviceInformationService : NSObject, SenStickService //, CustomStringConvertible
{
    // Variables
    unowned let device: SenStickDevice
    
    // Properties, KVO
    dynamic public private(set) var manufacturerName: String
    dynamic public private(set) var hardwareRevision: String
    dynamic public private(set) var firmwareRevision: String
    dynamic public private(set) var serialNumber: String
    
    // イニシャライザ
    required public init?(device:SenStickDevice)
    {
        self.device = device
        
        guard let service = device.findService(SenStickUUIDs.DeviceInformationServiceUUID) else { return nil }

        // 値を読み出し
        if let char = device.findCharacteristic(service, uuid:SenStickUUIDs.ManufacturerNameStringCharUUID) {
            device.readValue(char)
        }
        if let char = device.findCharacteristic(service, uuid:SenStickUUIDs.HardwareRevisionStringCharUUID) {
            device.readValue(char)
        }
        if let char = device.findCharacteristic(service, uuid:SenStickUUIDs.FirmwareRevisionStringCharUUID) {
            device.readValue(char)
        }
        if let char = device.findCharacteristic(service, uuid:SenStickUUIDs.SerialNumberStringCharUUID) {
            device.readValue(char)
        }

        self.manufacturerName = ""
        self.hardwareRevision = ""
        self.firmwareRevision = ""
        self.serialNumber     = ""
    }
    
    // 値更新通知
    func didUpdateValue(characteristic: CBCharacteristic, data: [UInt8])
    {
        switch characteristic.UUID {
        case SenStickUUIDs.ManufacturerNameStringCharUUID:
            if let s = String(bytes: data, encoding: NSUTF8StringEncoding) {
                self.manufacturerName = s
            }
            
        case SenStickUUIDs.HardwareRevisionStringCharUUID:
            if let s = String(bytes: data, encoding: NSUTF8StringEncoding) {
                self.hardwareRevision = s
            }
            
        case SenStickUUIDs.FirmwareRevisionStringCharUUID:
            if let s = String(bytes: data, encoding: NSUTF8StringEncoding) {
                self.firmwareRevision = s
            }
            
        case SenStickUUIDs.SerialNumberStringCharUUID:
            if let s = String(bytes: data, encoding: NSUTF8StringEncoding) {
                self.serialNumber = s
            }
            
        default:
            debugPrint("\(#function): unexpected character: \(characteristic).")
            break
        }
    }
    
    // CustomStringConvertible
    override public var description: String {
        get {
            return "Device information: \n  Manufacturer: \(manufacturerName)\n  Hardware revision:\(hardwareRevision)\n  Firmware revision:\(firmwareRevision)\n  Serial number:\(serialNumber)"
        }
    }
}
