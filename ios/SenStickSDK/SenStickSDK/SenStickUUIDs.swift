//
//  SenStickUUIDs.swift
//  SenStickSDK
//
//  Created by AkihiroUehara on 2016/03/16.
//  Copyright © 2016年 AkihiroUehara. All rights reserved.
//

import Foundation
import CoreBluetooth


public struct SenStickUUIDs
{
    // ベースUUID
    public static let baseUUIDString = "F0000000-0451-4000-B000-000000000000"
    public static let baseUUID:CBUUID = {return SenStickUUIDs.createSenstickUUID(0x0000)}()
    // UUID生成メソッド
    public static func createSenstickUUID(_ uuid:UInt16) -> CBUUID {
        return CBUUID.init(string: (SenStickUUIDs.baseUUIDString as NSString).replacingCharacters(in: NSMakeRange(4, 4), with: NSString(format: "%04x", uuid) as String))
    }
    public static func getShortUUID(_ uuid:CBUUID) -> UInt16 {
        // UUIDをバイト配列にして、16ビット短縮UUID部分を取り出す
        var buf = [UInt8](repeating: 0, count: 16)
        (uuid.data as NSData).getBytes(&buf, length: buf.count)
        return UInt16.unpack(buf[2..<4], byteOrder: .bigEndian)!
        /*
        // UUIDをバイト配列にして、16ビット短縮UUID部分を取り出す
        var buf = [UInt8](count: 16, repeatedValue: 0)
        uuid.data.getBytes(&buf, length: buf.count)
        let shortuuid = UInt16.unpack(buf[2..<4], byteOrder: .BigEndian)

        // 短縮UUID部分を0クリアして、ベースUUIDと比較する
        buf[2] = 0
        buf[3] = 0

        var baseuuid_data = [UInt8](count: 16, repeatedValue: 0)
        baseUUID.data.getBytes(&baseuuid_data, length: baseuuid_data.count)
        if baseuuid_data == buf {
            return shortuuid
        } else {
            return nil
        }*/
    }

    // アドバタイジングするサービスUUID (コントロールサービス)
    public static let advertisingServiceUUID:CBUUID    = ControlServiceUUID
    
    // Device information service
    public static let DeviceInformationServiceUUID:CBUUID    = {return CBUUID(string: "180A")}()
    public static let ManufacturerNameStringCharUUID:CBUUID  = {return CBUUID(string: "2A29")}()
    public static let HardwareRevisionStringCharUUID:CBUUID  = {return CBUUID(string: "2A27")}()
    public static let FirmwareRevisionStringCharUUID:CBUUID  = {return CBUUID(string: "2A26")}()
    public static let SerialNumberStringCharUUID:CBUUID      = {return CBUUID(string: "2A25")}()
    
    // Battery service
    public static let BatteryServiceUUID:CBUUID    = {return CBUUID(string: "180F")}()
    public static let BatteryLevelCharUUID:CBUUID  = {return CBUUID(string: "2A19")}()

    // control service
    public static let ControlServiceUUID:CBUUID        = {return SenStickUUIDs.createSenstickUUID(0x2000)}()
    public static let StatusCharUUID:CBUUID            = {return SenStickUUIDs.createSenstickUUID(0x7000)}()
    public static let AvailableLogCountCharUUID:CBUUID = {return SenStickUUIDs.createSenstickUUID(0x7001)}()
    public static let StorageStatusCharUUID:CBUUID     = {return SenStickUUIDs.createSenstickUUID(0x7002)}()
    public static let DateTimeCharUUID:CBUUID          = {return SenStickUUIDs.createSenstickUUID(0x7003)}()
    public static let AbstractCharUUID:CBUUID          = {return SenStickUUIDs.createSenstickUUID(0x7004)}()
    public static let DeviceNameCharUUID:CBUUID        = {return SenStickUUIDs.createSenstickUUID(0x7005)}()
    
    // メタデータ読み出しサービス
    public static let MetaDataServiceUUID:CBUUID    = {return SenStickUUIDs.createSenstickUUID(0x2001)}()
    public static let TargetLogIDCharUUID:CBUUID    = {return SenStickUUIDs.createSenstickUUID(0x7010)}()
    public static let TargetDateTimeCharUUID:CBUUID = {return SenStickUUIDs.createSenstickUUID(0x7011)}()
    public static let TargetAbstractCharUUID:CBUUID = {return SenStickUUIDs.createSenstickUUID(0x7012)}()
    
    // センサーのサービスのベースUUID。下1桁はセンサータイプのrawValueがはいります。
    public static let sensorServiceBaseUUID:UInt16           = 0x2100
    public static let sensorSettingCharBaseUUID:UInt16       = 0x7100
    public static let sensorRealTimeDataCharBaseUUID:UInt16  = 0x7200
    public static let sensorLogIDCharBaseUUID:UInt16         = 0x7300
    public static let sensorLogMetaDataCharBaseUUID:UInt16   = 0x7400
    public static let sensorLogDataCharBaseUUID:UInt16       = 0x7500
    
    // センサのサービスのUUID生成
    public static func createSenstickSensorServiceUUID(_ sensorType:SenStickSensorType) -> CBUUID {
        return SenStickUUIDs.createSenstickUUID(sensorServiceBaseUUID | UInt16(sensorType.rawValue))
    }
    // Senstickのセンサデバイスの、キャラクタリスティクスのUUIDの配列、を返します。
    public static func createSenstickSensorCharacteristicUUIDs(_ sensorType: SenStickSensorType) -> [CBUUID]
    {
        return [
            createSenstickUUID(sensorSettingCharBaseUUID | UInt16(sensorType.rawValue)),
            createSenstickUUID(sensorRealTimeDataCharBaseUUID | UInt16(sensorType.rawValue)),
            createSenstickUUID(sensorLogIDCharBaseUUID | UInt16(sensorType.rawValue)),
            createSenstickUUID(sensorLogMetaDataCharBaseUUID | UInt16(sensorType.rawValue)),
            createSenstickUUID(sensorLogDataCharBaseUUID | UInt16(sensorType.rawValue))
        ]
    }

    // センサごとのサービスのUUID
    public static let accelerationSensorServiceUUID:CBUUID  = {return SenStickUUIDs.createSenstickSensorServiceUUID(SenStickSensorType.accelerationSensor) }()
    public static let gyroSensorServiceUUID:CBUUID          = {return SenStickUUIDs.createSenstickSensorServiceUUID(SenStickSensorType.gyroSensor) }()
    public static let magneticFieldSensorServiceUUID:CBUUID = {return SenStickUUIDs.createSenstickSensorServiceUUID(SenStickSensorType.magneticFieldSensor) }()
    public static let brightnessSensorServiceUUID:CBUUID    = {return SenStickUUIDs.createSenstickSensorServiceUUID(SenStickSensorType.brightnessSensor) }()
    public static let uvSensorServiceUUID:CBUUID            = {return SenStickUUIDs.createSenstickSensorServiceUUID(SenStickSensorType.ultraVioletSensor) }()
    public static let humiditySensorServiceUUID:CBUUID      = {return SenStickUUIDs.createSenstickSensorServiceUUID(SenStickSensorType.humidityAndTemperatureSensor)} ()
    public static let pressureSensorServiceUUID:CBUUID      = {return SenStickUUIDs.createSenstickSensorServiceUUID(SenStickSensorType.airPressureSensor)}()

    // デバイスが持つべき、サービスUUIDがキー、キャラクタリスティクスの配列、の辞書を返します。
    public static let SenStickServiceUUIDs: [CBUUID: [CBUUID]] = [

        DeviceInformationServiceUUID : [ManufacturerNameStringCharUUID, HardwareRevisionStringCharUUID, FirmwareRevisionStringCharUUID, SerialNumberStringCharUUID],
        
        BatteryServiceUUID: [BatteryLevelCharUUID],

        ControlServiceUUID   : [StatusCharUUID, AvailableLogCountCharUUID, StorageStatusCharUUID, DateTimeCharUUID, AbstractCharUUID, DeviceNameCharUUID],
        
        MetaDataServiceUUID  : [TargetLogIDCharUUID, TargetDateTimeCharUUID, TargetAbstractCharUUID],

        {return SenStickUUIDs.createSenstickUUID(sensorServiceBaseUUID | UInt16(SenStickSensorType.accelerationSensor.rawValue)) }() :
          { return SenStickUUIDs.createSenstickSensorCharacteristicUUIDs(SenStickSensorType.accelerationSensor ) }(),

        {return SenStickUUIDs.createSenstickUUID(sensorServiceBaseUUID | UInt16(SenStickSensorType.gyroSensor.rawValue)) }() :
          { return SenStickUUIDs.createSenstickSensorCharacteristicUUIDs(SenStickSensorType.gyroSensor ) }(),
          
        {return SenStickUUIDs.createSenstickUUID(sensorServiceBaseUUID | UInt16(SenStickSensorType.magneticFieldSensor.rawValue)) }() :
          { return SenStickUUIDs.createSenstickSensorCharacteristicUUIDs(SenStickSensorType.magneticFieldSensor ) }(),
          
        {return SenStickUUIDs.createSenstickUUID(sensorServiceBaseUUID | UInt16(SenStickSensorType.brightnessSensor.rawValue)) }() :
          { return SenStickUUIDs.createSenstickSensorCharacteristicUUIDs(SenStickSensorType.brightnessSensor ) }(),
          
        {return SenStickUUIDs.createSenstickUUID(sensorServiceBaseUUID | UInt16(SenStickSensorType.ultraVioletSensor.rawValue)) }() :
          { return SenStickUUIDs.createSenstickSensorCharacteristicUUIDs(SenStickSensorType.ultraVioletSensor ) }(),
          
        {return SenStickUUIDs.createSenstickUUID(sensorServiceBaseUUID | UInt16(SenStickSensorType.humidityAndTemperatureSensor.rawValue)) }() :
          { return SenStickUUIDs.createSenstickSensorCharacteristicUUIDs(SenStickSensorType.humidityAndTemperatureSensor ) }(),
          
        {return SenStickUUIDs.createSenstickUUID(sensorServiceBaseUUID | UInt16(SenStickSensorType.airPressureSensor.rawValue)) }() :
          { return SenStickUUIDs.createSenstickSensorCharacteristicUUIDs(SenStickSensorType.airPressureSensor ) }()
    ]
}
