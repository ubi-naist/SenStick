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
    case stopping      = 0x00
    case starting      = 0x01
    case formatStorage = 0x10
    case deepSleep     = 0x20
    case enterDFUMode  = 0x40
    
    public var description : String
    {
        switch self {
        case .stopping      : return "Stopping"
        case .starting      : return "Starting"
        case .formatStorage : return "FormatStorage"
        case .deepSleep     : return "DeepSleep"
        case .enterDFUMode  : return "EnterDFUMode"
        }
    }
}
