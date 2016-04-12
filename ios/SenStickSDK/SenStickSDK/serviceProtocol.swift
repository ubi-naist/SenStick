//
//  serviceProtocol.swift
//  SenStickSDK
//
//  Created by AkihiroUehara on 2016/03/17.
//  Copyright © 2016年 AkihiroUehara. All rights reserved.
//

import Foundation
import CoreBluetooth

protocol serviceProtocol
{
    // サービスのUUID
    var serviceUUID: CBUUID {get}
    // キャラクタリスティクスのUUID
    var characteristicsUUDs: Set<CBUUID> {get}
    
    // デバイスが呼び出す発見したサービス配列の処理。
    // 対象サービスが含まれていれば、発見すべきキャラクタリスティクスのUUIDを返す。
    func findService(services:[CBService]) -> [CBUUID]
    func findCharacteristics(characteristic:[CBCharacteristic]) -> Void
    func updateValue(characteristic:[CBCharacteristic], data:NSData) -> Void
}
