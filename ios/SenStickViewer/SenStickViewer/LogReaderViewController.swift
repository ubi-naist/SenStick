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
    var sensorMetaData: SenStickMetaDataService?
    
    var dataModels : [SensorDataModelProtocol]?
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
    
    override func viewDidAppear(_ animated: Bool) {
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
    
    override func viewWillDisappear(_ animated: Bool) {
        super.viewWillDisappear(animated)
        
        device?.delegate = nil
    }
    
    // MARK: - SenStickDeviceDelegate
    
    func didServiceFound(_ sender: SenStickDevice)
    {
    }
    
    func didConnected(_ sender:SenStickDevice)
    {
    }
    
    func didDisconnected(_ sender:SenStickDevice)
    {
        _ = self.navigationController?.popToRootViewController(animated: true)
    }
    
    // SensorDataModelDelegate
    func didStopReadingLog(_ sender: SensorDataModelProtocol)
    {
        debugPrint("\(#function) \(sender.sensorName).")

        // センサデータの記録開始時間を設定。
        sender.sensorDataStartAt = self.sensorMetaData!.dateTime
        
        // データフォルダを用意
        let folder         = getDataFileFolder()
        if !FileManager.default.fileExists(atPath: folder)
        {
            try! FileManager.default.createDirectory(atPath: folder, withIntermediateDirectories: true, attributes: nil)
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
    override func tableView(_ tableView: UITableView, willDisplay cell: UITableViewCell, forRowAt indexPath: IndexPath)
    {
        let dataCell = cell as? SensorDataCellView
        
        dataCell?.iconButton?.isUserInteractionEnabled = false
        dataModels![(indexPath as NSIndexPath).row].cell = dataCell
    }
    
    func getDataFileFolder() -> String
    {
        let documentFolder = NSSearchPathForDirectoriesInDomains(.documentDirectory,  .userDomainMask, true).first! as NSString
        return documentFolder.appendingPathComponent("\(self.device!.name)") as String
    }
    
    func getDataFilePath(_ model: SensorDataModelProtocol) -> String
    {
        let folder = self.getDataFileFolder() as NSString
        return folder.appendingPathComponent("\(model.sensorName)_\(model.logid).csv") as String
    }
    
    func saveDataFile()
    {
        // サンプリング周期の配列を作成, ミリ秒単位で整数で。
        let samplingDurations = dataModels!.map { Int($0.duration.duration * 1000) }
        
        var content = ""
        
        // ヘッダを吐き出す
        content += "time,"
        content +=  dataModels!.map { $0.csvHeader}.joined(separator: ",")
        content += "\n"
        
        var time: Int   = 0
        // データ系列の終端時間を求める。
        let endTime = dataModels!.map{ Int($0.duration.duration * 1000 ) * $0.logData[0].count }.max()!
        let dataStartAt = Int(self.sensorMetaData!.dateTime.timeIntervalSince1970 * 1000)
        repeat {
            // 次のサンプリング時間を求める。
            time = samplingDurations.map { (time / $0 + 1) * $0 }.min()!
            // CSVデータ部分を作る
            var isValid = false
            let csv = dataModels!.map {
                let ms    = Int($0.duration.duration * 1000)
                let index = time / ms - 1
                if time % ms == 0 && index < $0.logData[0].count {
                    isValid = true
                    return $0.getCSVDataText(index)
                } else {
                    return $0.csvEmptyData
                }
                }.joined(separator: ",")
            // CSVを出力
            if isValid {
                // 行先頭の時刻は、1970年を基準にしたミリ秒で表示する
//                content += "\(Double(time) / 1000),"
                content += "\(dataStartAt + time),"
                content += csv
                content += "\n"
            }            
        } while(time <= endTime)
        
        let folder   = self.getDataFileFolder() as NSString
        let filePath = folder.appendingPathComponent("data_\(self.dataModels![0].logid).csv")
        
        // データフォルダを用意
        if !FileManager.default.fileExists(atPath: folder as String)
        {
            try! FileManager.default.createDirectory(atPath: folder as String, withIntermediateDirectories: true, attributes: nil)
        }
        // ファイルに保存
        try! content.write(toFile: filePath, atomically: true, encoding: String.Encoding.utf8)
    }
}

