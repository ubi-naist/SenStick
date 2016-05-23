//
//  SensorDataContrller.swift
//  SenStickViewer
//
//  Created by AkihiroUehara on 2016/05/23.
//  Copyright © 2016年 AkihiroUehara. All rights reserved.
//

import Foundation
import SenStickSDK

public class SensorDataController<T: SensorDataPackableType, S: RawRepresentable where S.RawValue == UInt16, T.RangeType == S>
{
}