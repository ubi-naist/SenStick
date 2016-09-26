//
//  SenStickDevice.swift
//  SenStickSDK
//
//  Created by AkihiroUehara on 2016/03/15.
//  Copyright © 2016年 AkihiroUehara. All rights reserved.
//

import Foundation
import CoreBluetooth

public protocol SenStickDeviceDelegate : class {
    func didServiceFound(_ sender:SenStickDevice)
    func didConnected(_ sender:SenStickDevice)
    func didDisconnected(_ sender:SenStickDevice)
}

open class SenStickDevice : NSObject, CBPeripheralDelegate
{
    // MARK: variables
    open unowned let manager: CBCentralManager
    open let peripheral:      CBPeripheral

    open weak var delegate: SenStickDeviceDelegate?
    
    // MARK: Properties
    open fileprivate(set) var isConnected: Bool
    
    open var name: String
    open fileprivate(set) var identifier: UUID

    open fileprivate(set) var deviceInformationService:   DeviceInformationService?
    open fileprivate(set) var batteryLevelService:        BatteryService?
    
    open fileprivate(set) var controlService:             SenStickControlService?
    open fileprivate(set) var metaDataService:            SenStickMetaDataService?
    open fileprivate(set) var accelerationSensorService:  AccelerationSensorService?
    open fileprivate(set) var gyroSensorService:          GyroSensorService?
    open fileprivate(set) var magneticFieldSensorService: MagneticFieldSensorService?
    open fileprivate(set) var brightnessSensorService:    BrightnessSensorService?
    open fileprivate(set) var uvSensorService:            UVSensorService?
    open fileprivate(set) var humiditySensorService:      HumiditySensorService?
    open fileprivate(set) var pressureSensorService:      PressureSensorService?
    
    // MARK: initializer
    init(manager: CBCentralManager, peripheral:CBPeripheral, name: String?)
    {
        self.manager    = manager
        self.peripheral = peripheral
        self.isConnected = false
        self.name        = name ?? peripheral.name ?? ""
        self.identifier  = peripheral.identifier
        
        super.init()

        self.peripheral.delegate = self
        if self.peripheral.state == .connected {
            self.isConnected = true
            findSensticServices()
        }
    }
   
    // Internal , device managerが呼び出します
    internal func onConnected()
    {
        self.isConnected = true
        DispatchQueue.main.async(execute: {
            self.delegate?.didConnected(self)
        })
        
        findSensticServices()
    }
    
    internal func onDisConnected()
    {
        self.isConnected = false
        
        self.deviceInformationService   = nil
        self.batteryLevelService        = nil
        
        self.controlService             = nil
        self.metaDataService            = nil
        self.accelerationSensorService  = nil
        self.magneticFieldSensorService = nil
        self.gyroSensorService          = nil
        self.brightnessSensorService    = nil
        self.uvSensorService            = nil
        self.humiditySensorService      = nil
        self.pressureSensorService      = nil

        DispatchQueue.main.async(execute: {
            self.delegate?.didDisconnected(self)
        })
    }
    
    // Private methods
    func findSensticServices()
    {
        // サービスの発見
        let serviceUUIDs = Array(SenStickUUIDs.SenStickServiceUUIDs.keys)
        peripheral.discoverServices(serviceUUIDs)
    }
    
    // MARK: Public methods
    open func connect()
    {
        if peripheral.state == .disconnected || peripheral.state == .disconnecting {
            manager.connect(peripheral, options:nil)
        }
    }
    
    open func cancelConnection()
    {
        manager.cancelPeripheralConnection(peripheral)
    }
    
    // MARK: CBPeripheralDelegate
    
    // サービスの発見、次にキャラクタリスティクスの検索をする
    open func peripheral(_ peripheral: CBPeripheral, didDiscoverServices error: Error?)
    {
        // エラーなきことを確認
        if error != nil {
            debugPrint("Unexpected error in \(#function), \(error).")
            return
        }
        
        // FIXME サービスが一部なかった場合などは、どのような処理になる? すべてのサービスが発見できなければエラー?
        for service in peripheral.services! {
            let characteristicsUUIDs = SenStickUUIDs.SenStickServiceUUIDs[service.uuid]
            peripheral.discoverCharacteristics(characteristicsUUIDs, for: service)
        }
    }
    
    // サービスのインスタンスを作る
    open func peripheral(_ peripheral: CBPeripheral, didDiscoverCharacteristicsFor service: CBService, error: Error?)
    {
        // エラーなきことを確認
        if error != nil {
            debugPrint("Unexpected error in \(#function), \(error).")
            return
        }
        
        // FIXME キャラクタリスティクスが発見できないサービスがあった場合、コールバックに通知が来ない。タイムアウトなどで処理する?
//   debugPrint("\(#function), \(service.UUID).")
        // すべてのサービスのキャラクタリスティクスが発見できれば、インスタンスを生成
        switch service.uuid {
        case SenStickUUIDs.DeviceInformationServiceUUID:
            self.deviceInformationService = DeviceInformationService(device:self)
        case SenStickUUIDs.BatteryServiceUUID:
            self.batteryLevelService = BatteryService(device:self)
        case SenStickUUIDs.ControlServiceUUID:
            self.controlService = SenStickControlService(device: self)
        case SenStickUUIDs.MetaDataServiceUUID:
            self.metaDataService = SenStickMetaDataService(device: self)
        case SenStickUUIDs.accelerationSensorServiceUUID:
            self.accelerationSensorService = AccelerationSensorService(device: self)
        case SenStickUUIDs.gyroSensorServiceUUID:
            self.gyroSensorService = GyroSensorService(device: self)
        case SenStickUUIDs.magneticFieldSensorServiceUUID:
            self.magneticFieldSensorService = MagneticFieldSensorService(device: self)
        case SenStickUUIDs.brightnessSensorServiceUUID:
            self.brightnessSensorService = BrightnessSensorService(device: self)
        case SenStickUUIDs.uvSensorServiceUUID:
            self.uvSensorService = UVSensorService(device: self)
        case SenStickUUIDs.humiditySensorServiceUUID:
            self.humiditySensorService = HumiditySensorService(device: self)
        case SenStickUUIDs.pressureSensorServiceUUID:
            self.pressureSensorService = PressureSensorService(device: self)

        default:
            debugPrint("\(#function):unexpected service is found, \(service)" )
            break
        }
        
        DispatchQueue.main.async(execute: {
            self.delegate?.didServiceFound(self)
        })        
    }
    
    open func peripheral(_ peripheral: CBPeripheral, didUpdateValueFor characteristic: CBCharacteristic, error: Error?)
    {
        if error != nil {
            debugPrint("didUpdate: \(characteristic.uuid) \(error)")
            return
        }
        
        // キャラクタリスティクスから[UInt8]を取り出す。なければreturn。
        guard let characteristics_nsdata = characteristic.value else { return }
        
        var data = [UInt8](repeating: 0, count: characteristics_nsdata.count)
        (characteristics_nsdata as NSData).getBytes(&data, length: data.count)

//debugPrint("didUpdate: \(characteristic.uuid) \(data)")
        
        switch characteristic.service.uuid {
        case SenStickUUIDs.DeviceInformationServiceUUID:
            self.deviceInformationService?.didUpdateValue(characteristic, data: data)
        case SenStickUUIDs.BatteryServiceUUID:
            self.batteryLevelService?.didUpdateValue(characteristic, data: data)
        case SenStickUUIDs.ControlServiceUUID:
            self.controlService?.didUpdateValue(characteristic, data: data)
        case SenStickUUIDs.MetaDataServiceUUID:
            self.metaDataService?.didUpdateValue(characteristic, data: data)
        case SenStickUUIDs.accelerationSensorServiceUUID:
            self.accelerationSensorService?.didUpdateValue(characteristic, data: data)
        case SenStickUUIDs.gyroSensorServiceUUID:
            self.gyroSensorService?.didUpdateValue(characteristic, data: data)
        case SenStickUUIDs.magneticFieldSensorServiceUUID:
            self.magneticFieldSensorService?.didUpdateValue(characteristic, data: data)
        case SenStickUUIDs.brightnessSensorServiceUUID:
            self.brightnessSensorService?.didUpdateValue(characteristic, data: data)
        case SenStickUUIDs.uvSensorServiceUUID:
            self.uvSensorService?.didUpdateValue(characteristic, data: data)
        case SenStickUUIDs.humiditySensorServiceUUID:
            self.humiditySensorService?.didUpdateValue(characteristic, data: data)
        case SenStickUUIDs.pressureSensorServiceUUID:
            self.pressureSensorService?.didUpdateValue(characteristic, data: data)
            
        default:
            break
        }
    }
    
    open func peripheralDidUpdateName(_ peripheral: CBPeripheral)
    {
        self.name = peripheral.name ?? "(unknown)"
    }
    
    //    optional public func peripheral(peripheral: CBPeripheral, didModifyServices invalidatedServices: [CBService])
    //    optional public func peripheral(peripheral: CBPeripheral, didReadRSSI RSSI: NSNumber, error: NSError?)
    //    optional public func peripheral(peripheral: CBPeripheral, didDiscoverIncludedServicesForService service: CBService, error: NSError?)
    //    optional public func peripheral(peripheral: CBPeripheral, didWriteValueForCharacteristic characteristic: CBCharacteristic, error: NSError?)
    //    optional public func peripheral(peripheral: CBPeripheral, didUpdateNotificationStateForCharacteristic characteristic: CBCharacteristic, error: NSError?)
    //    optional public func peripheral(peripheral: CBPeripheral, didDiscoverDescriptorsForCharacteristic characteristic: CBCharacteristic, error: NSError?)
    //    optional public func peripheral(peripheral: CBPeripheral, didUpdateValueForDescriptor descriptor: CBDescriptor, error: NSError?)
    //    optional public func peripheral(peripheral: CBPeripheral, didWriteValueForDescriptor descriptor: CBDescriptor, error: NSError?)
}

// サービスなど発見のヘルパーメソッド
extension SenStickDevice {
    // ペリフェラルから指定したUUIDのCBServiceを取得します
    func findService(_ uuid: CBUUID) -> CBService?
    {
        return (self.peripheral.services?.filter{$0.uuid == uuid}.first)
    }
    // 指定したUUIDのCharacteristicsを取得します
    func findCharacteristic(_ service: CBService, uuid: CBUUID) -> CBCharacteristic?
    {
        return (service.characteristics?.filter{$0.uuid == uuid}.first)
    }
}

// SenstickServiceが呼び出すメソッド
extension SenStickDevice {
    // Notificationを設定します。コールバックはdidUpdateValueの都度呼びだされます。初期値を読みだすために、enabeledがtrueなら初回の読み出しが実行されます。
    internal func setNotify(_ characteristic: CBCharacteristic, enabled: Bool)
    {
        peripheral.setNotifyValue(enabled, for: characteristic)
    }
    // 値読み出しを要求します
    internal func readValue(_ characteristic: CBCharacteristic)
    {
        peripheral.readValue(for: characteristic)
    }
    // 値の書き込み
    internal func writeValue(_ characteristic: CBCharacteristic, value: [UInt8])
    {
        let data = Data(bytes: UnsafePointer<UInt8>(value), count: value.count)
//        peripheral.writeValue( data, forCharacteristic: characteristic, type: .WithoutResponse)
        peripheral.writeValue( data, for: characteristic, type: .withResponse)
//debugPrint("writeValue: \(characteristic.uuid) \(value)")
    }
}
