//
//  LogReaderViewController.swift
//  SenStickViewer
//
//  Created by AkihiroUehara on 2016/06/26.
//  Copyright © 2016年 AkihiroUehara. All rights reserved.
//

import UIKit
import SenStickSDK

class LogReaderViewController: UITableViewController, SenStickDeviceDelegate, SensorDataModelDelegate {
    var device: SenStickDevice?
    var logID: UInt8?
    var dataModels : [SensorDataModel]?
    var accelerationDataModel:  AccelerationDataModel?
    var gyroDataModel:          GyroDataModel?
    var magneticFieldDataModel: MagneticFieldDataModel?
    var brightnessDataModel:    BrightnessDataModel?
    var uvDataModel:            UVDataModel?
    var humidityDataModel:      HumidityDataModel?
    var pressureDataModel:      PressureDataModel?
    
    var savedLogCount: Int = 0
    
    // View controller life cycle
    override func viewDidLoad()
    {
        super.viewDidLoad()
        
        accelerationDataModel  = AccelerationDataModel(self)
        gyroDataModel          = GyroDataModel(self)
        magneticFieldDataModel = MagneticFieldDataModel(self)
        brightnessDataModel    = BrightnessDataModel(self)
        uvDataModel            = UVDataModel(self)
        humidityDataModel      = HumidityDataModel(self)
        pressureDataModel      = PressureDataModel(self)
        
        dataModels = [accelerationDataModel!, gyroDataModel!, magneticFieldDataModel!, brightnessDataModel!, uvDataModel!, humidityDataModel!, pressureDataModel!]
    }
    
    override func viewDidAppear(animated: Bool) {
        super.viewDidAppear(animated)
        
        device?.delegate = self
        
        // set services
        accelerationDataModel?.service  = device?.accelerationSensorService
        gyroDataModel?.service          = device?.gyroSensorService
        magneticFieldDataModel?.service = device?.magneticFieldSensorService
        brightnessDataModel?.service    = device?.brightnessSensorService
        uvDataModel?.service            = device?.uvSensorService
        humidityDataModel?.service      = device?.humiditySensorService
        pressureDataModel?.service      = device?.pressureSensorService
        
        savedLogCount = 0
        for model in dataModels! {
            model.startToReadLog(self.logID!)
        }
    }
    
    override func viewWillDisappear(animated: Bool) {
        super.viewWillDisappear(animated)
        
        device?.delegate = nil
    }
    
    // MARK: - SenStickDeviceDelegate
    
    func didServiceFound(sender: SenStickDevice)
    {
    }
    
    func didConnected(sender:SenStickDevice)
    {
    }
    
    func didDisconnected(sender:SenStickDevice)
    {
        self.navigationController?.popToRootViewControllerAnimated(true)
    }
    
    // SensorDataModelDelegate
    func didStopReadingLog(sender: SensorDataModel)
    {
        debugPrint("\(#function) \(sender.sensorName).")
        
        // データフォルダを用意
        let folder         = getDataFileFolder()
        if !NSFileManager.defaultManager().fileExistsAtPath(folder)
        {
            try! NSFileManager.defaultManager().createDirectoryAtPath(folder, withIntermediateDirectories: true, attributes: nil)
        }
        // ファイルに保存
        let filePath       = getDataFilePath(sender)
        sender.saveToFile(filePath)
        
        // すべて保存したなら、ファイル統合
        savedLogCount += 1
        if savedLogCount == dataModels!.count {
            saveDataFile()
        }
    }
    
    // table view source/delegate
    override func tableView(tableView: UITableView, willDisplayCell cell: UITableViewCell, forRowAtIndexPath indexPath: NSIndexPath)
    {
        let dataCell = cell as? SensorDataCellView
        
        dataCell?.iconButton?.userInteractionEnabled = false
        dataModels![indexPath.row].cell = dataCell
    }
    
    func getDataFileFolder() -> String
    {
        let documentFolder = NSSearchPathForDirectoriesInDomains(.DocumentDirectory,  .UserDomainMask, true).first! as NSString
        return documentFolder.stringByAppendingPathComponent("\(self.device!.name)") as String
    }
    
    func getDataFilePath(model: SensorDataModel) -> String
    {
        let folder = self.getDataFileFolder() as NSString
        return folder.stringByAppendingPathComponent("\(model.sensorName)_\(model.logid).csv") as String
    }
    
    func saveDataFile()
    {
        // サンプリング周期の配列を作成, ミリ秒単位で整数で。
        let samplingDurations = dataModels!.map { Int($0.duration.duration * 1000) }
        
        var content = ""
        
        // ヘッダを吐き出す
        content += "time,\t"
        content +=  dataModels!.map { $0.csvHeader}.joinWithSeparator(",\t")
        content += "\n"
        
        var time: Int   = 0
        // データ系列の終端時間を求める。
        let endTime = dataModels!.map{ Int($0.duration.duration * 1000 ) * $0.logData[0].count }.maxElement()!
        repeat {
            // 次のサンプリング時間を求める。
            time = samplingDurations.map { (time / $0 + 1) * $0 }.minElement()!
            // CSVデータ部分を作る
            var isValid = false
            let csv = dataModels!.map {
                let ms    = Int($0.duration.duration * 1000)
                let index = time / ms
                if time % ms == 0 && index < $0.logData[0].count {
                    isValid = true
                    return $0.getCSVDataText(time / ms)
                } else {
                    return $0.csvEmptyData
                }
                }.joinWithSeparator(",\t")
            // CSVを出力
            if isValid {
                content += "\(Double(time) / 1000),\t"
                content += csv
                content += "\n"
            }            
        } while(time < endTime)
        
        let folder   = self.getDataFileFolder() as NSString
        let filePath = folder.stringByAppendingPathComponent("data_\(self.dataModels![0].logid).csv")
        
        // データフォルダを用意
        if !NSFileManager.defaultManager().fileExistsAtPath(folder as String)
        {
            try! NSFileManager.defaultManager().createDirectoryAtPath(folder as String, withIntermediateDirectories: true, attributes: nil)
        }
        // ファイルに保存
        try! content.writeToFile(filePath, atomically: true, encoding: NSUTF8StringEncoding)
    }
}

