//
//  SenStickControlService.swift
//  SenStickSDK
//
//  Created by AkihiroUehara on 2016/03/29.
//  Copyright © 2016年 AkihiroUehara. All rights reserved.
//

import Foundation
import CoreBluetooth

class SenStickControlService : NSObject, SenStickService
{
    static let ServiceUUID: CBUUID = SenStickUUIDs.senstickServiceUUID
    static let CharacteristicsUUIDs : [CBUUID] = [SenStickUUIDs.senstickControlPointUUID]
    
    unowned let device: SenStickDevice
    let controlPointCharacteristic: CBCharacteristic
    
    // イニシャライザ
    required init?(device:SenStickDevice)
    {
        super.init()

        guard let ctr_char = SenStickDevice.findCharacteristic(device, uuid: SenStickUUIDs.senstickControlPointUUID) else {
            return nil
        }
        
        self.device = device
        controlPointCharacteristic = ctr_char
        
        // Notifyを有効に。初期値読み出し。
        device.setNotify(controlPointCharacteristic, enabled: true)
        device.readValue(controlPointCharacteristic)
    }
    
    func didUpdateValue(peripheral: CBPeripheral, data:NSData)
    
    
    
    
    // インスタンスの構築、サービスとキャラクタリスティクスを検索して内部保存。必要があればキャラクタリスティクスの読み出し。
    // 初期化のためのフラグを内部で保存していないとダメか?
    // Connectingの状態ではあるんだよね。でもサービスはサービスで隠蔽できるけど。読み出し処理が終わらないとダメだよね。その遷移の状態をどう扱えばいいだろうか。
    // 状態としては不定。デバイス自体はCBPeripheralで取れているのだけど、Connectingでサービスそれ自体は独立しているから、なくてもいい。
    // SenStickDeviceが取れても、サービスはしばらく不定でいい。で、コールバックが帰ってきて初期化まですめば、それを戻せばいいけど、どこで戻るのかがわからない。
    // でもサービスがあってCharがあれば、戻ってくるよね?
    static func hoge(CBPeripheral: peripheral) -> SenStickControlService? {
    
    }
    // Variables
    // Properties
    // Initializer
    // Public methods
    // センシングを示す
    // ロギングをせいmす
    // スリープを示す
    // ストレージのフォーマット
    // DFUモード
    
    private(set) var isSensing    : Bool
    private(set) var isLogging    : Bool
    private(set) var isFormatting : Bool

    init()
    {
        // デバイス上を対の読み出し
    }
    
    // センシングのステートを変更します
    func setSensing(shouldStart:Bool, callback:(device:SenStickDevice, service:SenStickControlService) -> Void)
    {
    }
    
    func stopSensing() -> Void {
        
    }
    
    func startlogging() -> Void {
        
    }
    
    func stopLogging() -> Void {
        
    }
    
    func startFormat() -> Void {
        
    }
    

}