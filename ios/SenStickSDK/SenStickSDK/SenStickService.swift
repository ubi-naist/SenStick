//
//  SenStickService.swift
//  SenStickSDK
//
//  Created by AkihiroUehara on 2016/03/29.
//  Copyright © 2016年 AkihiroUehara. All rights reserved.
//

import Foundation
import CoreBluetooth

protocol SenStickService
{
    static var ServiceUUID : CBUUID {get}
    static var CharacteristicsUUIDs : [CBUUID] {get}

    // イニシャライザ
    init?(device: SenStickDevice)
    
    // 値が更新される都度CBPeripheralDelegateから呼びだされます
    func didUpdateValue(device: SenStickDevice, characteristic: CBCharacteristic, data: NSData)
}
