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
    
    // control service
    public static let senstickServiceUUID:CBUUID      = CBUUID.init(string: "F000bb00-0451-4000-B000-000000000000")
    public static let senstickControlPointUUID:CBUUID = CBUUID.init(string: "F000bb01-0451-4000-B000-000000000000")
    
    // streaming point
    public static let sensorServiceUUID:  CBUUID    = CBUUID.init(string: "F000cc00-0451-4000-B000-000000000000")
    public static let sensorSettingUUID:CBUUID      = CBUUID.init(string: "F000cc01-0451-4000-B000-000000000000")
    public static let sensorDataUUID:  CBUUID       = CBUUID.init(string: "F000cc02-0451-4000-B000-000000000000")
    public static let sensorDateTimeUUID:CBUUID     = CBUUID.init(string: "F000cc03-0451-4000-B000-000000000000")
    public static let sensorMetaDataAbstractTextUUID:CBUUID = CBUUID.init(string: "F000cc04-0451-4000-B000-000000000000")
    
    // log readout
    public static let logReadOutServiceUUID: CBUUID = CBUUID.init(string: "F000dd00-0451-4000-B000-000000000000")
    public static let logReadOutControlPointUUID:CBUUID     = CBUUID.init(string: "F000dd01-0451-4000-B000-000000000000")
    public static let logReadOutStatusUUID:CBUUID           = CBUUID.init(string: "F000dd02-0451-4000-B000-000000000000")
    public static let logReadOutDataUUID:CBUUID             = CBUUID.init(string: "F000dd03-0451-4000-B000-000000000000")
    public static let logReadOutMetaDataSettingUUID:CBUUID  = CBUUID.init(string: "F000dd04-0451-4000-B000-000000000000")
    public static let logReadOutMetaDataDateTimeUUID:CBUUID = CBUUID.init(string: "F000dd05-0451-4000-B000-000000000000")
    public static let logReadOutMetaDataAbstractUUID:CBUUID = CBUUID.init(string: "F000dd06-0451-4000-B000-000000000000")
}