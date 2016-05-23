//
//  SenStickSensorService.swift
//  SenStickSDK
//
//  Created by AkihiroUehara on 2016/04/19.
//  Copyright © 2016年 AkihiroUehara. All rights reserved.
//

import Foundation
import CoreBluetooth

public protocol SenStickSensorServiceDelegate : class
{
    func didUpdateSetting(sender:AnyObject)
    func didUpdateRealTimeData(sender: AnyObject)
    func didUpdateMetaData(sender: AnyObject)
    func didUpdateLogData(sender: AnyObject)
}

// センサー各種のベースタイプ, Tはセンサデータ独自のデータ型, Sはサンプリングの型
public class SenStickSensorService<T: SensorDataPackableType, S: RawRepresentable where S.RawValue == UInt16, T.RangeType == S> 
{
    // Variables
    unowned let device: SenStickDevice
    weak var delegate: SenStickSensorServiceDelegate?
    
    var sensorSettingChar:       CBCharacteristic
    var sensorRealTimeDataChar:  CBCharacteristic
    var sensorLogIDChar:         CBCharacteristic
    var sensorLogMetaDataChar:   CBCharacteristic
    var sensorLogDataChar:       CBCharacteristic
    
    // Properties
    public private(set) var settingData:  SensorSettingData<S>?
    public private(set) var realtimeData: [T]?
    public private(set) var logID:       SensorLogID?
    public private(set) var logMetaData: SensorLogMetaData<S>?
    public private(set) var logData:     [T]?
    
    // イニシャライザ
    init?(device:SenStickDevice, sensorType:SenStickSensorType)
    {
        self.device = device
        
        // UUID
        let serviceUUID                  = SenStickUUIDs.createSenstickUUID(SenStickUUIDs.sensorServiceBaseUUID           | UInt16(sensorType.rawValue))
        let sensorSettingCharUUID        = SenStickUUIDs.createSenstickUUID(SenStickUUIDs.sensorSettingCharBaseUUID       | UInt16(sensorType.rawValue))
        let sensorRealTimeDataCharUUID   = SenStickUUIDs.createSenstickUUID(SenStickUUIDs.sensorRealTimeDataCharBaseUUID  | UInt16(sensorType.rawValue))
        let sensorLogIDCharUUID          = SenStickUUIDs.createSenstickUUID(SenStickUUIDs.sensorLogIDCharBaseUUID         | UInt16(sensorType.rawValue))
        let sensorLogMetaDataCharUUID    = SenStickUUIDs.createSenstickUUID(SenStickUUIDs.sensorLogMetaDataCharBaseUUID   | UInt16(sensorType.rawValue))
        let sensorLogDataCharUUID        = SenStickUUIDs.createSenstickUUID(SenStickUUIDs.sensorLogDataCharBaseUUID | UInt16(sensorType.rawValue))

        // サービス
        guard let service = device.findService(serviceUUID) else { return nil }
        // キャラクタリスティクスの取得
        guard let _sensorSettingChar       = device.findCharacteristic(service, uuid: sensorSettingCharUUID) else { return nil }
        guard let _sensorRealTimeDataChar  = device.findCharacteristic(service, uuid: sensorRealTimeDataCharUUID) else { return nil }
        guard let _sensorLogIDChar         = device.findCharacteristic(service, uuid: sensorLogIDCharUUID) else { return nil }
        guard let _sensorLogMetaDataChar   = device.findCharacteristic(service, uuid: sensorLogMetaDataCharUUID) else { return nil }
        guard let _sensorLogDataChar = device.findCharacteristic(service, uuid: sensorLogDataCharUUID) else { return nil }
        
        self.sensorSettingChar       = _sensorSettingChar
        self.sensorRealTimeDataChar  = _sensorRealTimeDataChar
        self.sensorLogIDChar         = _sensorLogIDChar
        self.sensorLogMetaDataChar   = _sensorLogMetaDataChar
        self.sensorLogDataChar = _sensorLogDataChar
        
        //初期値読み出し。
        device.readValue(self.sensorSettingChar)
        
        // Notifyを有効に
        device.setNotify(self.sensorRealTimeDataChar, enabled: true)
        device.setNotify(self.sensorLogDataChar, enabled: true)
    }
    
    // internal methods
    
    // 値更新通知
    func didUpdateValue(characteristic: CBCharacteristic, data: [UInt8])
    {
        switch characteristic.UUID {
        case sensorSettingChar.UUID:
            guard let settingData = SensorSettingData<S>.unpack(data) else {
                // FIXME 不正なデータ処理
                assert(false, #function)
                break
            }
            self.settingData = settingData
            delegate?.didUpdateSetting(self)
            
        case sensorRealTimeDataChar.UUID:
            guard let setting = settingData else {
                assert(false, #function)
                break
            }
            guard let newdata = T.unpack(setting.range, value: data) else {
                assert(false, #function)
                break
            }
            self.realtimeData = newdata
            delegate?.didUpdateRealTimeData(self)
debugPrint("\(self.realtimeData)")
            
        case sensorLogIDChar.UUID:
            guard let logID = SensorLogID.unpack(data) else {
                assert(false, #function)
                break
            }
            self.logID = logID

        case sensorLogMetaDataChar.UUID:
            guard let metadata = SensorLogMetaData<S>.unpack(data) else {
                assert(false, #function)
                break
            }
            self.logMetaData = metadata
            delegate?.didUpdateMetaData(self)
            
        case sensorLogDataChar.UUID:
            guard let metadata = self.logMetaData else {
                assert(false, #function)
                break
            }
            guard let logData = T.unpack(metadata.range, value: data) else {
                assert(false, #function)
                break
            }
            self.logData = logData
            delegate?.didUpdateLogData(self)
            
        default:
            assert(false, "\(#function), unexpected cahatacter: \(characteristic)")
            break
        }
    }

    // Public methods

    // セッテイングを書き込みます
    public func writeSetting(setting: SensorSettingData<S>)
    {
        let data = setting.pack()
        device.writeValue(sensorSettingChar, value: data)
    }

    // ログIDを書き込みます。自動的にメタデータが更新されます。
    public func writeLogID(logID: SensorLogID)
    {
        let data = logID.pack()
        device.writeValue(sensorLogIDChar, value: data)
        updateLogMetaData()
    }
    
    // メタデータの更新
    public func updateLogMetaData()
    {
        device.readValue(sensorLogMetaDataChar)
    }
}
