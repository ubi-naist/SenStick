//
//  SenStickDeviceManager.swift
//  SenStickSDK
//
//  Created by AkihiroUehara on 2016/03/15.
//  Copyright © 2016年 AkihiroUehara. All rights reserved.
//

import Foundation
import CoreBluetooth

public class SenStickDeviceManager : NSObject, CBCentralManagerDelegate
{
    let queue: dispatch_queue_t   = dispatch_queue_create("senstick.ble-queue", DISPATCH_QUEUE_SERIAL)
    var centralManager: CBCentralManager!

    // Properties
    public private(set) var devices:[SenStickDevice] = []
    
    // Initializer, Singleton design pattern.
    public static let sharedInstance: SenStickDeviceManager = SenStickDeviceManager()
    
    private override init() {
        super.init()
        
        centralManager = CBCentralManager.init(delegate: self, queue: queue)
    }
    
    // MARK: Public methods
    public func scan(duration:NSTimeInterval)
    {
        retrievePeripherals()
        
        if centralManager!.isScanning && centralManager!.state == CBCentralManagerState.PoweredOn {
            dispatch_after(dispatch_time(DISPATCH_TIME_NOW, Int64(duration * Double(NSEC_PER_SEC))), self.queue, { self.centralManager?.stopScan() })
        }
    }
    
    public func connect(peripheral:CBPeripheral)
    {
        centralManager?.connectPeripheral(peripheral, options: nil)
    }
    
    // MARK: Private methods
    func retrievePeripherals()
    {
        for peripheral in (centralManager!.retrieveConnectedPeripheralsWithServices([SenStickUUIDs.advertisingServiceUUID])) {
            addPeripheral(peripheral)
        }
    }

    func addPeripheral(peripheral: CBPeripheral)
    {
        //すでに配列にあるかどうか探す, なければ追加
        if let _ = devices.indexOf({$0.peripheral == peripheral}) {
        } else {
            let device = SenStickDevice(peripheral)
            devices.append(device)
        }
    }

    // MARK: CBCentralManagerDelegate
    public func centralManagerDidUpdateState(central: CBCentralManager)
    {
        switch central.state {
        case CBCentralManagerState.PoweredOn:
            scan(5.0) // 5秒ほどスキャン
        default:
            devices = []
        }
    }
    
    public func centralManager(central: CBCentralManager, didDiscoverPeripheral peripheral: CBPeripheral, advertisementData: [String : AnyObject], RSSI: NSNumber)
    {
    }
    
    public func centralManager(central: CBCentralManager, didConnectPeripheral peripheral: CBPeripheral)
    {
    }
    
    public func centralManager(central: CBCentralManager, didDisconnectPeripheral peripheral: CBPeripheral, error: NSError?)
    {
    }
}
