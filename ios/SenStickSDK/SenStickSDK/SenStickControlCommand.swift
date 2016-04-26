//
//  SenStickStatus.swift
//  SenStickSDK
//
//  Created by AkihiroUehara on 2016/04/20.
//  Copyright © 2016年 AkihiroUehara. All rights reserved.
//

import Foundation


public enum SenStickControlCommand : UInt8, CustomStringConvertible
{
    case DeepSleep              = 0x00
    case StopSensingAndLogging  = 0x01
    case StartSensing           = 0x02
    case StartLogging           = 0x03
    case FormatStrage           = 0x04
    case EnterDFUMode           = 0x10
    
    public var description : String
    {
        switch self {
        case .DeepSleep              : return "DeepSleepCommand"
        case .StopSensingAndLogging  : return "StopSensingAndLoggingCommand"
        case .StartSensing           : return "StartSensingCommand"
        case .StartLogging           : return "StartLoggingCommand"
        case .FormatStrage           : return "FormatStrageCommand"
        case .EnterDFUMode           : return "EnterDFUModeCommand"
        }
    }
}
