//
//  SenStickStatus.swift
//  SenStickSDK
//
//  Created by AkihiroUehara on 2016/04/20.
//  Copyright © 2016年 AkihiroUehara. All rights reserved.
//

import Foundation

// 動作状態
public enum SenStickStatus : UInt8 , CustomStringConvertible
{
    case stopping          = 0x00
    case sensing           = 0x01
    case sensingAndLogging = 0x03

    public var description : String
    {
        switch self {
        case .stopping:          return "Stopping"
        case .sensing:           return "Sensing"
        case .sensingAndLogging: return "SensingAndLogging"
        }
    }
}
