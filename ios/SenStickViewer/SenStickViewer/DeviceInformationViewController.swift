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
    
    override func viewWillAppear(animated: Bool) {
        super.viewWillAppear(animated)

        self.serialNumberTextLabel.text     = self.device?.deviceInformationService?.serialNumber
        self.manufacturerNameTextLabel.text = self.device?.deviceInformationService?.manufacturerName
        self.hardwareRevisionTextLabel.text = self.device?.deviceInformationService?.hardwareRevision
        self.firmwareRevisionTextLabel.text = self.device?.deviceInformationService?.firmwareRevision

        self.progressBar.hidden       = true
        self.progressTextLabel.text   = ""
        self.dfuMessageTextLabel.text = ""
        
        // plistを開いて更新ファーム情報を読み取る
        if let path = NSBundle.mainBundle().pathForResource("firmwareInfo", ofType: "plist") {
            let dict = NSDictionary(contentsOfFile: path)
            firmwareRevision = dict!.valueForKey("revision")! as! String
            let resourcePath = NSBundle.mainBundle().resourcePath! as NSString
            firmwareFilePath = resourcePath.stringByAppendingPathComponent( dict!.valueForKey("filename") as! String)
        }
        
        // ファームの番号確認。最新でなければ、更新ボタンを有効に
        if self.device?.deviceInformationService?.firmwareRevision != firmwareRevision {
            updateFirmwareButton.enabled = true
        } else {
            updateFirmwareButton.enabled = false
        }
    }
    
    func dismiss()
    {
        if didEnterDFUmode || didStartingFirmwareUpdating {
            // DFU更新でCentralManagerのDelegateが持って行かれているのを、元に戻す
            SenStickDeviceManager.sharedInstance.reset()
            self.navigationController?.popToRootViewControllerAnimated(true)
        } else {
            self.navigationController?.popViewControllerAnimated(true)
        }
    }
    
    func showDialogAndDismiss(title: String, message:String)
    {
        let alert    = UIAlertController(title: title, message: message, preferredStyle: .Alert)
        let okAction = UIAlertAction(title: "OK", style: .Default, handler: { (UIAlertAction) -> Void in
            self.dismiss()
        })
        alert.addAction(okAction)
        
        presentViewController(alert, animated: true, completion:nil)
    }
    
    func performDfu(peripheral:CBPeripheral)
    {
        didStartingFirmwareUpdating = true
        
        let firmware = DFUFirmware(urlToZipFile: NSURL(fileURLWithPath: self.firmwareFilePath))
        let initiator = DFUServiceInitiator(centralManager: self.device!.manager, target: peripheral)
        initiator.withFirmwareFile(firmware!)
        initiator.logger           = self
        initiator.delegate         = self
        initiator.progressDelegate = self
        
        self.dfuController = initiator.start()
    }
    
    // Event handler
    @IBAction func doneButtonTouchUpInside(sender: UIBarButtonItem) {
        dismiss()
    }
    
    @IBAction func updateFirmwareButtonToutchUpInside(sender: UIButton) {
        // ボタンを操作不可能に, DFU更新のUI初期状態
        self.updateFirmwareButton.enabled = false

        self.progressBar.hidden   = false
        self.progressBar.progress = 0
        
        // DFUモードに入る
        self.device?.controlService?.writeCommand(.EnterDFUMode)

        // dfu更新フラグを消しておく
        didEnterDFUmode = true
        
        // 切断処理、デリゲートを設定
        self.device!.manager.delegate = self
        self.device!.cancelConnection()

        // この後のDFU更新処理は、BLEデバイスの切断イベントの中で継続する

        // 5秒間でデバイスが発見できない場合は、アラート表示して、このVCを閉じる
        dispatch_after(dispatch_time(DISPATCH_TIME_NOW, Int64(5 * NSEC_PER_SEC)), dispatch_get_main_queue(), {
            if !self.didStartingFirmwareUpdating {
                self.device!.manager.stopScan()
                self.showDialogAndDismiss("タイムアウト", message: "DFU更新を開始できませんでした。")
            }
        })
    }

    // CBCentralManager
    func centralManagerDidUpdateState(central: CBCentralManager)
    {
    }
    

    func centralManager(central: CBCentralManager, didDisconnectPeripheral peripheral: CBPeripheral, error: NSError?) {
        debugPrint("\(#function) \(peripheral)")

        // デバイスとの接続切断、ペリフェラルを検索。DFU更新に入る
        // DFU時はサービスUUIDをアドバタイジングしない。したがって、すべてのデバイスがスキャン対象。
        self.device!.manager.scanForPeripheralsWithServices(nil, options: nil)
    }

    func centralManager(central: CBCentralManager, didDiscoverPeripheral peripheral: CBPeripheral, advertisementData: [String : AnyObject], RSSI: NSNumber)
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
    func logWith(level:LogLevel, message:String)
    {
        debugPrint("\(#function) \(message)")
        /*
        dispatch_async(dispatch_get_main_queue(), {
            self.dfuMessageTextLabel.text = message
        })*/
    }
    
    // DFUServiceDelegate
    func didStateChangedTo(state:State) {
        switch state {
            case .Connecting: break
            case .Starting: break
            case .EnablingDfuMode:
            break
            case .Uploading:
                self.dfuMessageTextLabel.text = "Uploading"
            break
            case .Validating:
                self.dfuMessageTextLabel.text = "Validating"
            break
            case .Disconnecting: break
            case .Completed:
                showDialogAndDismiss("完了", message: "ファームウェア更新完了")
            
            case .Aborted:
                showDialogAndDismiss("エラー", message: "アボート")
        }
    }

    func didErrorOccur(error:DFUError, withMessage message:String)
    {
        debugPrint("\(#function) \(message)")
        showDialogAndDismiss("エラー", message: message)
    }
    
    // DFUProgressDelegate
    func onUploadProgress(part:Int, totalParts:Int, progress:Int, currentSpeedBytesPerSecond:Double, avgSpeedBytesPerSecond:Double)
    {
        self.progressTextLabel.text = "\(progress)%"
        self.progressBar.progress = Float(progress) / Float(100)
    }
}
