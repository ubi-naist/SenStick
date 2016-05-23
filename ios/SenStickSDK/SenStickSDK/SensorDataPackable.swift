//
//  SensorDataPackable.swift
//  SenStickSDK
//
//  Created by AkihiroUehara on 2016/04/20.
//  Copyright © 2016年 AkihiroUehara. All rights reserved.
//

import Foundation

public protocol SensorDataPackableType {
    associatedtype RangeType
    static func unpack(range:RangeType, value: [UInt8]) -> Self?
}
