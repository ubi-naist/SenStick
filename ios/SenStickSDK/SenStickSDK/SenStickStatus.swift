//
//  SenStickStatus.swift
//  SenStickSDK
//
//  Created by AkihiroUehara on 2016/04/20.
//  Copyright © 2016年 AkihiroUehara. All rights reserved.
//

import Foundation

// 動作状態
public enum SenStickStatus : UInt8, CustomStringConvertible
{
    case Stopping          = 0x00
    case Sensing           = 0x01
    case SensingAndLogging = 0x03
    
    public var description : String
    {
        switch self {
        case .Stopping:          return "Stopping"
        case .Sensing:           return "Sensing"
        case .SensingAndLogging: return "SensingAndLogging"
        }
    }
}
