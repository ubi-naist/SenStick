//
//  SenstickServicesFacade.swift
//  SenStickSDK
//
//  Created by AkihiroUehara on 2016/03/17.
//  Copyright © 2016年 AkihiroUehara. All rights reserved.
//

import Foundation
import CoreBluetooth

// CoreBluetoothのサービスおよびキャラクタリスティクスの発見は、ややこしいく、かつ、接続時の最初の1回だけの処理なので、ファクトリクラスにまとめます。
class SenStickServicesFacade: NSObject, CBPeripheralDelegate {

}
/*

    
    
    
    
    
    
    
    
    
    static func 
    
    
    
    let peripheral: CBPeripheral
    let callback: (device:SenStickDevice?)->Void
    var isSenStickServiceFound, isSensorServiceFound, isLogReadOutServiceFound: Bool
    
    init(peripheral:CBPeripheral, callback: (device:SenStickDevice?)->Void)
    {
        self.peripheral     = peripheral
        peripheral.delegate = self
        self.callback       = callback
        
        super.init()
        
        // サービスを発見
        self.peripheral.discoverServices([SenStickUUIDs.senstickServiceUUID, SenStickUUIDs.sensorServiceUUID, SenStickUUIDs.logReadOutServiceUUID])
    }
    
    // MARK: CBPeripheralDelegate
    func peripheral(peripheral: CBPeripheral, didDiscoverServices error: NSError?)
    {
        if let _ = error {
            // サービス発見できず。
            self.callback(device: nil)
        } else {
            for s in peripheral.services! {
                switch( s.UUID ) {
                case SenStickUUIDs.senstickServiceUUID:
                    peripheral.discoverCharacteristics([SenStickUUIDs.senstickControlPointUUID], forService: s)

                case SenStickUUIDs.sensorServiceUUID:
                    peripheral.discoverCharacteristics(
                        [SenStickUUIDs.sensorSettingUUID,
                        SenStickUUIDs.sensorDataUUID,
                        SenStickUUIDs.sensorDateTimeUUID,
                        SenStickUUIDs.sensorMetaDataAbstractTextUUID],
                        forService: s)

                case SenStickUUIDs.logReadOutServiceUUID:
                    peripheral.discoverCharacteristics(
                        [SenStickUUIDs.sensorSettingUUID,
                            SenStickUUIDs.sensorDataUUID,
                            SenStickUUIDs.sensorDateTimeUUID,
                            SenStickUUIDs.sensorMetaDataAbstractTextUUID],
                        forService: s)

                }
            }
        }
    }

    func peripheral(peripheral: CBPeripheral, didDiscoverCharacteristicsForService service: CBService, error: NSError?)
    {
        if let _ = error {
            // キャラクタリスティクス発見できず
            self.callback(device: nil)
        } else {
            switch(service.UUID) {
            case SenStickUUIDs.senstickServiceUUID:
                isSenStickServiceFound = true
            case SenStickUUIDs.sensorServiceUUID:
                isSensorServiceFound = true
            case SenStickUUIDs.logReadOutServiceUUID:
                isLogReadOutServiceFound = true
            }
        }
        // 3つ揃ったら、インスタンスを構築
        if isSenStickServiceFound && isSensorServiceFound && isLogReadOutServiceFound {
            let device = SenStickDevice.init(peripheral: peripheral)
            self.callback(device: device)
        }
    }
}
 
 */