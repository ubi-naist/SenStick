//
//  SenStickDevice.swift
//  SenStickSDK
//
//  Created by AkihiroUehara on 2016/03/15.
//  Copyright © 2016年 AkihiroUehara. All rights reserved.
//

import Foundation
import CoreBluetooth

public class SenStickDevice : NSObject, CBPeripheralDelegate
{
    // MARK: variables
    unowned let manager: CBCentralManager
    let peripheral: CBPeripheral
    
    // MARK: Properties
    public private(set) var state : CBPeripheralState
    
    public private(set) var controlService:   SenStickControlService?
    public private(set) var metaDataService : SenStickMetaDataService?
    public private(set) var accelerationSensorService: AccelerationSensorService?
    
    // MARK: initializer
    init(manager: CBCentralManager, peripheral:CBPeripheral)
    {
        self.manager    = manager
        self.peripheral = peripheral
        self.state      = peripheral.state
        
        super.init()
        
        self.peripheral.delegate = self
        addObserver(self.peripheral, forKeyPath: "state", options: [.New], context: nil)
    }
    
    deinit
    {
        removeObserver(self.peripheral, forKeyPath: "state")
    }
    
    // KVO
    override public func observeValueForKeyPath(keyPath: String?, ofObject object: AnyObject?, change: [String : AnyObject]?, context: UnsafeMutablePointer<Void>) {
        guard let key = keyPath else { return }
        switch key {
        case "state":
            state = peripheral.state
            switch peripheral.state {
            case .Disconnected:
                break
            case .Connected:
                findSensticServices()
                break
            default:
                break
            }
        default:
            assert(false, "\(#function): unexpected KVO path, \(key).")
            break
        }
    }
    
    // Private methods
    func findSensticServices()
    {
        // サービスの発見
        let serviceUUIDs = Array(SenStickUUIDs.SenStickServiceUUIDs.keys)
        peripheral.discoverServices(serviceUUIDs)
    }
    
    // MARK: Public methods
    public func connect()
    {
        if peripheral.state == .Disconnected || peripheral.state == .Disconnecting {
            manager.connectPeripheral(peripheral, options:nil)
        }
    }
    
    // MARK: CBPeripheralDelegate
    
    // サービスの発見、次にキャラクタリスティクスの検索をする
    public func peripheral(peripheral: CBPeripheral, didDiscoverServices error: NSError?)
    {
        // エラーなきことを確認
        if error != nil {
            debugPrint("Unexpected error in \(#function), \(error).")
            return
        }
        
        // FIXME サービスが一部なかった場合などは、どのような処理になる? すべてのサービスが発見できなければエラー?
        for service in peripheral.services! {
            let characteristicsUUIDs = SenStickUUIDs.SenStickServiceUUIDs[service.UUID]
            peripheral.discoverCharacteristics(characteristicsUUIDs, forService: service)
        }
    }
    
    // サービスのインスタンスを作る
    public func peripheral(peripheral: CBPeripheral, didDiscoverCharacteristicsForService service: CBService, error: NSError?)
    {
        // エラーなきことを確認
        if error != nil {
            debugPrint("Unexpected error in \(#function), \(error).")
            return
        }
        
        // FIXME キャラクタリスティクスが発見できないサービスがあった場合、コールバックに通知が来ない。タイムアウトなどで処理する?
        
        // すべてのサービスのキャラクタリスティクスが発見できれば、インスタンスを生成
        switch service.UUID {
        case SenStickUUIDs.ControlServiceUUID:
            self.controlService = SenStickControlService(device: self)
        case SenStickUUIDs.MetaDataServiceUUID:
            self.metaDataService = SenStickMetaDataService(device: self)
        case SenStickUUIDs.accelerationSensorServiceUUID:
            self.accelerationSensorService = AccelerationSensorService(device: self)
        default:
            debugPrint("\(#function):unexpected service is found, \(service)" )
            break
        }
    }
    
    public func peripheral(peripheral: CBPeripheral, didUpdateValueForCharacteristic characteristic: CBCharacteristic, error: NSError?)
    {
        // キャラクタリスティクスから[UInt8]を取り出す。なければreturn。
        guard let characteristics_nsdata = characteristic.value else { return }
        
        var data = [UInt8](count: characteristics_nsdata.length, repeatedValue: 0)
        characteristics_nsdata.getBytes(&data, length: data.count)
        
        switch characteristic.service.UUID {
        case SenStickUUIDs.ControlServiceUUID:
            self.controlService?.didUpdateValue(characteristic, data: data)
        case SenStickUUIDs.MetaDataServiceUUID:
            self.metaDataService?.didUpdateValue(characteristic, data: data)
        case SenStickUUIDs.accelerationSensorServiceUUID:
            self.accelerationSensorService?.didUpdateValue(characteristic, data: data)
        default:
            break
        }
    }
    
    //    optional public func peripheralDidUpdateName(peripheral: CBPeripheral)
    //    optional public func peripheral(peripheral: CBPeripheral, didModifyServices invalidatedServices: [CBService])
    //    optional public func peripheral(peripheral: CBPeripheral, didReadRSSI RSSI: NSNumber, error: NSError?)
    //    optional public func peripheral(peripheral: CBPeripheral, didDiscoverServices error: NSError?)
    //    optional public func peripheral(peripheral: CBPeripheral, didDiscoverIncludedServicesForService service: CBService, error: NSError?)
    //    optional public func peripheral(peripheral: CBPeripheral, didDiscoverCharacteristicsForService service: CBService, error: NSError?)
    //    optional public func peripheral(peripheral: CBPeripheral, didUpdateValueForCharacteristic characteristic: CBCharacteristic, error: NSError?)
    //    optional public func peripheral(peripheral: CBPeripheral, didWriteValueForCharacteristic characteristic: CBCharacteristic, error: NSError?)
    //    optional public func peripheral(peripheral: CBPeripheral, didUpdateNotificationStateForCharacteristic characteristic: CBCharacteristic, error: NSError?)
    //    optional public func peripheral(peripheral: CBPeripheral, didDiscoverDescriptorsForCharacteristic characteristic: CBCharacteristic, error: NSError?)
    //    optional public func peripheral(peripheral: CBPeripheral, didUpdateValueForDescriptor descriptor: CBDescriptor, error: NSError?)
    //    optional public func peripheral(peripheral: CBPeripheral, didWriteValueForDescriptor descriptor: CBDescriptor, error: NSError?)
}

// サービスなど発見のヘルパーメソッド
extension SenStickDevice {
    // ペリフェラルから指定したUUIDのCBServiceを取得します
    func findService(uuid: CBUUID) -> CBService?
    {
        return (self.peripheral.services?.filter{$0.UUID == uuid}.first)
    }
    // 指定したUUIDのCharacteristicsを取得します
    func findCharacteristic(service: CBService, uuid: CBUUID) -> CBCharacteristic?
    {
        return (service.characteristics?.filter{$0.UUID == uuid}.first)
    }
}

// SenstickServiceが呼び出すメソッド
extension SenStickDevice {
    // Notificationを設定します。コールバックはdidUpdateValueの都度呼びだされます。初期値を読みだすために、enabeledがtrueなら初回の読み出しが実行されます。
    internal func setNotify(characteristic: CBCharacteristic, enabled: Bool)
    {
        peripheral.setNotifyValue(enabled, forCharacteristic: characteristic)
        if enabled {
            peripheral.readValueForCharacteristic(characteristic)
        }
    }
    // 値読み出しを要求します
    internal func readValue(characteristic: CBCharacteristic)
    {
        peripheral.readValueForCharacteristic(characteristic)
    }
    // 値の書き込み
    internal func writeValue(characteristic: CBCharacteristic, value: [UInt8])
    {
        peripheral.writeValue( NSData(bytes: value, length: value.count), forCharacteristic: characteristic, type: .WithoutResponse)
    }
}
