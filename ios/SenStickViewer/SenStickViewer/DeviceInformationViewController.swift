//
//  DeviceInformationViewController.swift
//  SenStickViewer
//
//  Created by AkihiroUehara on 2016/06/12.
//  Copyright © 2016年 AkihiroUehara. All rights reserved.
//

import UIKit
import SenStickSDK
import iOSDFULibrary
import CoreBluetooth

class DeviceInformationViewController : UIViewController, LoggerDelegate, DFUServiceDelegate, DFUProgressDelegate, CBCentralManagerDelegate
{
    // Properties
    
    @IBOutlet var serialNumberTextLabel: UILabel!
    @IBOutlet var manufacturerNameTextLabel: UILabel!
    @IBOutlet var hardwareRevisionTextLabel: UILabel!
    @IBOutlet var firmwareRevisionTextLabel: UILabel!
    @IBOutlet var updateFirmwareButton: UIButton!
    @IBOutlet var doneButton: UIBarButtonItem!
    
    @IBOutlet var progressBar: UIProgressView!
    @IBOutlet var progressTextLabel: UILabel!
    @IBOutlet var dfuMessageTextLabel: UILabel!
    
    var device: SenStickDevice?
    var firmwareRevision: String = ""
    var firmwareFilePath: String = ""
    var dfuController: DFUServiceController?
    var didEnterDFUmode             = false
    var didStartingFirmwareUpdating = false
    
    // View controller life cycle
    
    override func viewWillAppear(_ animated: Bool) {
        super.viewWillAppear(animated)

        self.serialNumberTextLabel.text     = self.device?.deviceInformationService?.serialNumber
        self.manufacturerNameTextLabel.text = self.device?.deviceInformationService?.manufacturerName
        self.hardwareRevisionTextLabel.text = self.device?.deviceInformationService?.hardwareRevision
        self.firmwareRevisionTextLabel.text = self.device?.deviceInformationService?.firmwareRevision

        self.progressBar.isHidden       = true
        self.progressTextLabel.text   = ""
        self.dfuMessageTextLabel.text = ""
        
        // plistを開いて更新ファーム情報を読み取る
        if let path = Bundle.main.path(forResource: "firmwareInfo", ofType: "plist") {
            let dict = NSDictionary(contentsOfFile: path)
            firmwareRevision = dict!.value(forKey: "revision")! as! String
            let resourcePath = Bundle.main.resourcePath! as NSString
            firmwareFilePath = resourcePath.appendingPathComponent( dict!.value(forKey: "filename") as! String)
        }
        
        // ファームの番号確認。最新でなければ、更新ボタンを有効に
        if self.device?.deviceInformationService?.firmwareRevision != firmwareRevision {
            updateFirmwareButton.isEnabled = true
        } else {
            updateFirmwareButton.isEnabled = false
        }
    }
    
    func dismiss()
    {
        if didEnterDFUmode || didStartingFirmwareUpdating {
            // DFU更新でCentralManagerのDelegateが持って行かれているのを、元に戻す
            SenStickDeviceManager.sharedInstance.reset()
            _ = self.navigationController?.popToRootViewController(animated: true)
        } else {
            _ = self.navigationController?.popViewController(animated: true)
        }
    }
    
    func showDialogAndDismiss(_ title: String, message:String)
    {
        let alert    = UIAlertController(title: title, message: message, preferredStyle: .alert)
        let okAction = UIAlertAction(title: "OK", style: .default, handler: { (UIAlertAction) -> Void in
            self.dismiss()
        })
        alert.addAction(okAction)
        
        present(alert, animated: true, completion:nil)
    }
    
    func performDfu(_ peripheral:CBPeripheral)
    {
        didStartingFirmwareUpdating = true
        
        let firmware = DFUFirmware(urlToZipFile: URL(fileURLWithPath: self.firmwareFilePath))
        let initiator = DFUServiceInitiator(centralManager: self.device!.manager, target: peripheral)
        _ = initiator.withFirmwareFile(firmware!)
        initiator.logger           = self
        initiator.delegate         = self
        initiator.progressDelegate = self
        
        self.dfuController = initiator.start()
    }
    
    // Event handler
    @IBAction func doneButtonTouchUpInside(_ sender: UIBarButtonItem) {
        dismiss()
    }
    
    @IBAction func updateFirmwareButtonToutchUpInside(_ sender: UIButton) {
        // ボタンを操作不可能に, DFU更新のUI初期状態
        self.updateFirmwareButton.isEnabled = false

        self.progressBar.isHidden   = false
        self.progressBar.progress = 0
        
        // DFUモードに入る
        self.device?.controlService?.writeCommand(.enterDFUMode)

        // dfu更新フラグを消しておく
        didEnterDFUmode = true
        
        // 切断処理、デリゲートを設定
        self.device!.manager.delegate = self
        self.device!.cancelConnection()

        // この後のDFU更新処理は、BLEデバイスの切断イベントの中で継続する

        // 5秒間でデバイスが発見できない場合は、アラート表示して、このVCを閉じる
        DispatchQueue.main.asyncAfter(deadline: DispatchTime.now() + Double(Int64(5 * NSEC_PER_SEC)) / Double(NSEC_PER_SEC), execute: {
            if !self.didStartingFirmwareUpdating {
                self.device!.manager.stopScan()
                self.showDialogAndDismiss("タイムアウト", message: "DFU更新を開始できませんでした。")
            }
        })
    }

    // CBCentralManager
    func centralManagerDidUpdateState(_ central: CBCentralManager)
    {
    }
    

    func centralManager(_ central: CBCentralManager, didDisconnectPeripheral peripheral: CBPeripheral, error: Error?) {
        debugPrint("\(#function) \(peripheral)")

        // デバイスとの接続切断、ペリフェラルを検索。DFU更新に入る
        // DFU時はサービスUUIDをアドバタイジングしない。したがって、すべてのデバイスがスキャン対象。
        self.device!.manager.scanForPeripherals(withServices: nil, options: nil)
    }

    func centralManager(_ central: CBCentralManager, didDiscover peripheral: CBPeripheral, advertisementData: [String : Any], rssi RSSI: NSNumber)
    {
        // MAGIC WORD, DFUのアドバタイジング時のローカルネームで判定
      if let localName = advertisementData[CBAdvertisementDataLocalNameKey] {
        if localName as! String == "ActDfu" {
            self.device!.manager.stopScan()
            performDfu(peripheral)
        }
      }
    }
    
    // LoggerDelegate
    func logWith(_ level:LogLevel, message:String)
    {
        debugPrint("\(#function) \(message)")
        /*
        dispatch_async(dispatch_get_main_queue(), {
            self.dfuMessageTextLabel.text = message
        })*/
    }
    
    // DFUServiceDelegate
    func didStateChangedTo(_ state:DFUState) {
        switch state {
            case .connecting: break
            case .starting: break
            case .enablingDfuMode: break
            case .uploading:
                self.dfuMessageTextLabel.text = "Uploading"
            case .validating:
                self.dfuMessageTextLabel.text = "Validating"
            case .disconnecting: break
            case .completed:
                showDialogAndDismiss("完了", message: "ファームウェア更新完了")
            case .aborted:
                showDialogAndDismiss("エラー", message: "アボート")
            case .signatureMismatch:
                showDialogAndDismiss("エラー", message: "署名の不一致")
            case .operationNotPermitted:
                showDialogAndDismiss("エラー", message: "操作の許可がありません")
            case .failed:
                showDialogAndDismiss("エラー", message: "失敗")
        }
    }

    func didErrorOccur(_ error:DFUError, withMessage message:String)
    {
        debugPrint("\(#function) \(message)")
        showDialogAndDismiss("エラー", message: message)
    }
    
    // DFUProgressDelegate
    func onUploadProgress(_ part:Int, totalParts:Int, progress:Int, currentSpeedBytesPerSecond:Double, avgSpeedBytesPerSecond:Double)
    {
        self.progressTextLabel.text = "\(progress)%"
        self.progressBar.progress = Float(progress) / Float(100)
    }
}
