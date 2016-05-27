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
    let queue: dispatch_queue_t
    var manager: CBCentralManager?
    
    var scanTimer: dispatch_source_t?
    var scanCallback:((remaining: NSTimeInterval) -> Void)?
    
    // Properties, KVO
    dynamic public private(set) var devices:[SenStickDevice] = []
    dynamic public private(set) var state: CBCentralManagerState = .Unknown
    dynamic public private(set) var isScanning: Bool = false
    
    // Initializer, Singleton design pattern.
    public static let sharedInstance: SenStickDeviceManager = SenStickDeviceManager()
    
    private override init() {
        queue = dispatch_queue_create("senstick.ble-queue", DISPATCH_QUEUE_SERIAL)
        
        super.init()
        
        manager = CBCentralManager.init(delegate: self, queue: queue)
    }
    
    // MARK: Public methods
    
    // 1秒毎にコールバックします。0になれば終了です。
    public func scan(duration:NSTimeInterval = 5.0, callback:((remaining: NSTimeInterval) -> Void)?)
    {
        // スキャン中、もしくはBTの電源がオフであれば、直ちに終了。
        if manager!.isScanning || manager!.state != CBCentralManagerState.PoweredOn {
            callback?(remaining: 0)
            return
        }
        
        // スキャン時間は1秒以上、30秒以下に制約
        let scanDuration = max(1, min(30, duration))
        scanCallback = callback
        
        // 接続済のペリフェラルを取得する
        for peripheral in (manager!.retrieveConnectedPeripheralsWithServices([SenStickUUIDs.advertisingServiceUUID])) {
            addPeripheral(peripheral)
        }
        
        // スキャンを開始する。
        manager!.scanForPeripheralsWithServices([SenStickUUIDs.advertisingServiceUUID], options: nil)
        isScanning = true
        
        var remaining = scanDuration
        scanTimer = dispatch_source_create(DISPATCH_SOURCE_TYPE_TIMER, 0, 0, dispatch_get_main_queue())
        dispatch_source_set_timer(scanTimer!, DISPATCH_TIME_NOW, 1 * NSEC_PER_SEC, 100 * 1000 * USEC_PER_SEC) // 1秒ごとのタイマー
        dispatch_source_set_event_handler(scanTimer!) {
            // 時間を-1秒。
            remaining = max(0, remaining - 1)
            if remaining <= 0 {
                self.cancelScan()
            }
            // 継続ならばシグナリング
            self.scanCallback?(remaining: remaining)
        }
        dispatch_resume(scanTimer!)
    }
    
    public func scan(duration:NSTimeInterval = 5.0)
    {
        scan(duration, callback: nil)
    }
    
    public func cancelScan()
    {
        guard manager!.isScanning else { return }
        
        dispatch_source_cancel(scanTimer!)
        
        self.scanCallback?(remaining: 0)
        self.scanCallback = nil
        
        self.manager!.stopScan()
        self.isScanning = false
    }
    
    // MARK: Private methods
    func addPeripheral(peripheral: CBPeripheral)
    {
        //すでに配列にあるかどうか探す, なければ追加。KVOを活かすため、配列それ自体を代入する
        if !devices.contains({ element -> Bool in element.peripheral == peripheral }) {
            var devs = Array<SenStickDevice>(self.devices)
            devs.append(SenStickDevice(manager: self.manager!, peripheral: peripheral))
            dispatch_async(dispatch_get_main_queue(), {
                self.devices = devs
            })
        }
    }
    
    // MARK: CBCentralManagerDelegate
    public func centralManagerDidUpdateState(central: CBCentralManager)
    {
        // BLEの処理は独立したキューで走っているので、KVOを活かすためにメインキューで代入する
        dispatch_async(dispatch_get_main_queue(), {
            self.state = central.state
        })
        
        switch central.state {
        case .PoweredOn: break
        case .PoweredOff:
            dispatch_async(dispatch_get_main_queue(), {
                self.devices = []
            })
        case .Unauthorized:
            dispatch_async(dispatch_get_main_queue(), {
                self.devices = []
            })
        case .Unknown:
            dispatch_async(dispatch_get_main_queue(), {
                self.devices = []
            })
        case .Unsupported:
            dispatch_async(dispatch_get_main_queue(), {
                self.devices = []
            })
            break
        default: break
        }
    }
    
    public func centralManager(central: CBCentralManager, didDiscoverPeripheral peripheral: CBPeripheral, advertisementData: [String : AnyObject], RSSI: NSNumber)
    {
        //        debugPrint("\(#function)")
        dispatch_async(dispatch_get_main_queue(), {
            self.addPeripheral(peripheral)
        })
    }
    
    public func centralManager(central: CBCentralManager, didConnectPeripheral peripheral: CBPeripheral)
    {
        //        debugPrint("\(#function)")
        for device in devices.filter({element -> Bool in element.peripheral == peripheral}) {
            device.onConnected()
        }
    }
    
    public func centralManager(central: CBCentralManager, didDisconnectPeripheral peripheral: CBPeripheral, error: NSError?)
    {
        //        debugPrint("\(#function)")
        for device in devices.filter({element -> Bool in element.peripheral == peripheral}) {
            device.onDisConnected()
        }
    }
}
