//
//  SensorDataModel.swift
//  SenStickViewer
//
//  Created by AkihiroUehara on 2016/06/25.
//  Copyright © 2016年 AkihiroUehara. All rights reserved.
//

import UIKit
import SenStickSDK

// 実装メモ
// センサデータの扱いパターンは同じものなので、ジェネリックで実装するのとスッキリする。
// しかしジェネリックを使うと、インスタンスの配列の定義ができない(ジェネリックの<T>が実装ごとに異なるから。）このため、扱う側が一律に扱えなくなる。
// これはジェネリックをインスタンスとして扱う側に見せてしまうために、そうなる。インスタンスとして外部に見せる部分はプロトコルにしておく、実装として同じパターンを1つにまとめるために、ジェネリックを利用する。

protocol SensorDataModelProtocol :class {
    var cell: SensorDataCellView? { get set }
    
    var duration: SamplingDurationType { get set }
    // センサデータの記録開始時刻
    var sensorDataStartAt: Date { get set }

    var logid: UInt8 { get }
    var logData: [[Double]] { get }

    var sensorName: String  { get }
    
    var csvHeader:    String { get }
    var csvEmptyData: String { get }
    
    func getCSVDataText(_ index:Int) -> String
    func saveToFile(_ filePath:String)
    func startToReadLog(_ logid: UInt8)    
}

protocol SensorDataModelDelegate :class {
    func didStopReadingLog(_ sender: SensorDataModelProtocol)
}

class SensorDataModel<RangeType: RawRepresentable, DataType: SensorDataPackableType>: SenStickSensorServiceDelegate, SensorDataModelProtocol where RangeType.RawValue == UInt16, DataType.RangeType == RangeType {
    
    weak var service: SenStickSensorService<DataType,RangeType>? {
        didSet {
            self.service?.delegate = self
            didUpdateSetting(self)
        }
    }
    
    weak var delegate: SensorDataModelDelegate?
    
    let sampleCount: Int = 300
    
    var cell: SensorDataCellView? {
        willSet {
            cell?.iconButton?.removeTarget(self, action: #selector(iconButtonToutchUpInside), for: .touchUpInside)
        }
        didSet {
            cell?.iconButton?.addTarget(self, action: #selector(iconButtonToutchUpInside), for: .touchUpInside)
            if self.isLogReading {
                didUpdateMetaData(self)
            } else {
                didUpdateSetting(self)
            }
            cell?.graphView?.plotData(logData)
//debugPrint("\(#function)")
        }
    }

    var isLogReading: Bool = false
    
    var sensorName: String   = "sensor"
    var csvHeader: String    = "header"
    var csvEmptyData: String = ""
    
    var logData: [[Double]] = [[], [], []]
    var logid: UInt8 = 0
    
    // MARK: - Properties
    
    var maxValue: Double {
        didSet {
            updateCell()
        }
    }
    var minValue: Double {
        didSet {
            updateCell()
        }
    }
    var duration: SamplingDurationType {
        didSet {
            updateCell()
        }
    }
    
    // センサデータの記録開始時刻
    var sensorDataStartAt: Date = Date.init(timeIntervalSince1970: 0)
    
    // MARK: - Initializer
    
    init()
    {
        maxValue = 1.0
        minValue = 0
        duration = SamplingDurationType(milliSeconds: 100)
    }
    
    convenience init(_ delegate: SensorDataModelDelegate?)
    {
        self.init()
        self.delegate = delegate
    }
    
    deinit
    {
        cell = nil
    }
    
    // MARK: - Methods , which should be override

    func dataToArray(_ data: DataType) -> [Double]
    {
        return []
    }

    
    func updateRange(_ range: RangeType) -> Void
    {
        return
    }
    
    
    // MARK: - Private methods
    
    func updateCell()
    {
        cell?.maxValue = self.maxValue
        cell?.minValue = self.minValue
        cell?.duration = duration
    }
    
    func clearPlot()
    {
        logData = [[], [], []]
        cell?.graphView?.clearPlot()
    }
    
    func drawRealTimeData(_ sensorData: DataType)
    {
        let data = dataToArray(sensorData)
        
        // データを追加
        for (index, d) in data.enumerated() {
            logData[index].append(d)
        }
        
        // データが範囲を溢れたらクリア
        if logData[0].count > self.sampleCount {
            logData = [[], [], []]
        }
        
        // 描画
        self.cell?.graphView?.plotData(logData)
    }
    
    func startToReadLog(_ logid: UInt8)
    {
        _ = service?.readLogData()
        
        self.isLogReading = true
        
        self.logid = logid
        logData = [[], [], []]
        
        cell?.graphView?.clearPlot()
        cell?.progressBar?.progress = 0
        cell?.progressBar?.isHidden   = false
        
        let logID = SensorLogID(logID: logid, skipCount: 0, position: 0)
        service?.writeLogID(logID)
    }
    
    func addReadLog(_ sensorData: DataType) {
        let data = dataToArray(sensorData)
        for (index, d) in data.enumerated() {
            logData[index].append(d)
        }
        
        cell?.graphView?.plotData(logData)
        cell?.progressBar?.progress = Float(logData[0].count) / Float(self.cell!.graphView!.sampleCount)
    }
    
    // 指定したrowのデータをCSVテキストに変換。該当データがない場合は、CSVとしてデータが空の文字列を返す。
    func getCSVDataText(_ index:Int) -> String
    {
        let colomn  = logData.count
        var items = [String]()
        for c in 0..<colomn {
            if logData[c].count > index {
                items.append( "\((logData[c])[index])" )
            } else {
                break
            }
        }
        return items.joined(separator: ",")
    }
    
    func saveToFile(_ filePath:String)
    {
        var content = ""
        let row     = logData[0].count
        
        // 浮動小数点型だと、インクリメントしていくと微妙な端数がでるので、ミリ秒単位で整数で扱う
        let samplingDuration = Int(duration.duration * 1000)
        let startAt = Int(self.sensorDataStartAt.timeIntervalSince1970 * 1000)
        var time :Int = 0
        for r in 0..<row {
            time    += samplingDuration
            // 行頭の時間は、1970年を基準としたミリ秒。
//            content += "\(Double(time) / 1000),"
            content += "\(startAt + time),"
            content += self.getCSVDataText(r)
            content += "\n"
        }
        
        do {
            if FileManager.default.fileExists(atPath: filePath) {
                try FileManager.default.removeItem(atPath: filePath)
            }
            try content.write(toFile: filePath, atomically: true, encoding: String.Encoding.utf8)
        } catch {
            debugPrint("\(#function) fatal error in file save.")
        }
    }
    
    // MARK: - Event handler
    
    @objc func iconButtonToutchUpInside(_ sender: UIButton) {
        let status :SenStickStatus = cell!.iconButton!.isSelected ? .stopping : .sensingAndLogging
        
        if let current_setting = self.service?.settingData {
            let setting = SensorSettingData<RangeType>(status: status, samplingDuration: current_setting.samplingDuration, range: current_setting.range)
            
            service?.writeSetting(setting)
        }
        service?.readSetting()
    }
    
    // MARK: - SenStickSensorServiceDelegate
    
    func didUpdateSetting(_ sender:AnyObject)
    {
        cell?.iconButton?.isEnabled  = (self.service != nil)
        cell?.iconButton?.isSelected = (service?.settingData?.status != .stopping)
        
        if let setting = service?.settingData {
            self.duration = setting.samplingDuration
            updateRange(setting.range)
        }
    }
    
    func didUpdateRealTimeData(_ sender: AnyObject)
    {
        if let data = service?.realtimeData {
            drawRealTimeData(data)
        }
    }
    
    func didUpdateMetaData(_ sender: AnyObject)
    {
        guard let metaData = service?.logMetaData else {
            return
        }
        
        //        debugPrint("\(#function), availableCount: \(service!.logMetaData!.availableSampleCount)")
        self.duration = metaData.samplingDuration
        updateRange(metaData.range)
        
        let count = metaData.availableSampleCount
        cell?.graphView?.sampleCount = Int(count)
        cell?.iconButton?.isEnabled    = (count != 0)
        cell?.iconButton?.isSelected   = (count != 0)
        cell?.progressBar?.isHidden    = (count == 0)
    }
    
    func didUpdateLogData(_ sender: AnyObject)
    {
        if let array = service?.readLogData() {
            for data in array {
                addReadLog(data)
            }
        }
    }
    
    func didFinishedLogData(_ sender: AnyObject)
    {
        // グラフの終了状態
        cell?.progressBar?.isHidden    = true
        if let dlgt = self.delegate {
            dlgt.didStopReadingLog(self)
        }
    }
}
