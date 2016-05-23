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
    case Stopping      = 0x00
    case Starting      = 0x01
    case FormatStorage = 0x10
    case DeepSleep     = 0x20
    case EnterDFUMode  = 0x40
    
    public var description : String
    {
        switch self {
        case .Stopping      : return "Stopping"
        case .Starting      : return "Starting"
        case .FormatStorage : return "FormatStorage"
        case .DeepSleep     : return "DeepSleep"
        case .EnterDFUMode  : return "EnterDFUMode"
        }
    }
}
