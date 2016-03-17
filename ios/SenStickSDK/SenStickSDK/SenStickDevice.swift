//
//  SenStickDevice.swift
//  SenStickSDK
//
//  Created by AkihiroUehara on 2016/03/15.
//  Copyright © 2016年 AkihiroUehara. All rights reserved.
//

import Foundation
import CoreBluetooth

public class SenStickDevice : NSObject, CBPeripheralDelegate {

    
    // MARK: Properties
    let peripheral:CBPeripheral
    
    // MARK: Initializer
    init(peripheral:CBPeripheral)
    {
        self.peripheral = peripheral
        self.peripheral.delegate = self
    }
    
    // MARK: Private methods
    func findServicesAn()
    {}
    
    // MARK: Public methods
    
    // CBPeripheralDelegate
//    optional public func peripheralDidUpdateName(peripheral: CBPeripheral)
//    optional public func peripheral(peripheral: CBPeripheral, didModifyServices invalidatedServices: [CBService])
//    optional public func peripheral(peripheral: CBPeripheral, didReadRSSI RSSI: NSNumber, error: NSError?)
//    optional public func peripheral(peripheral: CBPeripheral, didDiscoverServices error: NSError?)
//    optional public func peripheral(peripheral: CBPeripheral, didDiscoverIncludedServicesForService service: CBService, error: NSError?)
//    optional public func peripheral(peripheral: CBPeripheral, didDiscoverCharacteristicsForService service: CBService, error: NSError?)
//    optional public func peripheral(peripheral: CBPeripheral, didUpdateValueForCharacteristic characteristic: CBCharacteristic, error: NSError?)
//    optional public func peripheral(peripheral: CBPeripheral, didWriteValueForCharacteristic characteristic: CBCharacteristic, error: NSError?)
//    optional public func peripheral(peripheral: CBPeripheral, didUpdateNotificationStateForCharacteristic characteristic: CBCharacteristic, error: NSError?)
//    optional public func peripheral(peripheral: CBPeripheral, didDiscoverDescriptorsForCharacteristic characteristic: CBCharacteristic, error: NSError?)
//    optional public func peripheral(peripheral: CBPeripheral, didUpdateValueForDescriptor descriptor: CBDescriptor, error: NSError?)
//    optional public func peripheral(peripheral: CBPeripheral, didWriteValueForDescriptor descriptor: CBDescriptor, error: NSError?)
}

}