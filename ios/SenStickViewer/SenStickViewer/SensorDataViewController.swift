//
//  SensorDataViewController.swift
//  SenStickViewer
//
//  Created by AkihiroUehara on 2016/05/22.
//  Copyright © 2016年 AkihiroUehara. All rights reserved.
//

import UIKit
import SenStickSDK
import CoreMotion

class SensorDataViewController : UITableViewController, SenStickDeviceDelegate {
    
    @IBOutlet var deviceInformationButton: UIBarButtonItem!
    
    var device: SenStickDevice?
    
    var statusCell:             SensorStatusCellView?
    var accelerationDataModel:  AccelerationDataModel?
    var gyroDataModel:          GyroDataModel?
    var magneticFieldDataModel: MagneticFieldDataModel?
    var brightnessDataModel:    BrightnessDataModel?
    var uvDataModel:            UVDataModel?
    var humidityDataModel:      HumidityDataModel?
    var pressureDataModel:      PressureDataModel?
    
    override func viewDidLoad()
    {
        super.viewDidLoad()
        
        accelerationDataModel  = AccelerationDataModel()
        gyroDataModel          = GyroDataModel()
        magneticFieldDataModel = MagneticFieldDataModel()
        brightnessDataModel    = BrightnessDataModel()
        uvDataModel            = UVDataModel()
        humidityDataModel      = HumidityDataModel()
        pressureDataModel      = PressureDataModel()
    }
    
    override func viewWillAppear(animated: Bool)
    {
        super.viewWillAppear(animated)
        
        device?.delegate = self
        device?.connect()
        if device != nil {
            didServiceFound(device!)
        }
        
        // 右上のバーボタンアイテムのenable設定。ログ停止中のみ遷移可能
        /*
         if let control = device?.controlService {
         self.deviceInformationButton.enabled = (control.command == .Stopping)
         } else {
         self.deviceInformationButton.enabled = false
         }*/
    }
    
    override func viewWillDisappear(animated: Bool)
    {
        super.viewWillDisappear(animated)
        
        device?.delegate = nil
        
        // リスト表示に戻る場合は、デバイスとのBLE接続を切る
        if let backToListView = self.navigationController?.viewControllers.contains(self) {
            // ListViewに戻る時、ナビゲーションに自身が含まれていない。
            if backToListView == false {
                device?.cancelConnection()
            }
        }
    }

    func clearGraph()
    {
        accelerationDataModel?.clearPlot()
        gyroDataModel?.clearPlot()
        magneticFieldDataModel?.clearPlot()
        brightnessDataModel?.clearPlot()
        uvDataModel?.clearPlot()
        humidityDataModel?.clearPlot()
        pressureDataModel?.clearPlot()
    }

    // MARK: - SenStickDeviceDelegate
    func didServiceFound(sender: SenStickDevice) {
        self.statusCell?.name       = device?.name
        self.statusCell?.service    = device?.controlService

        accelerationDataModel?.service  = device?.accelerationSensorService
        gyroDataModel?.service          = device?.gyroSensorService
        magneticFieldDataModel?.service = device?.magneticFieldSensorService
        brightnessDataModel?.service    = device?.brightnessSensorService
        uvDataModel?.service            = device?.uvSensorService
        humidityDataModel?.service      = device?.humiditySensorService
        pressureDataModel?.service      = device?.pressureSensorService
    }
    
    func didConnected(sender:SenStickDevice)
    {
    }
    
    func didDisconnected(sender:SenStickDevice)
    {
        self.navigationController?.popToRootViewControllerAnimated(true)
    }
    
    override func tableView(tableView: UITableView, willDisplayCell cell: UITableViewCell, forRowAtIndexPath indexPath: NSIndexPath)
    {
        switch (indexPath.row) {
        case 0:
            self.statusCell              = cell as? SensorStatusCellView
        self.statusCell?.controller      = self
        case 1:
            accelerationDataModel?.cell  = cell as? SensorDataCellView
        case 2:
            gyroDataModel?.cell          = cell as? SensorDataCellView
        case 3:
            magneticFieldDataModel?.cell = cell as? SensorDataCellView
        case 4:
            brightnessDataModel?.cell    = cell as? SensorDataCellView
        case 5:
            uvDataModel?.cell            = cell as? SensorDataCellView
        case 6:
            humidityDataModel?.cell      = cell as? SensorDataCellView
        case 7:
            pressureDataModel?.cell      = cell as? SensorDataCellView
        default: break
        }
    }
    
    
    // MARK: - Segues
    override func shouldPerformSegueWithIdentifier(identifier: String, sender: AnyObject?) -> Bool
    {
        // デバイス詳細情報表示に遷移
        if identifier == "deviceInformation" {
            return true
        }
        
        // 詳細表示遷移できるのはログ停止時だけ
        if let control = device?.controlService {
            return (control.command == .Stopping)
        } else {
            return false
        }
    }
    
    override func prepareForSegue(segue: UIStoryboardSegue, sender: AnyObject?)
    {
        if let vc = segue.destinationViewController as? DeviceInformationViewController {
            vc.device = self.device
        }
        
        if let vc = segue.destinationViewController as?  LogListViewController {
            vc.device = self.device
        }
        
        //        debugPrint("  \(segue.destinationViewController)")
        if let vc = segue.destinationViewController as? SamplingDurationViewController {
            let indexPath = self.tableView.indexPathForSelectedRow!
            switch(indexPath.row) {
            case 1:
                vc.target = device?.accelerationSensorService
            case 2:
                vc.target = device?.gyroSensorService
            case 3:
                vc.target = device?.magneticFieldSensorService
            case 4:
                vc.target = device?.brightnessSensorService
            case 5:
                vc.target = device?.uvSensorService
            case 6:
                vc.target = device?.humiditySensorService
            case 7:
                vc.target = device?.pressureSensorService
            default: break
            }
        }
    }
}
