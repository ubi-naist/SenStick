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
    func didFinishedLogData(sender: AnyObject)
}

// センサー各種のベースタイプ, Tはセンサデータ独自のデータ型, Sはサンプリングの型
public class SenStickSensorService<T: SensorDataPackableType, S: RawRepresentable where S.RawValue == UInt16, T.RangeType == S> 
{
    let lockQueue = dispatch_queue_create("com.SenStickSensorService.LockQueue", nil)

    // code
    public weak var delegate: SenStickSensorServiceDelegate?
    
    // Variables
    unowned let device: SenStickDevice
    
    var sensorSettingChar:       CBCharacteristic
    var sensorRealTimeDataChar:  CBCharacteristic
    var sensorLogIDChar:         CBCharacteristic
    var sensorLogMetaDataChar:   CBCharacteristic
    var sensorLogDataChar:       CBCharacteristic

    var logData: [T] = []
    
    // Properties
    public private(set) var settingData:  SensorSettingData<S>? {
        didSet {
            dispatch_async(dispatch_get_main_queue(), {
                self.delegate?.didUpdateSetting(self)
            })
        }
    }
    public private(set) var realtimeData: T? {
        didSet {
            dispatch_async(dispatch_get_main_queue(), {
                self.delegate?.didUpdateRealTimeData(self)
            })
        }
    }

    public private(set) var logID: SensorLogID?

    public private(set) var logMetaData: SensorLogMetaData<S>? {
        didSet {
            dispatch_async(dispatch_get_main_queue(), {
                self.delegate?.didUpdateMetaData(self)
            })
        }
    }
/*
    public private(set) var logData: [T] {
        didSet {
            dispatch_async(dispatch_get_main_queue(), {
                self.delegate?.didUpdateLogData(self)
            })
        }
    }
*/
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
    func unpackDataArray(range:S, value: [UInt8]) -> [T]?
    {
        // 空配列
        if value.count == 1 && value[0] == 0 {
            return []
        }
        
        // 不正なデータチェック
        if value.count == 0 {
            return nil
        }

        let count = Int(value[0])
        
        // 0のデータ配列のチェック
        if value.count == 1 && count != 0 {
            return nil
        }

        // データ配列のバイト数
        if (value.count - 1) % count != 0 {
            return nil
        }

        let size  = (value.count - 1) / count
        var array = Array<T>()
        for i in 0..<count {
            // センサデータ1つ分を切り出す
            let startIndex = 1 + size * i
            let endIndex   = startIndex + size
            let unit       = Array(value[startIndex..<endIndex])
            let logunit = T.unpack(settingData!.range, value: unit)!
            array.append( logunit )
        }
        return array
    }

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
            
        case sensorRealTimeDataChar.UUID:
            self.realtimeData = T.unpack(self.settingData!.range, value: data)
//debugPrint("\(self.realtimeData)")
            
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
            
        case sensorLogDataChar.UUID:
            guard let metadata = self.logMetaData else {
//                assert(false, #function)
                break
            }
            if let d = unpackDataArray(metadata.range, value: data) {
                if d.count == 0 {
                    dispatch_async(dispatch_get_main_queue(), {
                        self.delegate?.didFinishedLogData(self)
                    })
                } else {
                    objc_sync_enter(self)
                    self.logData.appendContentsOf(d)
                    objc_sync_exit(self)
                
                    dispatch_async(dispatch_get_main_queue(), {
                        self.delegate?.didUpdateLogData(self)
                    })
                }
            }

//            debugPrint("unpacked : \(d)")
            
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
        device.readValue(sensorSettingChar)
    }

    // ログIDを書き込みます。自動的にメタデータが更新されます。
    public func writeLogID(logID: SensorLogID)
    {
        device.setNotify(self.sensorLogDataChar, enabled: false)

        self.logID = logID
        let data = logID.pack()
        device.writeValue(sensorLogIDChar, value: data)
        updateLogMetaData()
        
        device.setNotify(self.sensorLogDataChar, enabled: true)
    }
    
    // メタデータの更新
    public func updateLogMetaData()
    {
        device.readValue(sensorLogMetaDataChar)
    }
    
    // センサデータを読みだし
    public func readLogData() -> [T]
    {
        var ret: [T]
        
        objc_sync_enter(self)
        ret = self.logData
        self.logData = []
        objc_sync_exit(self)
        
        return ret
    }
}
