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
    public private(set)  var state : CBPeripheralState
    
    // MARK: initializer
    init(manager: CBCentralManager, peripheral:CBPeripheral)
    {
        self.manager    = manager
        self.peripheral = peripheral
        self.peripheral.delegate = self

        state = peripheral.state
        
        addObserver(self.peripheral, forKeyPath: "state", options: [.New], context: nil)
    }
    
    deinit {
        removeObserver(self.peripheral, forKeyPath: "state")
    }
    
    // KVO
    override public func observeValueForKeyPath(keyPath: String?, ofObject object: AnyObject?, change: [String : AnyObject]?, context: UnsafeMutablePointer<Void>) {
        guard let key = keyPath else { return }
        switch key as String {
            case "state":
                state = peripheral.state
        }
    }
    
    // MARK: Public methods
    func connect() -> Void
    {
        if peripheral.state == .Disconnected || peripheral.state == .Disconnecting {
            manager.connectPeripheral(peripheral, options:nil)
        }
    }
    
    // MARK: CBPeripheralDelegate
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
    static func findService(device: SenStickDevice, uuid: CBUUID) -> CBService?
    {
        return device.peripheral.services?.filter{$0.UUID == uuid}.first
    }
    // 指定したUUIDのCharacteristicsを取得します
    static func findCharacteristic(device: SenStickDevice, uuid: CBUUID) -> CBCharacteristic?
    {
        return findService(device, uuid: uuid).characteristics?.filter{$0.UUID == uuid}.first
    }
}

// SenstickServiceが呼び出すメソッド
extension SenStickDevice {
    // Notificationを設定します。コールバックはdidUpdateValueの都度呼びだされます
    internal func setNotify(characteristic: CBCharacteristic, enabled: Bool)
    {
        peripheral.setNotifyValue(enabled, forCharacteristic: characteristic)
    }
    // 値を要求します
    internal func readValue(characteristic: CBCharacteristic)
    {
        peripheral.readValueForCharacteristic(characteristic)
    }
}

/*
 
 
 
 
 
 var characteristics:[CBUUID : CBCharacteristic] = [CBUUID : CBCharacteristic]()
 
 // MARK: Initializer
 init(manager: SenStickDeviceManager, peripheral:CBPeripheral)
 {
 self.peripheral = peripheral
 self.peripheral.delegate = self
 
 // キャラクタリスティクスを辞書に保存
 for service in peripheral.services! {
 for c in service.characteristics! {
 characteristics[c.UUID] = c
 }
 }
 }
 
 // MARK: Senstick service
 
 public private(set) var isSensing: Bool
 public private(set) var isLogging: Bool
 
 public func startSensing(shouldStart:Bool)
 {
 // バッファを組み立てる
 var buffer = Array<UInt8>(count: 5, repeatedValue: 0)
 buffer[0]  = shouldStart ? 1 : 0
 buffer[1]  = isLogging   ? 1 : 0
 // 書き込む
 let data = NSData.init(bytes: buffer, length: buffer.count)
 peripheral.writeValue(data, forCharacteristic: characteristics[SenStickUUIDs.senstickControlPointUUID]!, type: .WithoutResponse)
 }
 
 public func startLogging(shouldStart:Bool)
 {
 // バッファを組み立てる
 var buffer = Array<UInt8>(count: 5, repeatedValue: 0)
 buffer[0]  = 1
 buffer[1]  = shouldStart ? 1 : 0
 // 書き込む
 let data = NSData.init(bytes: buffer, length: buffer.count)
 peripheral.writeValue(data, forCharacteristic: characteristics[SenStickUUIDs.senstickControlPointUUID]!, type: .WithoutResponse)
 }
 
 // デバイスをスリープ状態に
 public func deviceSleep()
 {
 // バッファを組み立てる
 var buffer = Array<UInt8>(count: 5, repeatedValue: 0)
 buffer[2] = 1
 // 書き込む
 let data = NSData.init(bytes: buffer, length: buffer.count)
 peripheral.writeValue(data, forCharacteristic: characteristics[SenStickUUIDs.senstickControlPointUUID]!, type: .WithoutResponse)
 }
 
 // ストレージのフォーマット
 public func formatStroage()
 {
 // バッファを組み立てる
 var buffer = Array<UInt8>(count: 5, repeatedValue: 0)
 buffer[3] = 1
 // 書き込む
 let data = NSData.init(bytes: buffer, length: buffer.count)
 peripheral.writeValue(data, forCharacteristic: characteristics[SenStickUUIDs.senstickControlPointUUID]!, type: .WithoutResponse)
 }
 
 // ファームウェア更新モードに入る
 public func startDFU()
 {
 // バッファを組み立てる
 var buffer = Array<UInt8>(count: 5, repeatedValue: 0)
 buffer[4] = 1
 // 書き込む
 let data = NSData.init(bytes: buffer, length: buffer.count)
 peripheral.writeValue(data, forCharacteristic: characteristics[SenStickUUIDs.senstickControlPointUUID]!, type: .WithoutResponse)
 }
 
 // senstickControlPointUUID, 値更新の解釈
 func parseControlPoint(c:CBCharacteristic)
 {
 if let value = c.value {
 // バイト長は5バイトであるべき
 if value.length != 5 {
 return
 }
 // 値を取得
 var buffer = Array<UInt8>(count: value.length, repeatedValue: 0)
 value.getBytes(&buffer, length: value.length)
 // 動作状態をフラグに転換
 isSensing = (buffer[0] == 1)
 isLogging = (buffer[1] == 2)
 }
 }
 
 // MARK: sensor service (streaming data)
 
 
 
 
 /*
 
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
 
 
 
 // MARK: Public methods
 
 }
 
 
 
 
 var characteristics:[CBUUID : CBCharacteristic] = [CBUUID : CBCharacteristic]()
 
 // MARK: Initializer
 init(manager: SenStickDeviceManager, peripheral:CBPeripheral)
 {
 self.peripheral = peripheral
 self.peripheral.delegate = self
 
 // キャラクタリスティクスを辞書に保存
 for service in peripheral.services! {
 for c in service.characteristics! {
 characteristics[c.UUID] = c
 }
 }
 }
 
 // MARK: Senstick service
 
 public private(set) var isSensing: Bool
 public private(set) var isLogging: Bool
 
 public func startSensing(shouldStart:Bool)
 {
 // バッファを組み立てる
 var buffer = Array<UInt8>(count: 5, repeatedValue: 0)
 buffer[0]  = shouldStart ? 1 : 0
 buffer[1]  = isLogging   ? 1 : 0
 // 書き込む
 let data = NSData.init(bytes: buffer, length: buffer.count)
 peripheral.writeValue(data, forCharacteristic: characteristics[SenStickUUIDs.senstickControlPointUUID]!, type: .WithoutResponse)
 }
 
 public func startLogging(shouldStart:Bool)
 {
 // バッファを組み立てる
 var buffer = Array<UInt8>(count: 5, repeatedValue: 0)
 buffer[0]  = 1
 buffer[1]  = shouldStart ? 1 : 0
 // 書き込む
 let data = NSData.init(bytes: buffer, length: buffer.count)
 peripheral.writeValue(data, forCharacteristic: characteristics[SenStickUUIDs.senstickControlPointUUID]!, type: .WithoutResponse)
 }
 
 // デバイスをスリープ状態に
 public func deviceSleep()
 {
 // バッファを組み立てる
 var buffer = Array<UInt8>(count: 5, repeatedValue: 0)
 buffer[2] = 1
 // 書き込む
 let data = NSData.init(bytes: buffer, length: buffer.count)
 peripheral.writeValue(data, forCharacteristic: characteristics[SenStickUUIDs.senstickControlPointUUID]!, type: .WithoutResponse)
 }
 
 // ストレージのフォーマット
 public func formatStroage()
 {
 // バッファを組み立てる
 var buffer = Array<UInt8>(count: 5, repeatedValue: 0)
 buffer[3] = 1
 // 書き込む
 let data = NSData.init(bytes: buffer, length: buffer.count)
 peripheral.writeValue(data, forCharacteristic: characteristics[SenStickUUIDs.senstickControlPointUUID]!, type: .WithoutResponse)
 }
 
 // ファームウェア更新モードに入る
 public func startDFU()
 {
 // バッファを組み立てる
 var buffer = Array<UInt8>(count: 5, repeatedValue: 0)
 buffer[4] = 1
 // 書き込む
 let data = NSData.init(bytes: buffer, length: buffer.count)
 peripheral.writeValue(data, forCharacteristic: characteristics[SenStickUUIDs.senstickControlPointUUID]!, type: .WithoutResponse)
 }
 
 // senstickControlPointUUID, 値更新の解釈
 func parseControlPoint(c:CBCharacteristic)
 {
 if let value = c.value {
 // バイト長は5バイトであるべき
 if value.length != 5 {
 return
 }
 // 値を取得
 var buffer = Array<UInt8>(count: value.length, repeatedValue: 0)
 value.getBytes(&buffer, length: value.length)
 // 動作状態をフラグに転換
 isSensing = (buffer[0] == 1)
 isLogging = (buffer[1] == 2)
 }
 }
 
 // MARK: sensor service (streaming data)
 
 
 
 
 /*
 
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
 
 
 
 // MARK: Public methods
 
 // MARK: CBPeripheralDelegate
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
 }*/