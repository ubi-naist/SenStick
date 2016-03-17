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
    // アドバタイジングするサービスUUID
    public static let advertisingServiceUUID:CBUUID = CBUUID.init(string: "F000bb00-0451-4000-B000-000000000000")
    
    // sensor(peripheral devices) settings
    public static let settingServiceUUID:CBUUID      = CBUUID.init(string: "F000dd00-0451-4000-B000-000000000000")
    public static let settingDateTimeUUID:CBUUID     = CBUUID.init(string: "F000dd01-0451-4000-B000-000000000000")
    public static let settingRangeUUID:CBUUID        = CBUUID.init(string: "F000dd02-0451-4000-B000-000000000000")
    public static let settingSamplingRateUUID:CBUUID = CBUUID.init(string: "F000dd03-0451-4000-B000-000000000000")

    // 
    
    // streaming
    // storage control point
    // storage readout point
    
    public static let streamingServiceUUID:  CBUUID = CBUUID.init(string: "F000dd00-0451-4000-B000-000000000000")
    public static let streamingSettingUUID:  CBUUID = CBUUID.init(string: "F000dd01-0451-4000-B000-000000000000")
    public static let streamingDataUUID:  CBUUID    = CBUUID.init(string: "F000dd01-0451-4000-B000-000000000000")
    
    
    
    
    
    public static let loggingServiceUUID:CBUUID     = CBUUID.init(string: "F000bb00-0451-4000-B000-000000000000")
    public static let loggingControlPointUUID:CBUUID= CBUUID.init(string: "F000bb01-0451-4000-B000-000000000000")
    
    
    public static let loggingStorageStatusUUID:CBUUID    = CBUUID.init(string: "F000bb02-0451-4000-B000-000000000000")
    public static let loggingRTCDateTimeUUID:CBUUID = CBUUID.init(string: "F000bb03-0451-4000-B000-000000000000")
    public static let loggingMetaDataAbstractUUID:CBUUID = CBUUID.init(string: "F000bb04-0451-4000-B000-000000000000")
    
    public static let logReadOutServiceUUID: CBUUID = CBUUID.init(string: "F000cc00-0451-4000-B000-000000000000")
    public static let logReadOutControlPointUUID:CBUUID = CBUUID.init(string: "F000cc01-0451-4000-B000-000000000000")
    public static let logReadOutStatusUUID:CBUUID  = CBUUID.init(string: "F000cc02-0451-4000-B000-000000000000")
    public static let logReadOutDataUUID:CBUUID    = CBUUID.init(string: "F000cc03-0451-4000-B000-000000000000")
    public static let logReadOutMetaDataSettingUUID:CBUUID  = CBUUID.init(string: "F000cc04-0451-4000-B000-000000000000")
    public static let logReadOutMetaDataDateTimeUUID:CBUUID = CBUUID.init(string: "F000cc05-0451-4000-B000-000000000000")
    public static let logReadOutMetaDataAbstractUUID:CBUUID = CBUUID.init(string: "F000cc06-0451-4000-B000-000000000000")
    
}