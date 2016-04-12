//
//  SenStickSensorSetting.swift
//  SenStickSDK
//
//  Created by AkihiroUehara on 2016/03/18.
//  Copyright © 2016年 AkihiroUehara. All rights reserved.
//

import Foundation

protocol ScalarBaseType
{
    associatedtype RawValueType
    var rawValue : RawValueType{get set}
    init(rawValue:RawValueType)
}

protocol ThreeAxisBaseType
{
    associatedtype RawValueType
    var xRawValue : RawValueType{get set}
    var yRawValue : RawValueType{get set}
    var zRawValue : RawValueType{get set}
    init(xRawValue:RawValueType, yRawValue:RawValueType, zRawValue:RawValueType)
}

// 16ビット 符号付き数値。フルスケールは設定レンジ値による。2, 4, 8, 16G。
struct AccelerationRawData : ThreeAxisBaseType
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
struct BrightnessRawData : ScalarBaseType
{
    typealias T = UInt16
    var rawValue : T
    init(rawValue:T)
    {
        self.rawValue = rawValue
    }
}

// UVデータ // サンプリング周期275ミリ秒、単位換算 2.14 uW/cm2/step
struct UltraVioletRawData : ScalarBaseType
{
    typealias T = UInt16
    var rawValue : T
    init(rawValue:T)
    {
        self.rawValue = rawValue
    }
}

// 気圧データ// 24-bit, 4096 LSB/ hPa
struct AirePressureRawData : ScalarBaseType
{
    typealias T = UInt32
    var rawValue : T
    init(rawValue:T)
    {
        self.rawValue = rawValue
    }
}

// 湿度データ // 変換式、仕様書10ページ、RH = -6 + 125 * SRH / 2^(16)
struct HumidityRawData : ScalarBaseType
{
    typealias T = UInt16
    var rawValue : T
    init(rawValue:T)
    {
        self.rawValue = rawValue
    }
}

// 温度データ // 変換式、仕様書10ページ、T = -46.85 + 175.72 * St/ 2^(16)
struct TemperatureRawData : ScalarBaseType
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

// 物理的なセンサーの種別
enum SensorDeviceType : UInt8, CustomStringConvertible
{
    case AccelerationSensor              = 0
    case GyroSensor                      = 1
    case MagneticFieldSensor             = 2
    case BrightnessSensor                = 3
    case UltraVioletSensor               = 4
    case HumidityAndTemperatureSensor    = 5
    case AirPressureSensor               = 6
    
    var description : String {
        switch self {
        case .AccelerationSensor:   return "AccelerationSensor"
        case .GyroSensor:           return "GyroSensor"
        case .MagneticFieldSensor:  return "MagneticFieldSensor"
        case .BrightnessSensor:     return "BrightnessSensor"
        case .UltraVioletSensor:    return "UltraVioletSensor"
        case .HumidityAndTemperatureSensor: return "HumidityAndTemperatureSensor"
        case .AirPressureSensor:    return "AirPressureSensor"
        }
    }
}


// 加速度センサーの範囲設定値。列挙側の値は、BLEでの設定値に合わせている。
enum AccelerationRange : UInt8, CustomStringConvertible
{
    case ACCELERATION_RANGE_2G   = 0x00 // +- 2g
    case ACCELERATION_RANGE_4G   = 0x01 // +- 4g
    case ACCELERATION_RANGE_8G   = 0x02 // +- 8g
    case ACCELERATION_RANGE_16G  = 0x03 // +- 16g
    
    var description : String {
        switch self {
        case .ACCELERATION_RANGE_2G: return "2G"
        case .ACCELERATION_RANGE_4G: return "4G"
        case .ACCELERATION_RANGE_8G: return "8G"
        case .ACCELERATION_RANGE_16G:return "16G"
        }
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

// サンプリング・レート
struct SamplingDurationType : CustomStringConvertible
{
    let duration : NSTimeInterval
    init(milliSec value: Int16)
    {
        duration = 1e-3 * Double(value)
    }
    
    var description: String {
        return "\(duration * 1e3) milliseconds"
    }
}

struct SensorSetting : CustomStringConvertible
{
    var accelerationRange   : AccelerationRange
    var rotationRange       : RotationRange
    
    // サンプリングレートの単位はミリ秒, 0はサンプリング対象外
    var accelerationSamplingPeriod      : SamplingDurationType
    var gyroSamplingPeriod              : SamplingDurationType
    var magneticFieldSamplingPeriod     : SamplingDurationType
    var humidityAndTemperatureSamplingPeriod : SamplingDurationType
    var airPressureSamplingPeriod       : SamplingDurationType
    var brightnessSamplingPeriod        : SamplingDurationType
    var ultraVioletSamplingPeriod       : SamplingDurationType
    
    var description : String {
        return
          "AccelerationRange: \(accelerationRange) ms,"
        + "rotationRange: \(rotationRange) ms,"
        + "accelerationSamplingPeriod: \(accelerationSamplingPeriod) ms,"
        + "gyroSamplingPeriod: \(gyroSamplingPeriod) ms,"
        + "magneticFieldSamplingPeriod: \(magneticFieldSamplingPeriod) ms,"
        + "humidityAndTemperatureSamplingPeriod: \(humidityAndTemperatureSamplingPeriod) ms,"
        + "airPressureSamplingPeriod: \(airPressureSamplingPeriod) ms,"
        + "brightnessSamplingPeriod: \(brightnessSamplingPeriod) ms,"
        + "ultraVioletSamplingPeriod: \(ultraVioletSamplingPeriod) ms"
    }
}
