//
//  controlSerivce.swift
//  SenStickSDK
//
//  Created by AkihiroUehara on 2016/03/17.
//  Copyright © 2016年 AkihiroUehara. All rights reserved.
//

import Foundation
import CoreBluetooth

extension senStickService : SenStickDevice
{
    
    /*
    var service: CBService?
    var controlPoint: CBCharacteristic?

    private(set) var isSensing: Bool
    private(set) var isLogging: Bool
    
    // Private method
    func parseControlPoint(c:CBCharacteristic)
    {
        if let value = c.value {
            // バイト長は5バイトであるべき
            if value.length != 5 {
                return
            }
            // 動作状態をフラグに転換
            
        }
    }
    
    // MARK: serviceProtocol
    var serviceUUID: CBUUID {return SenStickUUIDs.senstickServiceUUID }
    var characteristicsUUDs:[CBUUID] {return [SenStickUUIDs.senstickControlPointUUID]}
    
    func findService(services: [CBService]) -> [CBUUID]
    {
        if let s = services.filter( {$0.UUID == self.serviceUUID} ).first {
            service = s
            return characteristicsUUDs
        } else {
            return []
        }
    }
    
    func findCharacteristics(characteristics: [CBCharacteristic])
    {
        if let c = characteristics.filter({$0.UUID == SenStickUUIDs.senstickControlPointUUID}).first {
            controlPoint = c
            service?.peripheral.setNotifyValue(true, forCharacteristic: controlPoint!)
        }
    }
    
    func updateValue(characteristic:[CBCharacteristic], data:NSData) -> Void
    {
        if(characteristic == controlPoint) {
            parseControlPoint(characteristic)
        }
    }
*/
}