//
//  BatteryService.swift
//  SenStickSDK
//
//  Created by AkihiroUehara on 2016/06/11.
//  Copyright © 2016年 AkihiroUehara. All rights reserved.
//

import Foundation
import CoreBluetooth

public class BatteryService : NSObject, SenStickService //, CustomStringConvertible
{
    // Variables
    unowned let device: SenStickDevice
    
    // Properties, KVO
    dynamic public private(set) var batteryLevel: Int
    
    let batteryLevelChar:            CBCharacteristic
    
    // イニシャライザ
    required public init?(device:SenStickDevice)
    {
        self.device = device
        
        guard let service = device.findService(SenStickUUIDs.BatteryServiceUUID) else { return nil }
        guard let _batteryLevelChar = device.findCharacteristic(service, uuid:SenStickUUIDs.BatteryLevelCharUUID) else { return nil }
        
        self.batteryLevelChar = _batteryLevelChar
        self.batteryLevel     = 0
        
        device.setNotify(batteryLevelChar, enabled: true)
        device.readValue(batteryLevelChar)
    }
    
    // 値更新通知
    func didUpdateValue(characteristic: CBCharacteristic, data: [UInt8])
    {
        switch characteristic.UUID {
        case SenStickUUIDs.BatteryLevelCharUUID:
            batteryLevel = Int(data[0])

        default:
            debugPrint("\(#function): unexpected character: \(characteristic).")
            break
        }
    }
}
