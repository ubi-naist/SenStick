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
    
    var manager: CBCentralManager?
    var scanStopTime: NSDate = NSDate.init()
    
    // Properties, KVO
    public var devices:[SenStickDevice] = []
    public private(set) var state: CBCentralManagerState = .Unknown
    public private(set) var isScanning: Bool = false
    
    // Initializer, Singleton design pattern.
    public static let sharedInstance: SenStickDeviceManager = SenStickDeviceManager()
    
    private override init() {
        super.init()

        manager = CBCentralManager.init(delegate: self, queue: queue)
    }
    
    // MARK: Public methods
    public func scan(duration:NSTimeInterval = 15.0)
    {
        // スキャン時間は1秒以上、30秒以下に制約
        let scanDuration = max(1, min(30, duration))
        
        // 接続済のペリフェラルを取得する
        if manager!.state == CBCentralManagerState.PoweredOn {
            for peripheral in (manager!.retrieveConnectedPeripheralsWithServices([SenStickUUIDs.advertisingServiceUUID])) {
                addPeripheral(peripheral)
            }
        }
        
        // 電源がONでかつスキャンしていなければ、スキャンを開始する
        if manager!.state == CBCentralManagerState.PoweredOn && !manager!.isScanning {
            manager!.scanForPeripheralsWithServices([SenStickUUIDs.advertisingServiceUUID], options: nil)
            isScanning = true
        }
        
        // スキャン停止タイマーをセット, スキャンメソッドは何度も呼び出される可能性があるので、ストップが何度も呼び出されても不意なスキャン停止が起きないように、スキャン停止は絶対時間で判定する
        scanStopTime = NSDate(timeIntervalSinceNow: scanDuration - 0.5)
        dispatch_after(dispatch_time(DISPATCH_TIME_NOW, Int64(scanDuration * Double(NSEC_PER_SEC))), queue, {
            if self.scanStopTime.timeIntervalSinceNow < 0 {
                self.manager!.stopScan()
                self.isScanning = false
            }
        })
    }
    
    // MARK: Private methods
    func addPeripheral(peripheral: CBPeripheral)
    {
        //すでに配列にあるかどうか探す, なければ追加。KVOを活かすため、配列それ自体を代入する
        if !devices.contains({ element -> Bool in element.peripheral == peripheral }) {
            var devs = devices
            devs.append(SenStickDevice(manager: manager!, peripheral: peripheral))
            self.devices = devs
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
        case CBCentralManagerState.PoweredOn:
            // 電源ONで5秒ほどスキャンする
            scan(5.0)
        default:
            break
        }
    }
    
    public func centralManager(central: CBCentralManager, didDiscoverPeripheral peripheral: CBPeripheral, advertisementData: [String : AnyObject], RSSI: NSNumber)
    {
        dispatch_async(dispatch_get_main_queue(), {
            self.addPeripheral(peripheral)
        })
    }
    
    public func centralManager(central: CBCentralManager, didConnectPeripheral peripheral: CBPeripheral)
    {
    }
    
    public func centralManager(central: CBCentralManager, didDisconnectPeripheral peripheral: CBPeripheral, error: NSError?)
    {
    }
}
