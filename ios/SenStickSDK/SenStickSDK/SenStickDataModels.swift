//
//  SenStickSensorSetting.swift
//  SenStickSDK
//
//  Created by AkihiroUehara on 2016/03/18.
//  Copyright © 2016年 AkihiroUehara. All rights reserved.
//

import Foundation

/*
// ジャイロのデータ構造体
// 16ビット 符号付き数値。フルスケールは設定レンジ値による。250, 500, 1000, 2000 DPS。
struct RotationRawData: ThreeAxisBaseType
{
    typealias T = Int16
    var xRawValue : T
    var yRawValue : T
    var zRawValue : T
    init(xRawValue:T, yRawValue:T, zRawValue:T)
    {
        self.xRawValue = xRawValue
        self.yRawValue = yRawValue
        self.zRawValue = zRawValue
    }
}

// 磁界のデータ構造体
// 16-bit 符号付き数字 フルスケール f ±4800 μT
struct MagneticFielRawData: ThreeAxisBaseType
{
    typealias T = Int16
    var xRawValue : T
    var yRawValue : T
    var zRawValue : T
    init(xRawValue:T, yRawValue:T, zRawValue:T)
    {
        self.xRawValue = xRawValue
        self.yRawValue = yRawValue
        self.zRawValue = zRawValue
    }
}

// 輝度データ // 単位 lx // 変換時間約150ミリ秒
struct BrightnessRawData : RawRepresentable
{
    typealias T = UInt16
    var rawValue : T
    init(rawValue:T)
    {
        self.rawValue = rawValue
    }
}

// UVデータ // サンプリング周期275ミリ秒、単位換算 2.14 uW/cm2/step
struct UltraVioletRawData : RawRepresentable
{
    typealias T = UInt16
    var rawValue : T
    init(rawValue:T)
    {
        self.rawValue = rawValue
    }
}

// 気圧データ// 24-bit, 4096 LSB/ hPa
struct AirePressureRawData : RawRepresentable
{
    typealias T = UInt32
    var rawValue : T
    init(rawValue:T)
    {
        self.rawValue = rawValue
    }
}

// 湿度データ // 変換式、仕様書10ページ、RH = -6 + 125 * SRH / 2^(16)
struct HumidityRawData : RawRepresentable
{
    typealias T = UInt16
    var rawValue : T
    init(rawValue:T)
    {
        self.rawValue = rawValue
    }
}

// 温度データ // 変換式、仕様書10ページ、T = -46.85 + 175.72 * St/ 2^(16)
struct TemperatureRawData : RawRepresentable
{
    typealias T = UInt16
    var rawValue : T
    init(rawValue:T)
    {
        self.rawValue = rawValue
    }
}

// 温度と湿度データ
struct HumidityAndTemperatureRawData
{
    let humidity: HumidityRawData
    let temperature: TemperatureRawData
    init(humidityRawValue: UInt16, temperatureRawValue: UInt16)
    {
        humidity    = HumidityRawData(rawValue: humidityRawValue)
        temperature = TemperatureRawData(rawValue: temperatureRawValue)
    }
}

// ジャイロセンサーの範囲設定値。列挙側の値は、BLEでの設定値に合わせている。
enum RotationRange : UInt8, CustomStringConvertible
{
    case ROTATION_RANGE_250DPS   = 0x00
    case ROTATION_RANGE_500DPS   = 0x01
    case ROTATION_RANGE_1000DPS  = 0x02
    case ROTATION_RANGE_2000DPS  = 0x03

    var description : String {
        switch self {
        case .ROTATION_RANGE_250DPS:    return "250DPS"
        case .ROTATION_RANGE_500DPS:    return "500DPS"
        case .ROTATION_RANGE_1000DPS:   return "1000DPS"
        case .ROTATION_RANGE_2000DPS:   return "2000DPS"
        }
    }
}
 */
