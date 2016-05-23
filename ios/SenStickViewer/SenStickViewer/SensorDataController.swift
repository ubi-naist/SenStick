//
//  SensorDataContrller.swift
//  SenStickViewer
//
//  Created by AkihiroUehara on 2016/05/23.
//  Copyright © 2016年 AkihiroUehara. All rights reserved.
//

import Foundation
import SenStickSDK

class SensorDataController<T: SensorDataPackableType, S: RawRepresentable where S.RawValue == UInt16, T.RangeType == S> : SenStickSensorServiceDelegate
{
    weak var service: SenStickSensorService<T, S>? {
        willSet {
            self.service?.delegate = nil
        }
        didSet {
            self.service?.delegate = self
        }
    }
    
    // MARK: - SenStickSensorServiceDelegate
    func didUpdateSetting(sender:AnyObject)
    {
        debugPrint("metadata \(service?.settingData)")
    }
    func didUpdateRealTimeData(sender: AnyObject)
    {
        debugPrint("real \(service?.logData)")
    }
    func didUpdateMetaData(sender: AnyObject)
    {
        debugPrint("metadata \(service?.logMetaData)")
    }
    func didUpdateLogData(sender: AnyObject)
    {
        debugPrint("log \(service?.logData)")
    }
}