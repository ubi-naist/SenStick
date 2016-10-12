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
    func didUpdateSetting(_ sender:AnyObject)
    func didUpdateRealTimeData(_ sender: AnyObject)
    func didUpdateMetaData(_ sender: AnyObject)
    func didUpdateLogData(_ sender: AnyObject)
    func didFinishedLogData(_ sender: AnyObject)
}

// センサー各種のベースタイプ, Tはセンサデータ独自のデータ型, Sはサンプリングの型
open class SenStickSensorService<T: SensorDataPackableType, S: RawRepresentable> where S.RawValue == UInt16, T.RangeType == S
{
    let lockQueue = DispatchQueue(label: "com.SenStickSensorService.LockQueue", attributes: [])

    // code
    open weak var delegate: SenStickSensorServiceDelegate?
    
    // MARK: - Variables
    unowned let device: SenStickDevice
    open let sensorType: SenStickSensorType
    
    var sensorSettingChar:       CBCharacteristic
    var sensorRealTimeDataChar:  CBCharacteristic
    var sensorLogIDChar:         CBCharacteristic
    var sensorLogMetaDataChar:   CBCharacteristic
    var sensorLogDataChar:       CBCharacteristic

    // delegate.didUpdatelogData()の過度な呼び出しを防止するためのフラグ
    var _flag:Bool = false
    var _flagLockObj: NSObject = NSObject()
    var flag: Bool {
        get {
            objc_sync_enter(_flagLockObj)
            let val = _flag
            objc_sync_exit(_flagLockObj)
            return val
        }
        set(newValue) {
            objc_sync_enter(_flagLockObj)
            _flag = newValue
            objc_sync_exit(_flagLockObj)
        }
    }
    var logData: [T] = []

    // MARK: - Properties

    
    open fileprivate(set) var settingData:  SensorSettingData<S>? {
        didSet {
            DispatchQueue.main.async(execute: {
                self.delegate?.didUpdateSetting(self)
            })
        }
    }

    var _realTimeDataFlag:Bool = false
    var _lockObj: NSObject = NSObject()
    var realTimeDataFlag: Bool {
        get {
            objc_sync_enter(_lockObj)
            let val = _realTimeDataFlag
            objc_sync_exit(_lockObj)
            return val
        }
        set(newValue) {
            objc_sync_enter(_lockObj)
            _realTimeDataFlag = newValue
            objc_sync_exit(_lockObj)
        }
    }
    open fileprivate(set) var realtimeData: T? {
        didSet {
            if realTimeDataFlag == false {
                realTimeDataFlag = true
                DispatchQueue.main.async(execute: {
                    self.delegate?.didUpdateRealTimeData(self)
                    self.realTimeDataFlag = false
                })
            }
        }
    }

    open fileprivate(set) var logID: SensorLogID?

    open fileprivate(set) var logMetaData: SensorLogMetaData<S>? {
        didSet {
            DispatchQueue.main.async(execute: {
                self.delegate?.didUpdateMetaData(self)
            })
        }
    }

    // MARK: - イニシャライザ
    init?(device:SenStickDevice, sensorType:SenStickSensorType)
    {
        self.device     = device
        self.sensorType = sensorType
        
        // UUID
        let serviceUUID                  = SenStickUUIDs.createSenstickUUID(SenStickUUIDs.sensorServiceBaseUUID           | UInt16(sensorType.rawValue))
        let sensorSettingCharUUID        = SenStickUUIDs.createSenstickUUID(SenStickUUIDs.sensorSettingCharBaseUUID       | UInt16(sensorType.rawValue))
        let sensorRealTimeDataCharUUID   = SenStickUUIDs.createSenstickUUID(SenStickUUIDs.sensorRealTimeDataCharBaseUUID  | UInt16(sensorType.rawValue))
        let sensorLogIDCharUUID          = SenStickUUIDs.createSenstickUUID(SenStickUUIDs.sensorLogIDCharBaseUUID         | UInt16(sensorType.rawValue))
        let sensorLogMetaDataCharUUID    = SenStickUUIDs.createSenstickUUID(SenStickUUIDs.sensorLogMetaDataCharBaseUUID   | UInt16(sensorType.rawValue))
        let sensorLogDataCharUUID        = SenStickUUIDs.createSenstickUUID(SenStickUUIDs.sensorLogDataCharBaseUUID       | UInt16(sensorType.rawValue))

        // サービス
        guard let service = device.findService(serviceUUID) else { return nil }
        
        // キャラクタリスティクスの取得
        guard let _sensorSettingChar       = device.findCharacteristic(service, uuid: sensorSettingCharUUID) else { return nil }
        guard let _sensorRealTimeDataChar  = device.findCharacteristic(service, uuid: sensorRealTimeDataCharUUID) else { return nil }
        guard let _sensorLogIDChar         = device.findCharacteristic(service, uuid: sensorLogIDCharUUID) else { return nil }
        guard let _sensorLogMetaDataChar   = device.findCharacteristic(service, uuid: sensorLogMetaDataCharUUID) else { return nil }
        guard let _sensorLogDataChar       = device.findCharacteristic(service, uuid: sensorLogDataCharUUID) else { return nil }
        
        self.sensorSettingChar       = _sensorSettingChar
        self.sensorRealTimeDataChar  = _sensorRealTimeDataChar
        self.sensorLogIDChar         = _sensorLogIDChar
        self.sensorLogMetaDataChar   = _sensorLogMetaDataChar
        self.sensorLogDataChar       = _sensorLogDataChar
        
        //初期値読み出し。
        device.readValue(self.sensorSettingChar)
        
        // Notifyを有効に
        device.setNotify(self.sensorRealTimeDataChar, enabled: true)
        device.setNotify(self.sensorLogDataChar, enabled: true)
    }
    
    // MARK: - Private methods
    func unpackDataArray(_ range:S, value: [UInt8]) -> [T]?
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
    func didUpdateValue(_ characteristic: CBCharacteristic, data: [UInt8])
    {
        switch characteristic.uuid {
        case sensorSettingChar.uuid:
            guard let settingData = SensorSettingData<S>.unpack(data) else {
                // FIXME 不正なデータ処理
                assert(false, #function)
                break
            }
            self.settingData = settingData
            
        case sensorRealTimeDataChar.uuid:
            self.realtimeData = T.unpack(self.settingData!.range, value: data)
//debugPrint("\(self.realtimeData)")
            
        case sensorLogIDChar.uuid:
            guard let logID = SensorLogID.unpack(data) else {
                assert(false, #function)
                break
            }
            self.logID = logID

        case sensorLogMetaDataChar.uuid:
            guard let metadata = SensorLogMetaData<S>.unpack(data) else {
                assert(false, #function)
                break
            }
            self.logMetaData = metadata
            
        case sensorLogDataChar.uuid:
            guard let metadata = self.logMetaData else {
//                assert(false, #function)
                break
            }
            if let d = unpackDataArray(metadata.range, value: data) {
//                debugPrint("\(#function) count \(d.count)")
                // データの終端は、必ず送る。データ取りこぼしが起きないように、データ更新も呼び出す。
                if d.count == 0 {
                    DispatchQueue.main.async(execute: {
                        self.delegate?.didUpdateLogData(self)
                        self.delegate?.didFinishedLogData(self)
                    })
                } else {
                    // データを追加。スレッドをまたぐので、selfをロックオブジェクトに使う。
                    objc_sync_enter(self)
                    self.logData.append(contentsOf: d)
                    objc_sync_exit(self)
                    // 過度な呼び出しにならないように、メインスレッドでの処理が終わったら次の処理を入れるようにする。
                    if(self.flag == false) {
                        self.flag = true
                        DispatchQueue.main.async(execute: {
                            self.delegate?.didUpdateLogData(self)
                            self.flag = false
                        })
                    }
                }
            }
//            debugPrint("unpacked : \(d)")
            
        default:
            assert(false, "\(#function), unexpected cahatacter: \(characteristic)")
            break
        }
    }

    // MARK: -  Public methods

    // セッテイングを書き込みます
    open func writeSetting(_ setting: SensorSettingData<S>)
    {
        let data = setting.pack()
        device.writeValue(sensorSettingChar, value: data)
    }
    
    open func readSetting()
    {
        device.readValue(sensorSettingChar)
    }

    // ログIDを書き込みます。自動的にメタデータが更新されます。
    open func writeLogID(_ logID: SensorLogID)
    {
        device.setNotify(self.sensorLogDataChar, enabled: false)

        self.logID = logID
        let data = logID.pack()
        device.writeValue(sensorLogIDChar, value: data)
        updateLogMetaData()
        
        device.setNotify(self.sensorLogDataChar, enabled: true)
        
        // delegate呼び出しのフラグをクリアしておく
        self.flag = false
    }
    
    // メタデータの更新
    open func updateLogMetaData()
    {
        device.readValue(sensorLogMetaDataChar)
    }
    
    // センサデータを読みだし
    open func readLogData() -> [T]
    {
        var ret: [T]
        
        objc_sync_enter(self)
        ret = self.logData
        self.logData = []
        objc_sync_exit(self)
        
        return ret
    }
}
